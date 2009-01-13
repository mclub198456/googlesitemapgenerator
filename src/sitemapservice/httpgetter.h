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


#ifndef SITEMAPSERVICE_HTTPGETTER_H__
#define SITEMAPSERVICE_HTTPGETTER_H__

// This class is an client to send HTTP request and receive response from
// client. Currently, the supported HTTP request is simple.
// See methods for details.

#include <string>

class HttpGetter {
 public:
  // Max content length to receive.
  static const int kMaxContentLength = 1024 * 10;

  // Time-out value in seconds.
  static const int kTimeOut = 30;

  HttpGetter();
  ~HttpGetter() {}

  // Do http get.
  // It sends a GET request to http://host:port/path.
  // The result is saved in status() and content().
  bool Get(const char* host, int port, const char* path);

  // The following two getter are meaningful only if "Get" returns true.
  int status() { return status_; }

  const std::string& content() { return content_; }

 private:
  int status_;
  std::string content_;
};

#endif // SITEMAPSERVICE_HTTPGETTER_H__
