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
#include "sitemapservice/httpcontext.h"

class SessionManager;
class HttpSettingManager;
class SecurityManager;
class PageHandler;
class SettingUpdateListener;

class PageController { 

public:
  PageController();
  ~PageController();

  bool Initialize();

  void Process(HttpContext* context);

  HttpSettingManager* setting_manager() {
    return setting_manager_;
  }

  static PageController* instance() {
    return &instance_;
  }

private:
  static const std::string kRuntimeInfoAction;
  static const std::string kXmlGetAction;
  static const std::string kXmlSetAction;
  static const std::string kReloadAction;
  static const std::string kLogoutAction;
  static const std::string kLoginAction;
  static const std::string kChangePasswordAction;
  static const std::string kMainAction;
  static const std::string kMessageBundleAction;

  void RegisterHandler(const std::string& path, PageHandler* handler,
                       bool is_private);

  static PageController instance_;

  SessionManager* session_manager_;
 
  HttpSettingManager* setting_manager_;

  SecurityManager* security_manager_;

  std::map<std::string, PageHandler*> handlers_;
};

#endif
