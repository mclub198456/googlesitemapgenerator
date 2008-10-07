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

#include "sitemapservice/httpmanager.h"

const std::string HttpManager::kXmlContentParamName = "xmlcontent";
const std::string HttpManager::kOldPasswordParamName = "opswd";
const std::string HttpManager::kNewPasswordParamName = "npswd";
const std::string HttpManager::kUnittestPostParamName = "postval";
const std::string HttpManager::kForceSaveParamName = "force";
const std::string HttpManager::kXmlTimestampParamName = "ts";

const std::string HttpManager::kSaveFailAnswer =  "Save Failed";
const std::string HttpManager::kReloadFailAnswer = "Reload Failed";
const std::string HttpManager::kXmlWarnAnswer = "Settings is out-of-date";

const std::string HttpManager::kUsernameParamName = "username";
const std::string HttpManager::kPasswordParamName = "password";
const std::string HttpManager::kSIDParamName = "sid";
