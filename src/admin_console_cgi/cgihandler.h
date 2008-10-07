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

// This file defines two utility methods to handle CGI request and response.

#ifndef ADMINCONSOLECGI_CGIHANDLER_H__
#define ADMINCONSOLECGI_CGIHANDLER_H__

#include "common/httprequest.h"
#include "common/httpresponse.h"

class CgiHandler {
 public:
  // Construct an HttpRequest object from CGI environment.
  static bool ReadRequest(HttpRequest* request);

  // Write an HttpResponse object to CGI environment.
  static bool WriteResponse(const HttpResponse& response);

 private:
  CgiHandler();
};

#endif

