// Copyright 2009 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sitemapservice/pagehandler.h"

#include "common/logger.h"
#include "common/fileutil.h"
#include "common/httpconst.h"
#include "common/timesupport.h"
#include "sitemapservice/runtimeinfomanager.h"
#include "sitemapservice/httpsettingmanager.h"
#include "sitemapservice/sessionmanager.h"
#include "sitemapservice/webpagemanager.h"
#include "sitemapservice/passwordmanager.h"

#ifdef WIN32
#include "sitemapservice/mainservice.h"
#else
#include <errno.h>
#include "sitemapservice/daemon.h"
#endif

///////////////////////////////////////////////////////////////////////////
void RuntimeInfoHandler::Execute(HttpContext* context) {
  HttpResponse* response = context->response();

  std::string xmlstring;
  RuntimeInfoManager::Lock(true);
  bool res = RuntimeInfoManager::GetRuntimeInfoString(&xmlstring);
  RuntimeInfoManager::Unlock();
  if (!res) {
    response->Reset(HttpConst::kStatus500, "Failed to get runtime info.");
    Logger::Log(EVENT_ERROR, "!!Failed to get runtime info.");
  } else {
    response->set_status(HttpConst::kStatus200);
    response->SetHeader(HttpConst::kHeaderContentType,
                        "text/xml; charset=utf-8");
    response->set_message_body(xmlstring);
  }
}

void GetSettingHandler::Execute(HttpContext* context) {
  HttpResponse* response = context->response();

  std::string setting_str;
  if (!setting_manager_->GetXmlString(&setting_str)) {
    response->Reset(HttpConst::kStatus500, "Failed to load the settings.");
    return;
  }

  response->set_status(HttpConst::kStatus200);
  response->set_message_body(setting_str);
  response->SetHeader(HttpConst::kHeaderContentType,
                      "text/xml; charset=utf-8");
  response->SetHeader(HttpConst::kHeaderCacheControl,
                      "no-cache, must-revalidate");
}

const std::string SetSettingHandler::kWarnAnswer = "Settings is out-of-date";
const std::string SetSettingHandler::kSaveFailAnswer = "Save Failed";

bool SetSettingHandler::ReloadSetting() {
#ifdef WIN32
  return MainService::ReloadSetting() == 0;
#else
  return Daemon::ReloadSetting() == 0;
#endif
}

void SetSettingHandler::Execute(HttpContext* context) {
  HttpResponse* response = context->response();

  std::string xmlstring = context->GetParam(HttpContext::kXmlContentParamName);
  if (xmlstring.length() == 0) {
    Logger::Log(EVENT_ERROR, "no XML content");
    response->Reset(HttpConst::kStatus500, kSaveFailAnswer);
    return;
  }

  bool do_save = true;
  bool error = false;
  // Check if the timestamp is older than the current file written time.
  // If the 'force' flag is set, ignore the checking.
  if (context->GetParam(HttpContext::kForceSaveParamName) == "") {
    do {
      std::string ts_from_client = 
          context->GetParam(HttpContext::kXmlTimestampParamName);
      if (ts_from_client == "") {
        // if no timestamp, save it anyway
        break;
      }
      DLog(EVENT_NORMAL, "Timestamp from client [%s]", ts_from_client.c_str());

      // get the configuration file's timestamp
      time_t ts;
      if (!setting_manager_->GetLastModifiedTime((&ts))) {
        // if failed to get the timestamp of the file, report error
        response->Reset(HttpConst::kStatus500, kSaveFailAnswer);
        error = true;
        break;
      }

      std::string latest = FormatHttpDate(ts);
      DLog(EVENT_NORMAL, "Timestamp for XML now [%s]", latest.c_str());

      if (ts_from_client != latest) {
        // not the same timestamp, warning the client
        response->Reset(HttpConst::kStatus200, kWarnAnswer);
        do_save = false;
        break;
      } 
    } while (false);    
  }

  if (error || !do_save) {
    return;
  }

  // have the same timestamp, save the settings    
  if (!setting_manager_->SaveXml(xmlstring)) {
    Logger::Log(EVENT_ERROR, "!!Failed to save xml setting.");
    response->Reset(HttpConst::kStatus500, kSaveFailAnswer);
    return;
  }

  time_t ts;
  if(!setting_manager_->GetLastModifiedTime(&ts)) {
    // if one file access time cannot get, all the files should be sent
    response->Reset(HttpConst::kStatus500, "Get new timestamp failed");
    return;
  }

  std::string ts_str = FormatHttpDate(ts);
  response->Reset(HttpConst::kStatus200, ts_str);
  DLog(EVENT_NORMAL, "Timestamp after saving [%s]", response->message_body().c_str());

  if (!ReloadSetting()) {
    Logger::Log(EVENT_ERROR, "Failed to reload setting after saving.");
  }
}

