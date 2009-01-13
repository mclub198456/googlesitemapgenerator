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


#include "sitemapservice/informer.h"

#include "common/url.h"
#include "common/logger.h"
#include "sitemapservice/httpgetter.h"


HttpRequestInformer::HttpRequestInformer(const Url &url) {
  url_ = url;
}

int HttpRequestInformer::Inform(const std::string &url) {
  // Construct the request path.
  std::string path(url_.path_url());
  Url::EscapeUrlComponent(url.c_str(), &path);

  // Create the port.
  int port = url_.port();
  if (port <= 0) {
    port = 80;
  }

  // Send the request.
  HttpGetter http_getter;
  if (!http_getter.Get(url_.host().c_str(), port, path.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to submit [%s] to [%s].",
      url.c_str(), url_.url().c_str());
    return -1;
  }

  // Validate the status code.
  if (http_getter.status() != 200) {
    Logger::Log(EVENT_ERROR, "Failed to submit [%s] to [%s] with status [%d].",
      url.c_str(), url_.url().c_str(), http_getter.status());
    return -2;
  }

  return 0;
}
