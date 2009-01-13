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


#include "common/httprequest.h"

#include "common/messageconverter.h"
#include "common/logger.h"
#include "common/httpconst.h"

const std::string HttpRequest::kHttpsOn = "ON";

const char* HttpRequest::kKeyRemoteAddr = "_REMOTE_ADDR";
const char* HttpRequest::kKeyPathInfo = "_PATH_INFO";
const char* HttpRequest::kKeyQueryString = "_QUERY_STRING";
const char* HttpRequest::kKeyHttpMethod = "_HTTP_METHOD";
const char* HttpRequest::kKeyHttps = "_HTTPS";
const char* HttpRequest::kKeyMessageBody = "_MESSAGE_BODY";

void HttpRequest::Reset() {
  remote_addr_.assign("");
  path_info_.assign("");
  query_string_.assign("");
  http_method_.assign("");
  message_body_.assign("");
  https_.assign("");

  headers_.clear();
}


bool HttpRequest::FromString(const std::string& str) {
  // Parse string to a string->string map.
  headers_.clear();
  if (!MessageConverter::StringToMap(str, &headers_)) {
    Logger::Log(EVENT_ERROR, "Failed to convert [%s] to HttpRequest.",
              str.c_str());
    return false;
  }

  // Get values from map for each fields.
  remote_addr_ = headers_[kKeyRemoteAddr];
  path_info_ = headers_[kKeyPathInfo];
  query_string_ = headers_[kKeyQueryString];
  http_method_ = headers_[kKeyHttpMethod];
  https_ = headers_[kKeyHttps];
  message_body_ = headers_[kKeyMessageBody];

  headers_.erase(kKeyRemoteAddr);
  headers_.erase(kKeyPathInfo);
  headers_.erase(kKeyQueryString);
  headers_.erase(kKeyHttpMethod);
  headers_.erase(kKeyHttps);
  headers_.erase(kKeyMessageBody);

  return true;
}

void HttpRequest::ToString(std::string* str) const {
  // Save all values to a string->string map.
  std::map<std::string, std::string> values = headers_;
  values[kKeyRemoteAddr] = remote_addr_;
  values[kKeyPathInfo] = path_info_;
  values[kKeyQueryString] = query_string_;
  values[kKeyHttpMethod] = http_method_;
  values[kKeyHttps] = https_;
  values[kKeyMessageBody] = message_body_;

  // Convert string->string map to a string.
  str->clear();
  MessageConverter::MapToString(values, str);
}

void HttpRequest::SetHeader(const std::string& name,
                            const std::string& value) {
  headers_[name] = value;
}

std::string HttpRequest::GetHeader(const std::string& name) {
  std::map<std::string, std::string>::iterator itr = headers_.find(name); 
  return itr != headers_.end() ? itr->second : "";
}


