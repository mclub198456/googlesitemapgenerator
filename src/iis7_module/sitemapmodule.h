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


// SitemapModule is used to filter all the HTTP request/reponse.
// It extracts information from IIS7 native API, and sends the information
// to UrlPipe.

#ifndef IIS7_MODULE_SITEMAPMODULE_H__
#define IIS7_MODULE_SITEMAPMODULE_H__

#include "common/basefilter.h"

#include <httpserv.h>

class SitemapModule : public CHttpModule {
 public:
  SitemapModule(BaseFilter* basefilter);
  ~SitemapModule() {}

  virtual REQUEST_NOTIFICATION_STATUS SitemapModule::OnLogRequest(
    IN IHttpContext* httpcontext, IN IHttpEventProvider* provider);

 private:
  // Content-Length is added by http.sys, but not IIS.
  // We could only have this solution currently.
  int64 GetContentLength(HTTP_RESPONSE* response);

  // Base filter providing the common functionality.
  BaseFilter* basefilter_;
};



#endif  // IIS7_MODULE_SITEMAPMODULE_H__
