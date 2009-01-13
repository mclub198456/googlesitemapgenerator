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

// HttpContext represents all context information of a http handling process,
// including HttpRequest object, HttpResponse object and other related
// information.

#ifndef SITEMAPSERVICE_HTTPCONTEXT_H__
#define SITEMAPSERVICE_HTTPCONTEXT_H__

#include <map>
#include <string>

#include "common/httprequest.h"
#include "common/httpresponse.h"

class HttpContext {
public:
  HttpContext();
  ~HttpContext();

  // Initialize this HttpContext.
  void Initialize(HttpRequest* request, HttpResponse* response);

  // Get response/request object contained.
  HttpRequest* request() const { return request_; }
  HttpResponse* response() const { return response_; }

  // Get request param.
  // Params were parsed from query string contained in URL and message body.
  std::string GetParam(const std::string& name);

  // Get language information,
  // which may come from URL or Accept-Language header.
  std::string GetLanguage();

  // Get action name, which comes from URL query string.
  // Different actions have different handlings.
  const std::string& action() const {
    return action_;
  }

  // Get/set whether the client has logged in or not.
  bool need_login() const { return need_login_; }
  void set_need_login(bool need_login) {
    need_login_ = need_login;
  }

  // Get/set session id. 
  const std::string& session_id() const {
    return session_id_;
  }
  void set_session_id(const std::string& session_id) {
    session_id_ = session_id;
  }

  // HTTP parameters names.
  static const std::string kXmlContentParamName;
  static const std::string kForceSaveParamName;
  static const std::string kXmlTimestampParamName;
  static const std::string kPasswordParamName;
  static const std::string kSIDParamName;
  static const std::string kOldPasswordParamName;
  static const std::string kNewPasswordParamName;

private:
  // Parse params from a param tring.
  // The string should be in the form of "key1=value1&key2=value2"
  void ParseParams(const std::string& param_string);

  // Retrieve session id from cookie.
  std::string RetrieveSID(const std::string& cookie);
 
  // Returns the string that in the 'str' and begin with 'begin', end with 'end'
  static std::string FindSubString(const std::string& str, 
    const std::string& begin, 
    const std::string& end);

  // replace '+' to ' ' in the param value
  static void UnescapeWhitespace(std::string* val);

  HttpRequest* request_;
  HttpResponse* response_;

  std::string action_;
  bool need_login_;
  std::string session_id_;

  std::map<std::string, std::string> params_;
};

#endif // SITEMAPSERVICE_HTTPCONTEXT_H__
