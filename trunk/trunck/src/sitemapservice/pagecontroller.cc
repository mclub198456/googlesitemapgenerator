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
#include <Wincrypt.h>
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
#include "common/util.h"
#include "sitemapservice/httplanguageheaderparser.h"
#include "sitemapservice/runtimeinfomanager.h"
#include "sitemapservice/httpproto.h"
#include "sitemapservice/webserver.h"

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

const std::string PageController::kLocalFilePath = "templates";
const std::string PageController::kXmlPath = "sitesettings.xml";

const std::string PageController::kUnittestPostAndGetContentPath = "/unittest/postcontent.txt";

const std::string PageController::kMainFile = "/main.html";
const std::string PageController::kXmlContentParamName = "xmlcontent";
const std::string PageController::kOldPasswordParamName = "opswd";
const std::string PageController::kNewPasswordParamName = "npswd";
const std::string PageController::kUnittestPostParamName = "postval";

const std::string PageController::kSaveFailAnswer =  "Save Failed";
const std::string PageController::kRestartFailAnswer = "Restart Failed";

const unsigned int PageController::kMaxSessionNum = 16;
const unsigned int PageController::kSessionExpireTime = 30*60; // secs, 30 mins


const PageController::ParamPattern PageController::kLoginFlagPattern = {
  "var DO_LOGIN = null",
  "var DO_LOGIN = %s", // bool
  1,
  250
};

const PageController::ParamPattern PageController::kDebugFlagPattern = {
  "var IS_DEBUG = null",
  "var IS_DEBUG = %s", // bool
  1,
  250
};

// aggressive js file to reduce the latency
const PageController::ParamPattern PageController::kScriptIncludePattern = {
  "<!--SCRIPT BEGIN-->*<!--SCRIPT END-->",
  "<script type='text/javascript' src='all.js'></script>", // bool
  0,
  250
};


/////////////////////// static class function /////////////////////////////
const std::vector<std::string>& PageController::getJSFilesForAggregation() {
  static std::vector<std::string> jsfiles;
  if (jsfiles.size() == 0) {
    jsfiles.push_back("browser.js");
    jsfiles.push_back("constval.js");
    jsfiles.push_back("utility.js");
    jsfiles.push_back("tips.js");
    jsfiles.push_back("transmarkset.js");
    jsfiles.push_back("ajax.js");
    jsfiles.push_back("servermanager.js");
    jsfiles.push_back("runtimeinfomanager.js");
    jsfiles.push_back("list.js");
    jsfiles.push_back("settingcomponents.js");
    jsfiles.push_back("sitesettings.js");
    jsfiles.push_back("xmlmanager.js");
    jsfiles.push_back("htmlmanager.js");
    jsfiles.push_back("translatemanager.js");
    jsfiles.push_back("main.js");
  }

  return jsfiles;
}

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
    // logout    
    if (controller->SecurityCheck(r)) { // check authorization
      controller->Logout(r);
    }

  } else if (r->path_ == kUnittestPostAction ||
    r->path_ == kUnittestGetAction ||
    r->path_ == kUnittestCrashAction ||
    r->path_ == kUnittestLongtimeAction) {
      // access unittest files
      controller->UnittestProcess(r);  

  } else if (r->path_ == kSaveAndRestartAction) {
    // save and restart server
    if (controller->SecurityCheck(r)) { // check authorization
      if (controller->SaveSetting(r)) {
        controller->ServeRestartAction(r);
      }
    }

  } else if (r->path_ == kChangePasswordAction) {
    // save and restart server
    if (controller->SecurityCheck(r)) { // check authorization
      controller->ChangePassword(r);
    }

  } else if (r->path_ == kXmlGetAction) {
    // get sitesetting xml file
    if (controller->SecurityCheck(r)) { // check authorization
      controller->AccessXmlFile(r);
    }    

  } else if (r->path_ == kXmlSetAction) {
    // post sitesetting xml file
    if (controller->SecurityCheck(r)) { // check authorization
      controller->SaveSetting(r);
    }    

  } else if (r->path_ == kRuntimeInfoAction) {
    // get sitesetting xml file
    if (controller->SecurityCheck(r)) { // check authorization
      controller->GetRuntimeInfo(r);
    }    

  } else if (Util::Match(r->path_, -1, "all.js")) {
    controller->AccessAggregateJSFile(r);
  } else {
    // access static files
    controller->AccessStaticFile(r);
  }
}

