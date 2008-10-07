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

// This file defines a class that deals with application level http for 
// Google Sitemap Generator. Up to now, it only defines some const variables
// for PageController.

#ifndef SITEMAPSERVICE_HTTPMANAGER_H__
#define SITEMAPSERVICE_HTTPMANAGER_H__

#include <string>

class HttpManager {
public:
  // HTTP parameters names
  static const std::string kXmlContentParamName;
  static const std::string kOldPasswordParamName;
  static const std::string kNewPasswordParamName;
  static const std::string kUnittestPostParamName;
  static const std::string kForceSaveParamName;
  static const std::string kXmlTimestampParamName;

  // HTTP response messages
  static const std::string kSaveFailAnswer;
  static const std::string kReloadFailAnswer;
  static const std::string kXmlWarnAnswer;

  // TODO: Refactor this class.
  // This is moved from old HttpProto.
  static const std::string kUsernameParamName;  // Username param
  static const std::string kPasswordParamName;  // Password param
  static const std::string kSIDParamName; // SID param
};
#endif // SITEMAPSERVICE_HTTPMANAGER_H__
