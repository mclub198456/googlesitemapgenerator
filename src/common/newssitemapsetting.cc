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


#include "common/newssitemapsetting.h"


NewsSitemapSetting::NewsSitemapSetting()
  : SitemapSetting("NewsSitemapSetting") {
  ResetToDefault();
}

NewsSitemapSetting::~NewsSitemapSetting() {
}

void NewsSitemapSetting::ResetToDefault() {
  SitemapSetting::ResetToDefault();

  update_duration_ = 900;
  compress_ = false;
  max_file_url_number_ = 1000;
  max_file_size_ = 1024 * 1024;
  file_name_ = SitemapSetting::GenerateSitemapName("news_sitemap");
  expire_duration_= 86400 * 3;

  // News sitemap should be generated a week later.
  time(&update_start_time_);
  update_start_time_ += 7 * 24 * 3600;

  included_urls_.ResetToDefault();
  included_urls_.AddItem(UrlSetting(Url("/*.htm")));
  included_urls_.AddItem(UrlSetting(Url("/*.html")));
}

bool NewsSitemapSetting::LoadSetting(TiXmlElement* element) {
  if (!SitemapSetting::LoadSetting(element)) {
    return false;
  }

  LoadAttribute("expire_duration_in_seconds", expire_duration_);
  return true;
}

TiXmlElement* NewsSitemapSetting::SaveSetting() {
  SitemapSetting::SaveSetting();

  SaveAttribute("expire_duration_in_seconds", expire_duration_);
  return xml_node_;
}

TiXmlElement* NewsSitemapSetting::SaveSetting(const BaseSetting* global) {
  SitemapSetting::SaveSetting(global);

  const NewsSitemapSetting* another = (const NewsSitemapSetting*) global;
  SaveAttribute("expire_duration_in_seconds",
    expire_duration_, another->expire_duration_);
  return xml_node_;
}

bool NewsSitemapSetting::Validate() const {
  // max_file_url_number_ should between 1 ~ 1000
  if (max_file_url_number_ <= 0 || max_file_url_number_>1000)
    return false;

  // no particular restriction on news sitemap size?
  if (max_file_size_ <= 0 || max_file_size_ > 1024 * 1024 * 10) {
    return false;
  }

  if (expire_duration_ <= 0) {
    return false;
  }

  return SitemapSetting::Validate();
}

bool NewsSitemapSetting::Equals(const BaseSetting* a) const {
  const NewsSitemapSetting* another = (const NewsSitemapSetting*) a;
  if (!SitemapSetting::Equals(another)) {
    return false;
  }

  if (expire_duration_ != another->expire_duration_) {
    return false;
  }

  return true;
}

