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

// This file defines a class to deal with the issues related to security.
// It checks the user session id and IP address in the http request. It also
// verifies the user login password, checks the static pages' access path, and
// provides strong random-generated session id.

#ifndef SITEMAPSERVICE_SECURITYMANAGER_H__
#define SITEMAPSERVICE_SECURITYMANAGER_H__

#include <string>
#include <set>
#include "sitemapservice/httpcontext.h"

class HttpSettingManager;
class SessionManager;

class SecurityManager {
 public:
  SecurityManager() {}
  ~SecurityManager();

  bool Initialize(HttpSettingManager* setting_manager,
                  SessionManager* session_manager);

  bool Check(HttpContext* context);

  // Register private page to this security manager.
  // These pages require login.
  void RegisterPrivatePage(const std::string& path);

  // TODO: These two methods should be moved to other places.
  // Returns random-generated session id (weak version)
  static std::string GenerateSimpleRandomId(const std::string& seed);

  // Returns random-generated session id (strong version)
  static std::string GenerateRandomId(const std::string& seed);

 private:
  // Check remote client.
  bool CheckRemoteClient(const HttpRequest& request);

  // Check path to exclude ".." 
  bool CheckPath(const std::string& path);

  HttpSettingManager* setting_manager_;
  SessionManager* session_manager_;
  std::set<std::string> local_addrs_;
  std::set<std::string> private_pages_; 
};
#endif // SITEMAPSERVICE_SECURITYMANAGER_H__


