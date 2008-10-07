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

// This file defines constants used in http protocol,
// including common status code, http methods, and http header names.

#ifndef COMMON_HTTPCONST_H__
#define COMMON_HTTPCONST_H__

class HttpConst {
 public:
  // Status code constants.
  static const char* kStatus200;
  static const char* kStatus304;
  static const char* kStatus401;
  static const char* kStatus404;
  static const char* kStatus500;
  static const char* kStatus503;

  // Http method constants.
  static const char* kHttpMethodPost;
  static const char* kHttpMethodGet;

  // Header name constants.
  static const char* kHeaderDate;
  static const char* kHeaderContentLength;
  static const char* kHeaderContentType;
  static const char* kHeaderLastModified;
  static const char* kHeaderExpires;
  static const char* kHeaderSetCookie;
  static const char* kHeaderContentTransferEncoding;
  static const char* kHeaderCookie;
  static const char* kHeaderAcceptLanguage;
  static const char* kHeaderIfModifiedSince;
  static const char* kHeaderCacheControl;

 private:
  HttpConst() {}

};

#endif // COMMON_HTTPCONST_H__

