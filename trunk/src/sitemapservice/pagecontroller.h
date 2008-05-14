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

// This file defines a class to deal with the HTTP request from admin console
// of Google Sitemap Generator.

#ifndef SITEMAPSERVICE_PAGECONTROLLER_H__
#define SITEMAPSERVICE_PAGECONTROLLER_H__

#include <time.h>
#include <string>
#include <vector>

#include "common/util.h"

class HttpProto;
class SessionManager;
class SettingManager;
class SettingUpdateListener;

// deal with the HTTP request from config UI
class PageController { 

public:

  // The function that dispatch the HTTP requests.
  static void PageControl(HttpProto* r);

  // singleton factory
  static PageController* GetInstance();

  // Set the XML config file update listener. It pass the "listener" to the 
  // SettingManager object
  void SetUpdatelistener(SettingUpdateListener* listener);

private:
  PageController(); // forbidden creating instances directly

  // Functions for request handler

  // Provides some unittests' stubs
  bool UnittestProcessHandler(HttpProto* r);

  // Accesses main.html
  void AccessMainPageHandler(HttpProto* r);

  // Accesses the XML config file
  void AccessXmlFileHandler(HttpProto *r);

  // User login
  void LoginHandler(HttpProto *r);

  // User logout
  void LogoutHandler(HttpProto* r);

  // Saves setting
  bool SaveSettingHandler(HttpProto *r);

  // Restarts server
  void ServeRestartHandler(HttpProto* r);

  // Changes the login password
  bool ChangePasswordHandler(HttpProto* r);

  // Gets runtime information
  void AccessRuntimeInfoHandler(HttpProto *r);  

  // Other functions 

  // Execute the restart-server script in a new created process.
  bool RestartServer();

  // URIs
  static const std::string kRuntimeInfoAction;
  static const std::string kXmlGetAction;
  static const std::string kXmlSetAction;
  static const std::string kRestartAction;
  static const std::string kLogoutAction;
  static const std::string kLoginAction;
  static const std::string kChangePasswordAction;
  static const std::string kUnittestPostAction;
  static const std::string kUnittestGetAction;
  static const std::string kUnittestCrashAction;
  static const std::string kUnittestLongtimeAction;
  static const std::string kUnittestPostAndGetContentPath;
  static const std::string kMainAction;

  // The singleton instance
  static PageController* instance_;

  SessionManager* session_manager_;
  SettingManager* setting_manager_;
};


#endif
