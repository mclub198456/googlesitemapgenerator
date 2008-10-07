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

// HttpResponse encapsulate http response information used in this application.
// This class is not thread safe.

#ifndef COMMON_HTTPRESPONSE_H__
#define COMMON_HTTPRESPONSE_H__

#include <map>
#include <string>

class HttpResponse {
public:
  HttpResponse() {}
  ~HttpResponse() {}

  // Clear all information.
  void Reset();

  // Clear all information,
  // and set status line and message content after clearing.
  // Note, the content type is assuemed to be "text/plain; charset=utf-8".
  void Reset(const std::string& status, const std::string& message);

  // Parse information from a string.
  // Returns false if given string is in bad format.
  bool FromString(const std::string& str);

  // Convert this object to string representation.
  void ToString(std::string* str) const;

  // Get/set http status line.
  // It shoud be like "200 OK".
  std::string status() const {
    return status_;
  }
  void set_status(const std::string& status) {
    status_= status;
  }

  // Get set message body (http content).
  const std::string& message_body() const {
    return message_body_;
  }
  void set_message_body(const std::string& message_body) {
    message_body_ = message_body;
  }

  // Get all response headers.
  const std::map<std::string, std::string>& headers() const {
    return headers_;
  }

  // Get/set header with given name.
  void SetHeader(const std::string& name, const std::string& value);
  std::string GetHeader(const std::string& name);

private:
  // Key names used to convert corresponding member variables to string map.
  // These constants are used in ParseFrom and ToString methods.
  static const char* kKeyStatus;
  static const char* kKeyMessageBody;

  std::string status_;
  std::string message_body_;
  std::map<std::string, std::string> headers_;
};

#endif // COMMON_HTTPRESPONSE_H__