void LoginHandler::Execute(HttpContext* context) {
  HttpResponse* response = context->response();
  
  // Get and check setting id.
  std::string session_id = context->session_id();
  if (session_id.empty() || !session_manager_->Exist(session_id)) {
    response->Reset(HttpConst::kStatus401, "Invalid session id.");
    return;      
  }
  
  // Validate password.
  std::string raw_pwd = context->GetParam(HttpContext::kPasswordParamName);

  if (!setting_manager_->Login(raw_pwd)) {
    response->Reset(HttpConst::kStatus401, "Invalid password.");
    return;
  } 

  // Save the state.
  if (!session_manager_->Login(session_id)) {
    response->Reset(HttpConst::kStatus401, "Login failed.");
    return;
  }

  response->Reset(HttpConst::kStatus200, "Success");
}

void LogoutHandler::Execute(HttpContext* context) {
  HttpResponse* response = context->response();

  std::string session_id = context->session_id();
  session_manager_->RemoveSession(session_id);

  response->Reset(HttpConst::kStatus200, "Success");
}

void MessageBundleHandler::Execute(HttpContext* context) {
  HttpRequest* request = context->request();
  HttpResponse* response = context->response();
  
  // TODO: modify this line to support multiple languages.
  // std::string lang = context->GetLanguage();
  std::string lang = "en-us";
  std::string file = WebPageManager::TranslatePath(context->action(), lang);

  // Check last modified time.
  FileAttribute file_attr;
  if (!FileUtil::GetFileAttribute(file.c_str(), &file_attr)) {
    response->Reset(HttpConst::kStatus404, "File not found.");
    Logger::Log(EVENT_ERROR, "File not found [%s].", file.c_str());
    return;
  }

  // if_modified_since_ from remote IE7 will have 'length=xxx' append str, just
  // ignore it by only check the substr from beginning.
  std::string server_time = FormatHttpDate(file_attr.last_modified);
  std::string client_time = request->GetHeader(HttpConst::kHeaderIfModifiedSince);
  if (client_time.substr(0, server_time.length()) == server_time) {
    response->Reset(HttpConst::kStatus304, "Not Modified");
    response->SetHeader(HttpConst::kHeaderLastModified, server_time);
    return;
  }

  // Load file content
  std::string content;
  if (!FileUtil::LoadFile(file.c_str(), &content)) {
    response->Reset(HttpConst::kStatus404, "File can't be loaded.");
    return;
  }

  response->set_status(HttpConst::kStatus200);
  response->set_message_body(content);
  response->SetHeader(HttpConst::kHeaderContentType, "text/javascript; charset=utf-8");
  response->SetHeader(HttpConst::kHeaderExpires,
                      FormatHttpDate(time(NULL) + 24 * 60 * 60));
  response->SetHeader(HttpConst::kHeaderLastModified, server_time);
}

void MainPageHandler::Execute(HttpContext* context) {
  HttpResponse* response = context->response();

  std::string session_id = context->session_id();
  if (session_id.empty() || !session_manager_->Exist(session_id)) {
    // It's a new session request, check if the session connections are full
    if (session_manager_->IsFull()) {
      response->Reset(HttpConst::kStatus503, "Too many sessions.");
      return;
    }

    // Generate a new session id.
    if (!session_manager_->CreateSession(setting_manager_->GetPassword(),
                                         &session_id)) {
      response->Reset(HttpConst::kStatus503, "Failed to create session.");
      return;
    }
  }
  context->set_session_id(session_id);

  // Construct main file.
  std::string htmlstring;
  if (!WebPageManager::ConstructMainPage(&htmlstring,
         context->session_id(), context->need_login())) {
    response->Reset(HttpConst::kStatus404, "Failed to load file.");
    return;
  }

  response->set_status(HttpConst::kStatus200);
  response->set_message_body(htmlstring);
  response->SetHeader(HttpConst::kHeaderContentType, "text/html");
}

const std::string ReloadSettingHandler::kReloadFailAnswer = "Reload Failed";

void ReloadSettingHandler::Execute(HttpContext* context) {
  
  DLog(EVENT_IMPORTANT, "Get request (reload setting)");
  HttpResponse* response = context->response();

  if (!ReloadSetting()) {
    Logger::Log(EVENT_ERROR, "Fail to reload setting.");
    response->Reset(HttpConst::kStatus503, kReloadFailAnswer);
    return;
  }

  response->Reset(HttpConst::kStatus200, "Success");
}

bool ReloadSettingHandler::ReloadSetting() {
#ifdef WIN32
  return MainService::ReloadSetting() == 0;
#else
  return Daemon::ReloadSetting() == 0;
#endif
}

void ChangePasswordHandler::Execute(HttpContext *context) {
  HttpResponse* response = context->response();

  std::string old_pw = context->GetParam(HttpContext::kOldPasswordParamName);
  std::string new_pw = context->GetParam(HttpContext::kNewPasswordParamName);
  PasswordManager::LoginResult result = PasswordManager::Login(old_pw);
  if (result == PasswordManager::LOGIN_FAIL) {
    response->Reset(HttpConst::kStatus401, "");
    return;
  } else if (result == PasswordManager::LOGIN_ERROR) {
    response->Reset(HttpConst::kStatus500, "login error");
    return;
  }

  if (!PasswordManager::ChangePassword(new_pw)) {
    response->Reset(HttpConst::kStatus500, "save password error");
    return;
  }

  response->Reset(HttpConst::kStatus200, "");
}

