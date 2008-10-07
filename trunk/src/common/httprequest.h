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

// HttpRequest encapsulate http request information used in this application.
// This class is not thread safe.

#ifndef COMMON_HTTPREQUEST_H__
#define COMMON_HTTPREQUEST_H__

#include <map>
#include <string>

class HttpRequest {
public:
  HttpRequest() {}
  ~HttpRequest() {}

  // Clear all information in it.
  void Reset();

  // Parse request information from given string.
  // Returns false if given string is malformed.
  bool FromString(const std::string& str);

  // Convert this request to string representation.
  void ToString(std::string* str) const;

  // Get/set address of remote client.
  const std::string& remote_addr() const {
    return remote_addr_;
  }
  void set_remote_addr(const std::string remote_addr) {
    remote_addr_ = remote_addr;
  }

  // Get/set request path information.
  // It doesn't contain query information.
  const std::string& path_info() const {
    return path_info_;
  }
  void set_path_info(const std::string& path_info) {
    path_info_ = path_info;
  }

  // Get/set query string information.
  const std::string& query_string() const {
    return query_string_;
  }
  void set_query_string(const std::string& query_string) {
    query_string_ = query_string;
  }

  // Get/set http method used by this request.
  // It could be GET, POST and etc.
  const std::string& http_method() const {
    return http_method_;
  }
  void set_http_method(const std::string& http_method) {
    http_method_ = http_method;
  }

  // Get/set message body (http content).
  const std::string& message_body() const {
    return message_body_;
  }
  void set_message_body(const std::string& message_body) {
    message_body_ = message_body;
  }

  const std::string& https() const {
    return https_;
  }
  void set_https(const std::string& https) {
    https_ = https;
  }

  // Get all headers.
  const std::map<std::string, std::string>& headers() {
    return headers_;
  }

  // Get/set header with given name.
  void SetHeader(const std::string& name, const std::string& value);
  std::string GetHeader(const std::string& name);

  static const std::string kHttpsOn;

private:
  // Key names used to convert corresponding member variables to string map.
  // These constants are used in ParseFrom and ToString methods.
  static const char* kKeyRemoteAddr;
  static const char* kKeyPathInfo;
  static const char* kKeyQueryString;
  static const char* kKeyHttpMethod;
  static const char* kKeyHttps;
  static const char* kKeyMessageBody;

  std::string https_;
  std::string remote_addr_;
  std::string path_info_;
  std::string query_string_;
  std::string http_method_;
  std::string message_body_;
  std::map<std::string, std::string> headers_;
};

#endif // COMMON_HTTPREQUEST_H__

