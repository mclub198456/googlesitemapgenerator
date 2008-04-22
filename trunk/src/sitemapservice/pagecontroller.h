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

#ifndef SITEMAPSERVICE_PAGECONTROLLER_H__
#define SITEMAPSERVICE_PAGECONTROLLER_H__

#include <time.h>
#include <string>
#include <vector>

#include "common/hashmap.h"
#include "common/sitesettings.h"
#include "sitemapservice/settingupdatelistener.h"

class HttpProto;

// deal with the HTTP request from config UI
class PageController { 

  // data structure for each session
  struct SessionInfo {
    std::string session_id;
    bool is_login;
    time_t last_access;
    std::string language_;
  };

  // Hash table for sessions
  typedef HashMap<std::string, SessionInfo*>::Type HashTable;

 public:

  // The function that dispatch the HTTP requests.
  static void PageControl(HttpProto* r);

  // singleton factory
  static PageController* getInstance();

  // Set the XML config file update listener, so that the Generator service
  // can be informed online when the XML configuration is updated.
  void setUpdateListener(SettingUpdateListener* listener);
 
 private:
  PageController(); // forbidden creating instances directly

  // Returns the paths of all the JavaScript files that will be included in the 
  // main.html, will merge them into 'all.js'.
  static const std::vector<std::string>& getJSFilesForAggregation();

  // Replaces all the 'strsrc' sub-strings in the 'strBig' with 'strdst' string.
  // Returns the number of the 'strsrc' being replaced.
  static int StringReplace(std::string* strBig, 
                           const std::string & strsrc, 
                           const std::string &strdst);

  // Functions for request handler

  // Provides some unittests' stubs
  bool UnittestProcess(HttpProto* r);

  // Accesses main.html
  void AccessMainPage(HttpProto* r);

  // Accesses the static web files such as JavaScript/image/CSS 
  void AccessStaticFile(HttpProto *r);

  // Accesses the 'all.js' file, which is an aggregation of the JS files in
  // main.html
  void AccessAggregateJSFile(HttpProto *r);

  // Accesses the XML config file
  void AccessXmlFile(HttpProto *r);

  // User login
  void Login(HttpProto *r);

  // User logout
  void Logout(HttpProto* r);

  // Saves setting
  bool SaveSetting(HttpProto *r);

  // Restarts server
  void ServeRestartAction(HttpProto* r);

  // Gets runtime information
  void GetRuntimeInfo(HttpProto *r);


  // Creates the session, return the new session id
  std::string CreateSession();

  // Fetches session id from the request and get the session info from server
  SessionInfo* GetSession(HttpProto *r);

  // Removes the expired sessions
  void RemoveExpireSession();

  // Checks if the session has login
  bool CheckSessionLogin(HttpProto* r);

  // Check if the access is valid.
  bool SecurityCheck(HttpProto *r);

  // Functions for IP

  // Checks if remote access is allowed in config file.
  bool AllowRemoteAccess();

  // Checks if the IP for the request is allowed.
  bool CheckIp(HttpProto *r);

  // Functions for password
  
  // Validates the login password
  bool VerifyPasswd(HttpProto *r);
  
  // Changes the login password
  bool ChangePassword(HttpProto* r);


  // The pattern for parameters that will be inserted into main.html
  // It's a way to pass some data to UI web files
  struct ParamPattern {
    std::string find_pattern;
    std::string replace_pattern;
    int param_num;
    int max_length;
  };

  // String vector
  typedef std::vector<std::string> StringVector;
  
  // Inserts the parameters into HTML string
  bool InsertParamToHtml(std::string* html_string, 
                         const ParamPattern& param_pattern, 
                         StringVector params);
  
  // Inserts the session id and login flag into the main.html
  bool FillMainPageParam(std::string* html, 
                         const std::string& session_id, 
                         bool login);

  // Other functions
  // Returns the root path of web files
  std::string GetLocalPath();

  // Returns the user's prefer language
  std::string GetLanguage(HttpProto* r);

  // Checks the access path from the HTTP request.
  // Forbidden the '../' path
  bool CheckPath(HttpProto* r);

  // Saves XML configuration, does some preprocess
  bool SaveXml(const std::string& xml_string);

  // Saves XML configuration to file
  bool SaveSettingToFile();

  // Changes password for login
  int ChangePassword(const std::string& password, 
                     const std::string& new_password);

  // Gets XML configuration from file
  bool GetXmlSettingFromFile();

  // Returns random-generated session id (weak version)
  std::string GenerateSimpleRandomId();
  
  // Returns random-generated session id (strong version)
  std::string GenerateRandomId();

  // Returns if user want the IIS/apache server being restart automatically.
  bool requireAutoRestartWebServer();

  // Execute the restart-server script in a new created process.
  bool RestartServer();

  // URIs
  static const std::string kRuntimeInfoAction;
  static const std::string kXmlGetAction;
  static const std::string kXmlSetAction;
  static const std::string kSaveAndRestartAction;
  static const std::string kLogoutAction;
  static const std::string kLoginAction;
  static const std::string kChangePasswordAction;
  static const std::string kUnittestPostAction;
  static const std::string kUnittestGetAction;
  static const std::string kUnittestCrashAction;
  static const std::string kUnittestLongtimeAction;
  static const std::string kUnittestPostAndGetContentPath;
  static const std::string kMainAction;

  // file paths
  static const std::string kXmlPath;
  static const std::string kMainFile;
  static const std::string kLocalFilePath;

  // HTTP parameters names
  static const std::string kXmlContentParamName;
  static const std::string kOldPasswordParamName;
  static const std::string kNewPasswordParamName;
  static const std::string kUnittestPostParamName;

  // HTTP response messages
  static const std::string kSaveFailAnswer;
  static const std::string kRestartFailAnswer;

  // ParamPatterns for main.html
  static const ParamPattern kLoginFlagPattern;
  static const ParamPattern kDebugFlagPattern;
  static const ParamPattern kScriptIncludePattern;

  static const unsigned int kMaxSessionNum;
  static const unsigned int kSessionExpireTime; // secs, 30 mins

  // The singleton instance
  static PageController* instance_;

  // The sessions table
  HashTable sessions_;

  // The XML configuration content
  // Each HTTP request will trigger a load in case the XML config file has been
  // modified manually.
  SiteSettings settings_; // since the web server is running in single 
                          // thread, so we don't need to lock it.

  // True if loading configuration info from webserver succeed.
  bool isSettingsLoadedFromWebServer_;

  // True if the configuration 'settings_' is up-to-date compared to the file
  bool isSettingsLoadedFromFile_;

  // The timestamp for the configuration 'settings_'
  time_t lastModifyTimeForSettingsFile_;

  // The listener for the setting change
  SettingUpdateListener* settingListener_;

  // These two set of settings will be used to judge the necessary to restart
  // the IIS/Apache server.

  // The initial site enable settings
  std::vector<bool> initSiteEnables_;

  // The initial site filter settings
  std::vector<bool> initWebserverFilterEnables_;
};


#endif
