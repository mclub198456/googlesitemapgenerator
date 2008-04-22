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


// Informer can inform search engine the location of a sitemap.
// This is a pure abstract class to let plug in different kinds of informing
// strategy.
// See http://www.sitemaps.org for various kinds of informing ways.

#ifndef SITEMAPSERVICE_INFORMER_H__
#define SITEMAPSERVICE_INFORMER_H__

#include <string>

#include "common/url.h"

class Informer {
 public:
  // Inform search engine that a sitemap is in given url location.
  // Input
  //    sitemap_url the url location of a sitemap or sitemap index file.
  virtual int Inform(const std::string& sitemap_url) = 0;
};


// This implementation of informer provides a way to inform search engine via
// http request. The request url should just be like:
// http://www.search-engine.org/ping?sitemap=http://mydomain.org/sitemap.xml
// The search engine url and its port are configurable via constructor.
class HttpRequestInformer : public Informer {
 public:
  // Construct an http request informer.
  // Input
  //    search_engine search engine url location, like http://www.google.com/ping?sitemap=
  HttpRequestInformer(const Url &url);

  // Informs the search engine with sitemap url
  virtual int Inform(const std::string& sitemap_url);

 private:
  Url     url_;
};

#endif // SITEMAPSERVICE_INFORMER_H__

