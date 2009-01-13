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


#include "admin_console_cgi/cgihandler.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "common/httpconst.h"
#include "admin_console_cgi/cgiconst.h"

bool CgiHandler::ReadRequest(HttpRequest* request) {
  const char* env = NULL;
  request->Reset();

  // Read common environment vars.
  env = getenv(CgiConst::kEnvRemoteAddr);
  if (env != NULL) request->set_remote_addr(env);

  env = getenv(CgiConst::kEnvPathInfo);
  if (env != NULL) request->set_path_info(env);

  env = getenv(CgiConst::kEnvQueryString);
  if (env != NULL) request->set_query_string(env);

  env = getenv(CgiConst::kEnvRequestMethod);
  if (env != NULL) request->set_http_method(env);

  // Read HTTP headers.
  env = getenv(CgiConst::kEnvHttpCookie);
  if (env != NULL) request->SetHeader(HttpConst::kHeaderCookie, env);

  env = getenv(CgiConst::kEnvHttpAcceptLanguage);
  if (env != NULL) request->SetHeader(HttpConst::kHeaderAcceptLanguage, env);

  env = getenv(CgiConst::kEnvHttpIfModifiedSince);
  if (env != NULL) request->SetHeader(HttpConst::kHeaderIfModifiedSince, env);

  // Read HTTPS flag.
  env = getenv(CgiConst::kEnvHttps);
#ifdef WIN32
  // env could be "ON" or "OFF"
  if (env != NULL && stricmp(env, "ON") == 0) {
    request->set_https(HttpRequest::kHttpsOn);
  }
#else
  // env could be set or not set to distinguish https.
  if (env != NULL && strcasecmp(env, "OFF") != 0) {
    request->set_https(HttpRequest::kHttpsOn);
  }
#endif

  // Read content length.
  env = getenv(CgiConst::kEnvContentLength);
  if (env == NULL) return true;
  int length = atoi(env);

  // Read content value.
  std::string message;
  char buffer[1024];
  while (length > 0) {
    int t = length < 1024 ? length : 1024;
    if (fread(buffer, sizeof(char), t, stdin) != t) {
      fprintf(stderr, "GSG Admin Console CGI reads message body error (%d).",
              errno);
      return false;
    }
    message.append(buffer, buffer + t);
    length -= t;
  }
  request->set_message_body(message);

  return true;
}

bool CgiHandler::WriteResponse(const HttpResponse& response) {
  // Write status line.
  fprintf(stdout, "Status: %s\n", response.status().c_str());

  // Write http headers.
  const std::map<std::string, std::string>& headers = response.headers();
  std::map<std::string, std::string>::const_iterator itr = headers.begin();
  for (; itr != headers.end(); ++itr) {
    fprintf(stdout, "%s: %s\n", itr->first.c_str(), itr->second.c_str());
  }
  fprintf(stdout, "\n");

  // Write http message body.
  fwrite(response.message_body().c_str(), sizeof(char),
         response.message_body().length(), stdout);
  return true;
}
