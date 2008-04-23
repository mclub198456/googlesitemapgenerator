// Copyright 2008 Google Inc.
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

#include "sitemapservice/pagecontroller.h"

#include <iostream>
#include <stdlib.h> 
#include <sstream>

#ifdef WIN32
//#include <shellapi.h>
#else 
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <errno.h> 
#include <fcntl.h>
#endif

#include "common/port.h"
#include "common/fileutil.h"
#include "common/timesupport.h"
#include "sitemapservice/httplanguageheaderparser.h"
#include "sitemapservice/runtimeinfomanager.h"
#include "sitemapservice/httpproto.h"
#include "sitemapservice/webserver.h"
#include "sitemapservice/settingmanager.h"
#include "sitemapservice/sessionmanager.h"
#include "sitemapservice/httpmanager.h"
#include "sitemapservice/securitymanager.h"
#include "sitemapservice/webpagemanager.h"

#ifdef _DEBUG
#undef DLog
#define DLog(level, format, ...) \
  Util::Log(level, "%d: " format, __LINE__, ##__VA_ARGS__)
#endif

/////////////////////// static class member /////////////////////////////
PageController* PageController::instance_ = NULL;
const std::string PageController::kXmlGetAction = "/getxml";
const std::string PageController::kRuntimeInfoAction = "/getruntimeinfo";
const std::string PageController::kXmlSetAction = "/postxml";
const std::string PageController::kSaveAndRestartAction = "/saverestart";
const std::string PageController::kLogoutAction = "/logout";
const std::string PageController::kLoginAction = "/login";
const std::string PageController::kChangePasswordAction = "/chpswd";

const std::string PageController::kUnittestPostAction = "/unittest/post";
const std::string PageController::kUnittestGetAction = "/unittest/get";
const std::string PageController::kUnittestCrashAction = "/unittest/crash";
const std::string PageController::kUnittestLongtimeAction = "/unittest/longtime";
const std::string PageController::kMainAction = "/main";



const std::string PageController::kUnittestPostAndGetContentPath = "/unittest/postcontent.txt";




/////////////////////// static class function /////////////////////////////
PageController* PageController::getInstance() { 
  if (instance_ == NULL) 
    instance_ = new PageController();
  return instance_; 
}

void PageController::PageControl(HttpProto *r) {
  PageController* controller = PageController::getInstance();

  if (r->path_ == kMainAction || r->path_ == "/") {
    // access main page
    controller->AccessMainPage(r);

  } else if (r->path_ == kLoginAction) {  
    // login
    controller->Login(r);

  } else if (r->path_ == kLogoutAction) {
    // logout, check authorization first
    if (SecurityManager::SecurityCheck(
      r, controller->sessionMng_, 
      controller->settingMng_->AllowRemoteAccess())) { 
        controller->Logout(r);
    }

  } else if (r->path_ == kUnittestPostAction ||
    r->path_ == kUnittestGetAction ||
    r->path_ == kUnittestCrashAction ||
    r->path_ == kUnittestLongtimeAction) {
      // access unittest files
      controller->UnittestProcess(r);  

  } else if (r->path_ == kSaveAndRestartAction) {
    // save and restart server, check authorization first
    if (SecurityManager::SecurityCheck(r, controller->sessionMng_,
      controller->settingMng_->AllowRemoteAccess())) { 
        if (controller->SaveSetting(r)) {
          controller->ServeRestartAction(r);
        }
    }

  } else if (r->path_ == kChangePasswordAction) {
    // save and restart server, check authorization first
    if (SecurityManager::SecurityCheck(r, controller->sessionMng_,
      controller->settingMng_->AllowRemoteAccess())) {
        controller->ChangePassword(r);
    }

  } else if (r->path_ == kXmlGetAction) {
    // get sitesetting xml file, check authorization first
    if (SecurityManager::SecurityCheck(r, controller->sessionMng_,
      controller->settingMng_->AllowRemoteAccess())) {
        controller->AccessXmlFile(r);
    }    

  } else if (r->path_ == kXmlSetAction) {
    // post sitesetting xml file, check authorization first
    if (SecurityManager::SecurityCheck(r, controller->sessionMng_,
      controller->settingMng_->AllowRemoteAccess())) {
        controller->SaveSetting(r);
    }    

  } else if (r->path_ == kRuntimeInfoAction) {
    // get sitesetting xml file, check authorization first
    if (SecurityManager::SecurityCheck(r, controller->sessionMng_,
      controller->settingMng_->AllowRemoteAccess())) {
        controller->GetRuntimeInfo(r);
    }    

  } else if (Util::Match(r->path_, -1, "all.js")) {
    WebPageManager::AccessAggregateJSFile(r);
  } else {
    // access static files
    WebPageManager::AccessStaticFile(r);
  }
}

/////////////////////// public class methods /////////////////////////////
// Constructor
PageController::PageController() {
  sessionMng_ = new SessionManager();
  settingMng_ = new SettingManager();
}

void PageController::setUpdateListener(SettingUpdateListener* listener) {
  settingMng_->SetUpdateListener(listener);
}


/////////////////////// private class methods /////////////////////////////
void PageController::GetRuntimeInfo(HttpProto *r) {
  DLog(EVENT_IMPORTANT, "Get request (RUNTIME INFO)");

  std::string xmlstring;
  RuntimeInfoManager::Lock(true);
  bool res = RuntimeInfoManager::GetRuntimeInfoString(&xmlstring);
  RuntimeInfoManager::Unlock();
  if (!res) {
    Util::Log(EVENT_ERROR, "!!Failed to get runtime info.");
    xmlstring = "";
  } else {
    r->answer_content_type_ = "text/xml";
  }

  r->answer_ = xmlstring;
  DLog(EVENT_IMPORTANT, "Send response: size %d", r->answer_.length());
}

bool PageController::ChangePassword(HttpProto* r) {
  std::string password = r->GetParam(HttpManager::kOldPasswordParamName);
  std::string new_password = r->GetParam(HttpManager::kNewPasswordParamName);

  DLog(EVENT_IMPORTANT, "Get request (PASSWORD CHANGE)");

  int result = settingMng_->ChangePassword(password, new_password);

  if (result == 0) {
    r->answer_ = "Success";
  } else if (result == -1) {
    r->answer_status_ = "500 Internal Server Error"; 
    r->answer_ = "change password failed";
  } else if (result == -2) {
    r->answer_status_ = "401 Unauthorized"; 
    r->answer_ = "password is not correct";
  }

  DLog(EVENT_IMPORTANT, "Send response (XML SET): %s", r->answer_.c_str());
  return result == 0;  
}

void PageController::AccessXmlFile(HttpProto *r) {
  DLog(EVENT_IMPORTANT, "Get request (XML GET)");

  //FileUtil::LoadFile(kXmlPath.c_str(), &xmlstring); 
  if (settingMng_->GetXmlString(&(r->answer_))) {
    r->answer_content_type_ = "text/xml";
  } else { 
    r->answer_status_ = "500 Internal Server Error"; 
    r->answer_ = "!!failed to load the settings";
  }

  DLog(EVENT_IMPORTANT, "Send response: size %d", r->answer_.length());
}

bool PageController::SaveSetting(HttpProto *r) {
  std::string xmlstring = r->GetParam(HttpManager::kXmlContentParamName);

  DLog(EVENT_IMPORTANT, "Get request (XML SET): size %d", 
    xmlstring.length());

  bool result;
  result = settingMng_->SaveXml(xmlstring);

  if (result) {
    r->answer_ = "Success";
  } else {
    Util::Log(EVENT_ERROR, "!!Failed to save xml setting.");
    r->answer_status_ = "500 Internal Server Error"; 
    r->answer_ = HttpManager::kSaveFailAnswer;
  }

  DLog(EVENT_IMPORTANT, "Send response (XML SET): %s", r->answer_.c_str());
  return result;
};




void PageController::ServeRestartAction(HttpProto* r) {

  DLog(EVENT_IMPORTANT, "Get request (server restart)");
  r->answer_ = "Success";

  if (!RestartServer()) {
    Util::Log(EVENT_ERROR, "Fail to restart server.");
    r->answer_status_ = "500 Internal Server Error"; 
    r->answer_ = HttpManager::kRestartFailAnswer;
  }  

  DLog(EVENT_IMPORTANT, 
    "Send response (server restart): %s", r->answer_.c_str());

}

void PageController::AccessMainPage(HttpProto* r) {
  DLog(EVENT_IMPORTANT, "Get request (MAIN)");

  if (!SecurityManager::CheckIp(r, settingMng_->AllowRemoteAccess())){
    return;
  }

  // get user session id, if null, need login, else need check the session id.
  bool need_login;
  std::string session_id_encrypt;
  session_id_encrypt = r->GetParam(HttpProto::kSessionIdParamName);
  if (session_id_encrypt.empty() || !sessionMng_->CheckSessionLogin(r)) {
    // if it's a new session request, check if the session connections are full
    sessionMng_->RemoveExpireSession();
    if (sessionMng_->isFull()) {
      r->answer_ = "Too many sessions!!";
      return;
    }

    // set needLogin flag in main page to true
    need_login = true;
  } else {
    // set needLogin flag in main page to false
    need_login = false;
  }

  std::string htmlstring;

  // load file
  if (WebPageManager::GetMainFile(&htmlstring, session_id_encrypt, need_login)) {
    r->answer_ = htmlstring;
    DLog(EVENT_IMPORTANT, "Send response: SUCCESS, size %d", 
      r->answer_.length());
  } else {
    // TODO: set Error response code
    r->answer_status_ = "404 Not Found"; 
    r->answer_ = "can not load file";
    DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
  }    
}


bool PageController::UnittestProcess(HttpProto* r) {
  // For security reason, by pass the function when release.
#ifndef _DEBUG
  r->answer_ = "Page not found";
  return false;
#endif

  bool ret = true;

  // UNITTEST post
  if (r->path_ == kUnittestPostAction) {
    DLog(EVENT_IMPORTANT, "Get request (UNITTEST post)");

    std::string teststring = r->GetParam(HttpManager::kUnittestPostParamName);
    DLog(EVENT_IMPORTANT, "post type: %s, content: %s", 
      r->content_type_.c_str(), teststring.c_str());

    if (FileUtil::WriteFile((WebPageManager::GetLocalPath() + kUnittestPostAndGetContentPath).c_str(), teststring)) {
      r->answer_ = "Success";
    } else {
      r->answer_status_ = "500 Internal Server Error"; 
      r->answer_ = "Failed";
      ret = false;
    }

    DLog(EVENT_IMPORTANT, 
      "Send response (UNITTEST post): %s", r->answer_.c_str());

    // UNITTEST get
  } else if (r->path_ == kUnittestGetAction) {

    DLog(EVENT_IMPORTANT, "Get request (UNITTEST get)");

    std::string xmlstring;
    if (!FileUtil::LoadFile((WebPageManager::GetLocalPath() + kUnittestPostAndGetContentPath).c_str(), &xmlstring)) {
      Util::Log(EVENT_ERROR, "!!Failed to load xml setting.");
      xmlstring = "";
    }

    r->answer_content_type_ = "text/xml";
    r->answer_ = xmlstring;
    DLog(EVENT_IMPORTANT, "Send response (UNITTEST get): %s", r->answer_.c_str());

    // UNITTEST crash
  } else if (r->path_ == kUnittestCrashAction) {

    DLog(EVENT_IMPORTANT, "Get request (UNITTEST crash)");

    exit(0);
    DLog(EVENT_IMPORTANT, "Send response (UNITTEST crash): %s", r->answer_.c_str());

    // UNITTEST longtime wait
  } else if (r->path_ == kUnittestLongtimeAction) {

    DLog(EVENT_IMPORTANT, "Get request (UNITTEST longtime)");

    Sleep(10000); // 10 sec
    r->answer_ = "Success";
    DLog(EVENT_IMPORTANT, "Send response (UNITTEST longtime): %s", r->answer_.c_str());
    // restart server
  }

  return ret;
}


void PageController::Login(HttpProto *r) {
  DLog(EVENT_IMPORTANT, "Get request (Login)"); 
  if (SecurityManager::CheckIp(r, settingMng_->AllowRemoteAccess())) {
    if (SecurityManager::VerifyPasswd(r, settingMng_)) {
      DLog(EVENT_IMPORTANT, "Login success");       

      std::string session_id_encrypt = 
        SecurityManager::GenerateRandomId(settingMng_);

      sessionMng_->CreateSession(session_id_encrypt);

      std::stringstream sstream;
      sstream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><Session id=\"" 
        << session_id_encrypt << "\"></Session>";

      r->answer_ = sstream.str();
      r->answer_content_type_ = "text/xml";
    } else {
      Util::Log(EVENT_ERROR, "Login failed!!");
      r->answer_status_ = "401 Unauthorized"; 
      r->answer_ = "Failed";
    }
  } 
  DLog(EVENT_IMPORTANT, "Send response (XML SET): %s", r->answer_.c_str());
}

void PageController::Logout(HttpProto *r) {
  DLog(EVENT_IMPORTANT, "Get request (Logout)"); 
  std::string session_id = r->GetParam(HttpProto::kSessionIdParamName);
  sessionMng_->RemoveSession(session_id);
  r->answer_ = "Success";
  DLog(EVENT_IMPORTANT, "Send response (XML SET): %s", r->answer_.c_str());
}

// Other functions 

bool PageController::requireAutoRestartWebServer() {
  return true;
}

bool PageController::RestartServer() {
  bool needRestartAll = 
    requireAutoRestartWebServer() && settingMng_->CheckRestart();

  std::string args = needRestartAll ? "all" : "";
#ifdef WIN32
  std::string cmd = Util::GetApplicationDir().append("/").append("restart.bat");
  HINSTANCE h = ShellExecuteA(NULL, "open", cmd.c_str(), args.c_str(), NULL, SW_SHOWNORMAL);
  Util::Log(EVENT_CRITICAL, "restart the server!! %d", h);
  return h > (HINSTANCE)32 ? true : false;
#else
  pid_t pid;
  // vfork will not copy content from parent process, the child process must call exec immediately
  // the parent process will not run util the child process call exec
  std::string cmd = Util::GetApplicationDir().append("/").append("restart.sh");
  int ret = 0;
  if ((pid = vfork()) < 0) {
    Util::Log(EVENT_ERROR, "fork restart process failed!! %d", errno);
    return false;
  } else if (pid == 0) { // child process
    DLog(EVENT_IMPORTANT, "start exec restart.sh...");
    ret = execl(cmd.c_str(), // filepath
      cmd.c_str(), // arg0, must be the filepath
      args.c_str(), // arg1, the 'all' flag
      (char *)0); // teminator of the args list
    if (ret < 0) {
      Util::Log(EVENT_ERROR, "exec restart.sh failed!! %d", errno);
    }
    _exit(0);
  }
  Util::Log(EVENT_CRITICAL, "restart the server!! %d", ret);
  return ret == -1 ? false : true;
#endif
}
