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

#ifndef WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_SECURITYMANAGER_H__
#define WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_SECURITYMANAGER_H__

class HttpProto;
class SessionManager;
class SettingManager;

class SecurityManager {
public:
  // Check if the access is valid.
  static bool SecurityCheck(HttpProto *r, SessionManager* sess, 
    bool allowRemote);

  // Functions for IP

  // Checks if the IP for the request is allowed.
  static bool CheckIp(HttpProto *r, bool allowRemote);

  // Functions for password

  // Validates the login password
  static bool VerifyPasswd(HttpProto *r, SettingManager* setting);

  // Checks the access path from the HTTP request.
  // Forbidden the '../' path
  static bool CheckPath(HttpProto* r);

  // Returns random-generated session id (weak version)
  static std::string GenerateSimpleRandomId(SettingManager* setting);

  // Returns random-generated session id (strong version)
  static std::string GenerateRandomId(SettingManager* setting);

};
#endif