/////////////////////// public class methods /////////////////////////////
// Constructor
PageController::PageController() {
  isSettingsLoadedFromWebServer_ = settings_.LoadWebserverConfig();
  GetXmlSettingFromFile();

  if (isSettingsLoadedFromWebServer_) {
    const std::vector<SiteSetting>& sites = settings_.site_settings();
    for (size_t i = 0; i < sites.size(); i++) {
      initSiteEnables_.push_back(sites[i].enabled());
      initWebserverFilterEnables_.push_back(
	        sites[i].webserver_filter_setting().enabled());
    }    
  } else {
    Util::Log(EVENT_ERROR, "failed to load from web server");
  }
}

void PageController::setUpdateListener(SettingUpdateListener* listener) {
  settingListener_ = listener;
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

/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::AccessXmlFile(HttpProto *r) {
  DLog(EVENT_IMPORTANT, "Get request (XML GET)");

  //FileUtil::LoadFile(kXmlPath.c_str(), &xmlstring); 
  if (GetXmlSettingFromFile() && settings_.SaveToString(r->answer_)) {
    r->answer_content_type_ = "text/xml";
  } else { 
    r->answer_status_ = "500 Internal Server Error"; 
    r->answer_ = "!!failed to load the settings";
  }

  DLog(EVENT_IMPORTANT, "Send response: size %d", r->answer_.length());
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::SaveSetting(HttpProto *r) {
  std::string xmlstring = r->GetParam(kXmlContentParamName);

  DLog(EVENT_IMPORTANT, "Get request (XML SET): size %d", 
    xmlstring.length());

  bool result;
  result = SaveXml(xmlstring);

  if (result) {
    r->answer_ = "Success";
  } else {
    Util::Log(EVENT_ERROR, "!!Failed to save xml setting.");
    r->answer_status_ = "500 Internal Server Error"; 
    r->answer_ = kSaveFailAnswer;
  }

  DLog(EVENT_IMPORTANT, "Send response (XML SET): %s", r->answer_.c_str());
  return result;
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::ChangePassword(HttpProto* r) {
  std::string password = r->GetParam(kOldPasswordParamName);
  std::string new_password = r->GetParam(kNewPasswordParamName);

  DLog(EVENT_IMPORTANT, "Get request (PASSWORD CHANGE)");

  int result = ChangePassword(password, new_password);

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

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::CheckPath(HttpProto* r) {
  std::string::size_type pos = r->path_.find("..");
  if (pos != std::string::npos) {
    // find invalid path
    return false;
  }
  return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::AccessStaticFile(HttpProto* r) {
  DLog(EVENT_IMPORTANT, "Get request: %s", Util::EscapeLogMessage(r->path_.c_str()).c_str());

  // check the r-> path has no access to parent dir
  if (!CheckPath(r)) {
    return;
  }
  
  std::string filename;
  // path preprocess  
  if (Util::Match(r->path_, 0, "/unittest/jsunit/")) {
      // google3 jsunit2.2 lib
      filename = "../../../third_party/java/jsunit/jsunit2.2/" + 
        r->path_.substr(strlen("/unittest/jsunit/"));
      // windows local jsunit lib
      // filename = GetLocalPath() + r->path_;
  } else if (Util::Match(r->path_, -1, "language.js")) {
    // language-related files
    // get language
    filename = 
        GetLocalPath() + "scripts/languages/" + GetLanguage(r) + r->path_;

  } else if (Util::Match(r->path_, -1, ".js")) {
    filename = GetLocalPath() + "scripts" + r->path_;
  } else if (Util::Match(r->path_, -1, ".jpg")) {
    filename = GetLocalPath() + "images" + r->path_;
  } else if (Util::Match(r->path_, -1, ".gif")) {
    filename = GetLocalPath() + "images" + r->path_;
  } else if (Util::Match(r->path_, -1, ".css")) {
    filename = GetLocalPath() + "styles" + r->path_;
  } else {
    filename = GetLocalPath() + r->path_;
  }


  DLog(EVENT_IMPORTANT, "File path: %s", Util::EscapeLogMessage(filename.c_str()).c_str());

  // cache control
  if (!Util::Match(r->path_, -1, "language.js")) {  
    FileAttribute file_attr;
    if(FileUtil::GetFileAttribute(filename.c_str(), &file_attr)) {  
      if (r->CheckCaching(file_attr.last_modified)) {
        r->answer_status_ = "304 Not Modified";
        DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_status_.c_str());
        return;
      }
    }   
  }

  // load the file
  if (FileUtil::Exists(filename.c_str())) {
    std::string contentstring;

    if (FileUtil::LoadFile(filename.c_str(), &contentstring)) {
      if (Util::Match(filename, -1, ".xml")) {
        r->answer_content_type_ = "text/xml";
      } else if (Util::Match(filename, -1, ".jpg")) {
        r->answer_content_type_ = "image/jpeg";
      } else if (Util::Match(filename, -1, ".gif")) {
        r->answer_content_type_ = "image/gif";
      } else if (Util::Match(filename, -1, ".css")) {
        r->answer_content_type_ = "text/css";
      }
      r->answer_ = contentstring;
      DLog(EVENT_IMPORTANT, "Send response: SUCCESS, size %d", r->answer_.length());

    } else {
      r->answer_status_ = "404 Not Found"; 
      r->answer_ = "can't load the file";
      Util::Log(EVENT_ERROR, "!!Failed to load the file: %s", Util::EscapeLogMessage(filename.c_str()).c_str());
      DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
    }
  } else {
    r->answer_status_ = "404 Not Found"; 
    r->answer_ = "File not found";
    Util::Log(EVENT_ERROR, "!!Failed to find the file: %s", Util::EscapeLogMessage(filename.c_str()).c_str());
    DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
  }    
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::AccessAggregateJSFile(HttpProto* r) {
  DLog(EVENT_IMPORTANT, "Get request: %s", Util::EscapeLogMessage(r->path_.c_str()).c_str());

  // get last modify timestamp, it's the latest file time of all the js files
  std::string filename;
  time_t latest = 0;
  const std::vector<std::string>& jsfiles = getJSFilesForAggregation();
  std::vector<std::string>::const_iterator itr = jsfiles.begin();
  for (;itr!=jsfiles.end();itr++) {
    filename = GetLocalPath() + "scripts/" + *itr;
    DLog(EVENT_IMPORTANT, "File path: %s", Util::EscapeLogMessage(filename.c_str()).c_str());
    
    FileAttribute file_attr;
    if(!FileUtil::GetFileAttribute(filename.c_str(), &file_attr)) {
      // if one file access time cannot get, all the files should be sent
      latest = 0;
      break;
    }
    
    if(file_attr.last_modified > latest) {
      latest = file_attr.last_modified;
    }
  }

  // check the timestamp
  if (latest != 0) {
    if (r->CheckCaching(latest)) {
      r->answer_status_ = "304 Not Modified";
      DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_status_.c_str());
      return;
    }
  }

  // load the file
  std::string allcontent;
  itr = jsfiles.begin();
  for (;itr!=jsfiles.end();itr++) {
    filename = GetLocalPath() + "scripts/" + *itr;
    if (FileUtil::Exists(filename.c_str())) {
      std::string contentstring;

      if (FileUtil::LoadFile(filename.c_str(), &contentstring)) {
        allcontent.append(contentstring);

      } else {
        r->answer_status_ = "404 Not Found"; 
        r->answer_ = "can't load the file";
        Util::Log(EVENT_ERROR, "!!Failed to load the file: %s", Util::EscapeLogMessage(filename.c_str()).c_str());
        DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
        return;
      }
    } else {
      r->answer_status_ = "404 Not Found"; 
      r->answer_ = "File not found";
      Util::Log(EVENT_ERROR, "!!Failed to find the file: %s", Util::EscapeLogMessage(filename.c_str()).c_str());
      DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
      return;
    }
  }

  r->answer_ = allcontent;
  DLog(EVENT_IMPORTANT, "Send response: SUCCESS, size %d", r->answer_.length());
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::ServeRestartAction(HttpProto* r) {

  DLog(EVENT_IMPORTANT, "Get request (server restart)");
  r->answer_ = "Success";

  if (!RestartServer()) {
    Util::Log(EVENT_ERROR, "Fail to restart server.");
    r->answer_status_ = "500 Internal Server Error"; 
    r->answer_ = kRestartFailAnswer;
  }  

  DLog(EVENT_IMPORTANT, 
    "Send response (server restart): %s", r->answer_.c_str());

}


/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::AccessMainPage(HttpProto* r) {
  DLog(EVENT_IMPORTANT, "Get request (MAIN)");

  if (!CheckIp(r)){
    return;
  }

  // get user session id, if null, need login, else need check the session id.
  bool need_login;
  std::string session_id_encrypt;
  session_id_encrypt = r->GetParam(HttpProto::kSessionIdParamName);
  if (session_id_encrypt.empty() || !CheckSessionLogin(r)) {
    // set needLogin flag in main page to true
    need_login = true;
  } else {
    // set needLogin flag in main page to false
    need_login = false;
  }
  
  std::string filename = GetLocalPath() + kMainFile;

  std::string htmlstring;

  // load file
  if (FileUtil::LoadFile(filename.c_str(), &htmlstring)) {
    RemoveExpireSession();
    if (sessions_.size() >= kMaxSessionNum) {
      r->answer_ = "Too many sessions!!";
      return;
    }

    // set param in main HTML page
    FillMainPageParam(&htmlstring, session_id_encrypt, need_login);
    r->answer_ = htmlstring;
    DLog(EVENT_IMPORTANT, "Send response: SUCCESS, size %d", 
      r->answer_.length());
  } else {
    // TODO: set Error response code
    r->answer_status_ = "404 Not Found"; 
    r->answer_ = "can not load file";
    Util::Log(EVENT_ERROR,
      "!!Failed to load main html file: %s", 
      filename.c_str());
    DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
  }    
}

bool PageController::FillMainPageParam(std::string* html, 
                                       const std::string& session_id, 
                                       bool login) {
  // session id
  StringVector params;

  // login flag
  params.clear();
  params.push_back(login ? "true" : "false");
  if (!InsertParamToHtml(html, kLoginFlagPattern, params)) {
    Util::Log(EVENT_ERROR, "Set login flag to main page failed!!");
    return false;
  }

  // debug flag
  params.clear();

#ifdef _DEBUG
  params.push_back("true");
  if (!InsertParamToHtml(html, kDebugFlagPattern, params)) {
    Util::Log(EVENT_ERROR, "Set debug flag to main page failed!!");
    return false;
  }
#endif 

#ifndef _DEBUG
  // js script file 'all.js'
  if (!InsertParamToHtml(html, kScriptIncludePattern, params)) {
    Util::Log(EVENT_ERROR, "Replace js file to main page failed!!");
    return false;
  }
#endif

  return true;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
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

    std::string teststring = r->GetParam(kUnittestPostParamName);
    DLog(EVENT_IMPORTANT, "post type: %s, content: %s", 
      r->content_type_.c_str(), teststring.c_str());

    if (FileUtil::WriteFile((GetLocalPath() + kUnittestPostAndGetContentPath).c_str(), teststring)) {
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
    if (!FileUtil::LoadFile((GetLocalPath() + kUnittestPostAndGetContentPath).c_str(), &xmlstring)) {
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



/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::CheckIp(HttpProto *r) {
  // check if it is local
  if (r->remote_ip_ != "127.0.0.1") {
    // not local
    if (!AllowRemoteAccess()){
      Util::Log(EVENT_ERROR, "Remote access is not allowed!");
      r->answer_status_ = "401 Unauthorized"; 
      r->answer_ = "Remote access is not allowed!";
      return false;
    }
  }
  return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
PageController::SessionInfo* PageController::GetSession(HttpProto *r) {
  std::string session_id = r->GetParam(HttpProto::kSessionIdParamName);
  RemoveExpireSession();
  HashTable::const_iterator it = sessions_.find(session_id);
  if (it != sessions_.end()) {
    DLog(EVENT_IMPORTANT, "access session %s", session_id.c_str()); 
    it->second->last_access = time(NULL);
    return it->second;
  }
  Util::Log(EVENT_ERROR, "!!invalid session %s", Util::EscapeLogMessage(session_id.c_str()).c_str()); 
  return NULL;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::VerifyPasswd(HttpProto *r) {
  // get username and password
  std::string username = r->GetParam(HttpProto::kUsernameParamName);
  std::string password = r->GetParam(HttpProto::kPasswordParamName);

  // check if settings is loaded
  if (!GetXmlSettingFromFile()) {
    Util::Log(EVENT_ERROR, "can not load xml for login"); 
    return false;
  }

  // encrypt the input password  
  std::string encPassword;
  if (!Util::MD5Encrypt(password.c_str(), &encPassword)) {
    Util::Log(EVENT_ERROR, "can not encrypt password for login"); 
    return false;
  }

  // check password MD5 and username
  return settings_.admin_password() == encPassword && 
         settings_.admin_name() == username;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::SecurityCheck(HttpProto *r) {
  // security check
  if (!CheckIp(r)){
    return false;
  }

  if (!CheckSessionLogin(r)) {
    r->answer_status_ = "401 Unauthorized"; 
    r->answer_ = "Invalid session, you may do login first";
    return false;
  }
  return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::Login(HttpProto *r) {
  DLog(EVENT_IMPORTANT, "Get request (Login)"); 
  if (CheckIp(r)) {
    if (VerifyPasswd(r)) {
      DLog(EVENT_IMPORTANT, "Login success"); 
      std::string session_id_encrypt = CreateSession();
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

/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::Logout(HttpProto *r) {
  DLog(EVENT_IMPORTANT, "Get request (Logout)"); 
  std::string session_id = r->GetParam(HttpProto::kSessionIdParamName);
  HashTable::iterator it = sessions_.find(session_id);
  if (it != sessions_.end()) {
    delete it->second;
    sessions_.erase(it);
  }
  r->answer_ = "Success";
  DLog(EVENT_IMPORTANT, "Send response (XML SET): %s", r->answer_.c_str());
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::GetXmlSettingFromFile() {
  //Begin to create default XML object.

  if (!isSettingsLoadedFromWebServer_) {
    return false;
  }
  
  const std::string filepath = SiteSettings::GetDefaultFilePath();
  FileAttribute attr;
  if (!FileUtil::GetFileAttribute(filepath.c_str(), &attr)) {
    Util::Log(EVENT_ERROR, "failed to get file last modify time");
    return false;
  }

  // Checks if the setting is up-to-date.
  if (!isSettingsLoadedFromFile_ || 
      lastModifyTimeForSettingsFile_ != attr.last_modified) {
    lastModifyTimeForSettingsFile_ = attr.last_modified;
    isSettingsLoadedFromFile_ = settings_.LoadFromFile(filepath.c_str());
  }

  return isSettingsLoadedFromFile_;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::AllowRemoteAccess() {
  if (!GetXmlSettingFromFile()) {
    // if can't get the exact value, we assume it is not allowed.
    return false;
  }

  return settings_.remote_admin();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::SaveXml(const std::string& xml_string) {
  // check settings load from file
#ifdef _DEBUG
    FILE* fd = fopen("xml.txt", "w");
    fwrite(xml_string.c_str(), sizeof(char), xml_string.length(), fd);
    fclose(fd);
#endif

  if (!GetXmlSettingFromFile()) {
    return false;
  }

  // load new settings from client XML string
  //SiteSettings newsettings;
  //if (!newsettings.LoadWebserverConfig() ||
  //    !newsettings.LoadFromString(xml_string.c_str())) {
  //  return false;
  //}
  SiteSettings oldsettings = settings_;
  if (!settings_.LoadFromString(xml_string.c_str())) {
    Util::Log(EVENT_CRITICAL, "LoadFromString failed, need restart!!");
    isSettingsLoadedFromFile_ = false;
    return false;
  }

  // check if user change the password
  if (settings_.admin_password() != oldsettings.admin_password()) {
    // since javascript will not encrypt the new password if user change the 
    // password, so check again in case the password is plain-text.   
    if (!settings_.ChangeAdminPassword(settings_.admin_password().c_str())) {
      // Encrypt failed
      isSettingsLoadedFromFile_ = false;
      return false;
    }

    if (settings_.admin_password() !=  oldsettings.admin_password()) {
      // if password has been changed, encrypt plain-text Password to MD5
      Util::Log(EVENT_ERROR, "password has changed in a wrong way!!");
      isSettingsLoadedFromFile_ = false;
      return false;
    }
  }    


  // save new settings to settings_, direct copy will cause problem.
  //settings_ = newsettings;  

  if (!settingListener_->PreUpdateSetting(oldsettings, settings_)) {
    Util::Log(EVENT_ERROR, "Pre-update setting failed.");
    isSettingsLoadedFromFile_ = false;
    return false;
  }

  // save settings_ to file
  bool save_result = SaveSettingToFile();
  isSettingsLoadedFromFile_ = save_result;
  if (!settingListener_->PostUpdateSetting(oldsettings, settings_, save_result)) {
    Util::Log(EVENT_ERROR, "Post-update setting failed. (Ignore).");
  }

  return save_result;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::SaveSettingToFile() {
  // Preupdate
  // Backup old settings first.
  if (!SiteSettings::BackupDefaultSettingFile()) {
    return false;
  }

  // save new settings to file
  const std::string filepath = SiteSettings::GetDefaultFilePath();
  if (!settings_.SaveToFile(filepath.c_str())) {
    return false;
  }

  FileAttribute attr;
  if (FileUtil::GetFileAttribute(filepath.c_str(), &attr)) {
    lastModifyTimeForSettingsFile_ = attr.last_modified;
  } else {
    Util::Log(EVENT_ERROR, "failed to get file last modify time");
  }

  return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
int PageController::ChangePassword(const std::string& password, 
                                    const std::string& new_password) {
  // verify the old password
  std::string enc_password;
  if (!Util::MD5Encrypt(password.c_str(), &enc_password)) {
    // Encrypt failed
    return -1;
  }

  if (enc_password != settings_.admin_password()) {
    // not correct old password
    Util::Log(EVENT_ERROR, "change password failed, old password is invalid!!");
    return -2;
  }

  // set the new password, pass the plain value to the function
  if (!settings_.ChangeAdminPassword(new_password.c_str())) {
    // change password failed
    return -1;
  }

  if (!SaveSettingToFile()) {
    return -1;
  }

  return 0;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::InsertParamToHtml(std::string* html_string, 
                                       const ParamPattern &param_pattern, 
                                       StringVector params) {
  // Generate the Replace string
  // TODO: implement multiple params format
  std::string replace_string;  
  if (param_pattern.param_num == 1) {
    char* temp_str = new char[param_pattern.max_length];
    sprintf(temp_str, param_pattern.replace_pattern.c_str(), params[0].c_str());
    replace_string = temp_str;
    delete [] temp_str;
  } else if (param_pattern.param_num == 0) {
    replace_string = param_pattern.replace_pattern;
  }
  

  // do replacement for the HTML page
  std::string::size_type pos;
  const std::string& pattern = param_pattern.find_pattern;
  // only support single '*' in find pattern, can use "\\*" to ignore the '*'
  if ((pos=pattern.find("*")) != std::string::npos &&
      (pos == 0 || pattern[pos-1] != '\\')) {

    std::string beginStr = pattern.substr(0, pos);
    std::string endStr = pattern.substr(pos + 1);
    std::string::size_type beginPos, endPos;
    
    pos = 0;
    if ((beginPos=html_string->find(beginStr, pos)) != std::string::npos) {
      if ((endPos=html_string->find(endStr, beginPos)) != std::string::npos) {
        html_string->replace(beginPos, endPos - beginPos + endStr.size(), 
                             replace_string);
        pos = beginPos + replace_string.size();
        return true;
      }
    }
    return false;
  } else { // no '*'
    int count = StringReplace(html_string, param_pattern.find_pattern, 
      replace_string);

    return count > 0;
  }
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
int PageController::StringReplace(std::string* strBig, 
                                  const std::string & strsrc, 
                                  const std::string &strdst) {
  std::string::size_type pos = 0;
  std::string::size_type srclen=strsrc.size();
  std::string::size_type dstlen=strdst.size();
  int count = 0;
  while( (pos=strBig->find(strsrc, pos)) != std::string::npos){
    strBig->replace(pos, srclen, strdst);
    pos += dstlen;
    count++;
  }
  return count;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
std::string PageController::GenerateSimpleRandomId() {
  srand((unsigned)time(NULL));

  int id = rand();
  std::ostringstream ostr;
  ostr << id;
  ostr << settings_.admin_password();
  std::string sid = ostr.str();

  // session id is MD5(randomValue + MD5(password))
  if(!Util::MD5Encrypt(sid.c_str(), &sid)) {
    Util::Log(EVENT_ERROR, "do MD5Encrypt failed");
  }

  return sid;
}

std::string PageController::GenerateRandomId() {
#ifdef WIN32
  HCRYPTPROV   hCryptProv;
  BYTE         pbData[16];
  std::string  sid = "";

  do {
    //  Acquire a cryptographic provider context handle.
    if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) {    
      DLog(EVENT_IMPORTANT, "CryptAcquireContext succeeded. \n");    
    } else {
      Util::Log(EVENT_ERROR, "Error during CryptAcquireContext!\n");
      break;
    }

    // Generate a random initialization vector.
    if(CryptGenRandom(hCryptProv, 8, pbData)) {
      DLog(EVENT_IMPORTANT, "Random sequence generated. \n");   
    } else {
      Util::Log(EVENT_ERROR, "Error during CryptGenRandom.\n");
      break;
    }

    // Encrypt the vector
    pbData[8] = 0;
    if(!Util::MD5Encrypt((const char*)pbData, &sid)) {
      Util::Log(EVENT_ERROR, "do MD5Encrypt failed");
      break;
    }
  } while(false);

  // release resource
  if(hCryptProv) {
    if(!(CryptReleaseContext(hCryptProv,0)))
      Util::Log(EVENT_ERROR, "Error during CryptReleaseContext");
  }

  // return result
  return sid.length() > 0 ? sid : GenerateSimpleRandomId();

  
#else
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    Util::Log(EVENT_ERROR, "open /dev/urandom failed");
    return GenerateSimpleRandomId();
  }

  char buf[16];
  int nbytes = 8;
  int ret = read(fd, buf, nbytes);
  if (ret < nbytes) { // fail to read
    Util::Log(EVENT_ERROR, "read /dev/urandom failed");
    return GenerateSimpleRandomId();
  }

  buf[8] = 0;
  close(fd);

  std::string sid;
  if(!Util::MD5Encrypt(buf, &sid)) {
    Util::Log(EVENT_ERROR, "do MD5Encrypt failed");
    return GenerateSimpleRandomId();
  }
  return sid;

#endif
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
std::string PageController::CreateSession() {
  std::string session_id = GenerateRandomId();

  SessionInfo* sess = new SessionInfo;
  sess->session_id = session_id;
  sess->last_access = time(NULL);
  
  sessions_[session_id] = sess;

  return session_id;
}

std::string PageController::GetLanguage(HttpProto* r) {
  // get user prefer language
  std::string language = r->GetParam("hl");
  if (language == "" || 
    !HttpLanguageHeaderParser::isSupportedLanguage(language)) {
      language = 
        HttpLanguageHeaderParser::getPreferLanguage(r->accept_language_);
  } else {
    language = 
      HttpLanguageHeaderParser::getLanguageRegularName(language);
  }
  return language;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::CheckSessionLogin(HttpProto* r) {
  SessionInfo* sess = GetSession(r);
  return sess != NULL;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void PageController::RemoveExpireSession() {
  HashTable::iterator it = sessions_.begin();
  while (it != sessions_.end()) {
    if (time(NULL) - it->second->last_access > kSessionExpireTime) {
       delete it->second;
       sessions_.erase(it);
       it = sessions_.begin();
    } else {
      ++it;
    }
  }
}

bool PageController::requireAutoRestartWebServer() {
  return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool PageController::RestartServer() {
  bool needRestartAll = false;
  if (requireAutoRestartWebServer()) {
    const std::vector<SiteSetting>& sites = settings_.site_settings();
    for (size_t i = 0; i < sites.size(); i++) {
      bool oldEnable = initSiteEnables_[i] && initWebserverFilterEnables_[i];
      bool newEnable = sites[i].enabled() && 
                       sites[i].webserver_filter_setting().enabled();
      if (oldEnable != newEnable) {
          needRestartAll = true;
          break;
      }
    }
  }
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

/************************************************************************/
/*                                                                      */
/************************************************************************/
std::string PageController::GetLocalPath() {
#ifdef _DEBUG
  // restart from command should failed before the unittest component checkin.

  // It allows the server access the UI js files directly, so developers don't
  // need to build the solution each time they change the js files.
  // It also allows the access to custom unittest HTML files and the external 
  // jsunit lib on the Windows platform.
  
  FILE* fd = fopen("config", "r");
  if (fd == NULL)
    return kLocalFilePath + "/";

  char buff[250];
  size_t numread = fread( buff, sizeof( char ), 250, fd );
  if (ferror(fd)) {
    fclose(fd);
    return kLocalFilePath + "/";
  } else {
    fclose(fd);
  }

  buff[numread] = 0;

  std::string path;
  path.append(buff).append("/");
  return path;
#else
  return Util::GetApplicationDir().append("/")
                                  .append(kLocalFilePath)
                                  .append("/");
#endif
}
