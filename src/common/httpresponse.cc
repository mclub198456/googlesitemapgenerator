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


#include "common/httpresponse.h"

#include "common/port.h"
#include "common/logger.h"
#include "common/messageconverter.h"
#include "common/httpconst.h"

const char* HttpResponse::kKeyStatus = "_STATUS";
const char* HttpResponse::kKeyMessageBody = "_MESSAGE_BODY";

void HttpResponse::SetHeader(const std::string& name,
                             const std::string& value) {
  headers_[name] = value;
}

std::string HttpResponse::GetHeader(const std::string& name) {
  std::map<std::string, std::string>::iterator itr = headers_.find(name); 
  return itr != headers_.end() ? itr->second : "";
}

void HttpResponse::Reset() {
  status_ = HttpConst::kStatus200;
  message_body_.assign("");
  
  headers_.clear();
}

void HttpResponse::Reset(const std::string& status, const std::string& msg) { 
  status_ = status;
  headers_.clear();

  if (msg.length() != 0) {
    SetHeader(HttpConst::kHeaderContentType, "text/plain; charset=utf-8");
    set_message_body(msg);
  } else {
    message_body_.assign("");
  }
}

bool HttpResponse::FromString(const std::string& str) {
  // Parse string to a string->string map.
  headers_.clear();
  if (!MessageConverter::StringToMap(str, &headers_)) {
    Logger::Log(EVENT_ERROR, "Failed to convert [%s] to HttpResponse.",
              str.c_str());
    return false;
  }

  // Retrieve field values from map.
  status_ = headers_[kKeyStatus];
  message_body_ = headers_[kKeyMessageBody];

  headers_.erase(kKeyStatus);
  headers_.erase(kKeyMessageBody);

  return true;
}

void HttpResponse::ToString(std::string* str) const {
  // Save all field values to a string->string map.
  std::map<std::string, std::string> values = headers_;
  values[kKeyStatus] = status_;
  values[kKeyMessageBody] = message_body_;

  // Convert the string->string map to a string.
  str->clear();
  MessageConverter::MapToString(values, str);
}


