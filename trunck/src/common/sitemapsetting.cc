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


#include "common/sitemapsetting.h"

#include <stdlib.h>
#include <vector>
#include "third_party/tinyxml/tinyxml.h"
#include "common/basesetting.h"
#include "common/util.h"
#include "common/port.h"

SitemapSetting::SitemapSetting(const char * setting_name)
  : BaseSetting(setting_name) {
}

SitemapSetting::~SitemapSetting() {
}

void SitemapSetting::ResetToDefault() {
  xml_node_ = NULL;
  enabled_ = false;
  update_duration_ = 24 * 3600;
  time(&update_start_time_);
  compress_ = true;
  max_file_url_number_ = 20000;
  max_file_size_ = 1024 * 1024 * 5;

  included_urls_.clear();

  excluded_urls_.clear();
  excluded_urls_.push_back(Url("/robots.txt"));
  excluded_urls_.push_back(Url("/*.jpg"));
  excluded_urls_.push_back(Url("/*.gif"));
  excluded_urls_.push_back(Url("/*.png"));
  excluded_urls_.push_back(Url("/*.css"));
  excluded_urls_.push_back(Url("/*.js"));
  excluded_urls_.push_back(Url("/*.swf"));
  excluded_urls_.push_back(Url("/*?*"));
  excluded_urls_.push_back(Url("/*password*"));

  // No search engine should be notified by default.
  notify_urls_.clear();
}

bool SitemapSetting::LoadSetting() {
  LoadAttribute("enabled", enabled_);
  LoadAttribute("update_duration_in_seconds", update_duration_);
  if (update_duration_ < 60)
    update_duration_ = 60;

  LoadAttribute("update_start_time", update_start_time_);
  LoadAttribute("file_name", file_name_);
  LoadAttribute("compress", compress_);
  LoadAttribute("max_file_url_number", max_file_url_number_);
  LoadAttribute("max_file_size", max_file_size_);

  LoadUrls("IncludedUrls", included_urls_);
  LoadUrls("ExcludedUrls", excluded_urls_);

  LoadUrls("NotifyUrls", notify_urls_);

  return true;
}

void SitemapSetting::SaveSetting() {
  SaveAttribute("enabled", enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_);
  SaveAttribute("update_start_time", update_start_time_);
  SaveAttribute("compress", compress_);
  SaveAttribute("file_name", file_name_);
  SaveAttribute("max_file_url_number", max_file_url_number_);
  SaveAttribute("max_file_size", max_file_size_);

  SaveUrls("IncludedUrls", included_urls_);
  SaveUrls("ExcludedUrls", excluded_urls_);
  SaveUrls("NotifyUrls", notify_urls_);
}

bool SitemapSetting::Validate() const {
  // Skips validation if enabled_ is false.
  if (!enabled_)
    return true;

  if (update_duration_ <= 0)
    return false;

  // file_name_ shouldn't be empty, and contain path separator.
  if (file_name_.length() == 0) return false;
  if (file_name_.find_first_of("\\/") != std::string::npos) {
    Util::Log(EVENT_ERROR, "Sitemap name contains path separator (%s).",
              file_name_.c_str());
    return false;
  }

  // max_file_url_number_ should between 1 ~ 50000
  if (max_file_url_number_ <= 0 || max_file_url_number_>50000)
    return false;

  // max_file_size should be in (0, 1M]
  if (max_file_size_ <= 0 || max_file_size_ > 1024 * 1024 * 10) {
    return false;
  }

  // Validate include_urls
  for (std::vector<Url>::const_iterator it = included_urls_.begin();
       it != included_urls_.end(); ++it) {
    if (!it->Validate())
      return false;
  }

  // Validate excluded_urls_
  for (std::vector<Url>::const_iterator it = excluded_urls_.begin();
       it != excluded_urls_.end(); ++it) {
    if (!it->Validate())
      return false;
  }
  
  // Validate notify_urls_.
  // We restrict notify URL on port 80.
  for (std::vector<Url>::const_iterator it = notify_urls_.begin();
       it != notify_urls_.end(); ++it) {
    if (!it->Validate())
      return false;

    if (it->port() != 80) {
      Util::Log(EVENT_ERROR, "Only port 80 is allowed for notify URL.");
      return false;
    }
  }

  return true;
}

std::string SitemapSetting::GenerateSitemapName(const std::string& prefix) {
  // Use current time value as a seed string.
  srand(static_cast<unsigned int>(time(NULL)));
  char buffer[16];
  itoa(rand(), buffer);

  // Encrypt the seed string.
  std::string randomstring;
  if (!Util::MD5Encrypt(buffer, &randomstring)) {
    // This rarely happens.
    randomstring = "12345678abcdefgh";
  }

  std::string name(prefix);
  name.append("_");

  // Randomly pick 8 characters from the encrypted string.
  srand(static_cast<unsigned int>(time(NULL)));
  for (int i = 0; i < 8; ++i) {
    int index = rand() % static_cast<int>(randomstring.length());
    name.push_back(randomstring[index]);
    randomstring.erase(randomstring.begin() + index);
  }

  name.append(".xml");
  return name;
}

