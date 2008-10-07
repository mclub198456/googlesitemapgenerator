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

#include "sitemapservice/httpcontext.h"

#include "common/logger.h"
#include "common/util.h"
#include "common/httpconst.h"

#include "sitemapservice/httplanguageheaderparser.h"

const std::string HttpContext::kXmlContentParamName = "xmlcontent";
const std::string HttpContext::kForceSaveParamName = "force";
const std::string HttpContext::kXmlTimestampParamName = "ts";
const std::string HttpContext::kPasswordParamName = "password";
const std::string HttpContext::kOldPasswordParamName = "opswd";
const std::string HttpContext::kNewPasswordParamName = "npswd";
const std::string HttpContext::kSIDParamName = "sid";

HttpContext::HttpContext() {
  // does nothing.
}

HttpContext::~HttpContext() {
  // does nothing.
}

void HttpContext::Initialize(HttpRequest* request, HttpResponse* response) {
  request_ = request;
  response_ = response;

  need_login_ = true;
  session_id_ = RetrieveSID(request->GetHeader(HttpConst::kHeaderCookie));

  params_.clear();
  ParseParams(request->query_string());
  if (request->http_method() == HttpConst::kHttpMethodPost) {
    ParseParams(request->message_body());
  }

  action_ = GetParam("action");
  if (action_.empty() || action_[0] != '/') {
    action_.insert(action_.begin(), '/');
  }
}

std::string HttpContext::GetParam(const std::string& name) {
  std::map<std::string, std::string>::iterator itr = params_.find(name);
  if (itr == params_.end()) {
    return "";
  } else {
    return itr->second;
  }
}

std::string HttpContext::GetLanguage() {
  // Get user prefer language.
  std::string language = GetParam("hl");
  if (language.empty() || 
    !HttpLanguageHeaderParser::IsSupportedLanguage(language)) {
      language = 
        HttpLanguageHeaderParser::GetPreferLanguage(HttpConst::kHeaderAcceptLanguage);
  } else {
    language = 
      HttpLanguageHeaderParser::GetLanguageRegularName(language);
  }
  return language; 
}

void HttpContext::ParseParams(const std::string& param_string) {
  Util::StringVector name_value_pairs;
  Util::StrSplit(param_string, '&', &name_value_pairs);
  for (size_t i = 0; i < name_value_pairs.size(); i++) {
    Util::StringVector name_value;
    if (Util::StrSplit(name_value_pairs[i], '=', &name_value) != 2) {
      return;// wrong param
    }
    std::string nam = name_value[0];
    std::string val = name_value[1];

    // unescapeURL
    UnescapeWhitespace(&val);
    std::string unencoded;
    Url::UnescapeUrl(val.c_str(), &unencoded);

    // insert params
    params_[nam] = unencoded;
  }
}

std::string HttpContext::RetrieveSID(const std::string& cookie) {
  // Find cookie contains SID.
  const std::string kCookieName = "gsg=";
  const std::string kCookieSplit = ";";
  std::string sid_cookie = FindSubString(cookie, kCookieName, kCookieSplit);
  if (sid_cookie.empty()) return "";

  // Parse sid value from cookie.
  const std::string ksionIdCookieName = "sid:";
  const std::string kCookieValueSplit = "&";
  std::string sessionId = 
    FindSubString(sid_cookie, ksionIdCookieName, kCookieValueSplit);
  return sessionId;
}

std::string HttpContext::FindSubString(const std::string& str, 
                                     const std::string& begin, 
                                     const std::string& end) {
  std::string::size_type begin_pos = str.find(begin);
  if (begin_pos != std::string::npos) {
    begin_pos += begin.size();
    std::string::size_type end_pos = str.find(end, begin_pos);
    if (end_pos == std::string::npos) {
      return str.substr(begin_pos);
    } else {
      return str.substr(begin_pos, end_pos - begin_pos);
    }
  }
  return "";
}

void HttpContext::UnescapeWhitespace(std::string* val) {
  // replace '+' to ' '
  std::string::size_type pos;
  while ( (pos = val->find_first_of('+')) != std::string::npos ) {
    val->replace(pos, 1, " ");
  }
}



