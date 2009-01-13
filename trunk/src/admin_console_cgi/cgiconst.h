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

// This file defines CGI environment variable names.

#ifndef ADMINCONSOLECGI_CGICONST_H__
#define ADMINCONSOLECGI_CGICONST_H__

class CgiConst {
 public:
  // Common CGI environment variables.
  static const char* kEnvRemoteAddr;
  static const char* kEnvPathInfo;
  static const char* kEnvQueryString;
  static const char* kEnvRequestMethod;
  static const char* kEnvContentLength;
  
  // HTTP header environment variables.
  static const char* kEnvHttpCookie;
  static const char* kEnvHttpAcceptLanguage;
  static const char* kEnvHttpIfModifiedSince;

  // Environment variables related to https.
  static const char* kEnvHttps;
  static const char* kEnvHttpsKeysize;

 private:
   CgiConst() {}
};

#endif // ADMINCONSOLECGI_CGICONST_H__

