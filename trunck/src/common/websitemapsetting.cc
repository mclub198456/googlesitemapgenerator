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


#include "common/websitemapsetting.h"


WebSitemapSetting::WebSitemapSetting()
  : SitemapSetting("WebSitemapSetting") {
  ResetToDefault();
}

WebSitemapSetting::~WebSitemapSetting() {
}

void WebSitemapSetting::ResetToDefault() {
  SitemapSetting::ResetToDefault();

  enabled_ = true;
  included_in_robots_txt_ = false;
  file_name_ = SitemapSetting::GenerateSitemapName("web_sitemap"); 

  // Set notify URLs.
  notify_urls_.push_back(
    Url("http://www.google.com/webmasters/sitemaps/ping?sitemap="));
  notify_urls_.push_back(
    Url("http://search.yahooapis.com/SiteExplorerService/V1/ping?sitemap="));
  notify_urls_.push_back(Url("http://submissions.ask.com/ping?sitemap="));
  notify_urls_.push_back(Url("http://api.moreover.com/ping?u="));
  notify_urls_.push_back(Url("http://webmaster.live.com/ping.aspx?siteMap="));
}

bool WebSitemapSetting::LoadSetting() {
  LoadAttribute("included_in_robots_txt", included_in_robots_txt_);

  return SitemapSetting::LoadSetting();
}

void WebSitemapSetting::SaveSetting() {
  SaveAttribute("included_in_robots_txt", included_in_robots_txt_);

  SitemapSetting::SaveSetting();
}
