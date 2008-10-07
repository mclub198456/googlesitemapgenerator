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
#include "common/logger.h"
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

  included_urls_.ResetToDefault();

  excluded_urls_.ResetToDefault();
  excluded_urls_.AddItem(UrlSetting(Url("/robots.txt")));
  excluded_urls_.AddItem(UrlSetting(Url("/*.jpg")));
  excluded_urls_.AddItem(UrlSetting(Url("/*.gif")));
  excluded_urls_.AddItem(UrlSetting(Url("/*.png")));
  excluded_urls_.AddItem(UrlSetting(Url("/*.css")));
  excluded_urls_.AddItem(UrlSetting(Url("/*.js")));
  excluded_urls_.AddItem(UrlSetting(Url("/*.swf")));
  excluded_urls_.AddItem(UrlSetting(Url("/*?*")));
  excluded_urls_.AddItem(UrlSetting(Url("/*password*")));

  // No search engine should be notified by default.
  notify_urls_.ResetToDefault();
}

bool SitemapSetting::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;

  LoadAttribute("enabled", enabled_);
  LoadAttribute("update_duration_in_seconds", update_duration_);
  if (update_duration_ < 60)
    update_duration_ = 60;

  LoadTimeAttribute("update_start_time", update_start_time_);
  LoadAttribute("file_name", file_name_);
  LoadAttribute("compress", compress_);
  LoadAttribute("max_file_url_number", max_file_url_number_);
  LoadAttribute("max_file_size", max_file_size_);

  included_urls_.LoadFromParent(xml_node_);
  excluded_urls_.LoadFromParent(xml_node_);
  notify_urls_.LoadFromParent(xml_node_);

  return true;
}

TiXmlElement* SitemapSetting::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());

  SaveAttribute("enabled", enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_);
  SaveTimeAttribute("update_start_time", update_start_time_);
  SaveAttribute("compress", compress_);
  SaveAttribute("file_name", file_name_);
  SaveAttribute("max_file_url_number", max_file_url_number_);
  SaveAttribute("max_file_size", max_file_size_);

  SaveChild(&included_urls_);
  SaveChild(&excluded_urls_);
  SaveChild(&notify_urls_);

  return xml_node_;
}

TiXmlElement* SitemapSetting::SaveSetting(const BaseSetting* global) {
  xml_node_ = NULL;
  const SitemapSetting* another = (const SitemapSetting*) global;
  SaveAttribute("enabled",enabled_, another->enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_,
    another->update_duration_);
  SaveTimeAttribute("update_start_time", update_start_time_,
    another->update_start_time_);
  SaveAttribute("compress", compress_, another->compress_);
  SaveAttribute("file_name", file_name_, another->file_name_);
  SaveAttribute("max_file_url_number", max_file_url_number_,
    another->max_file_url_number_);
  SaveAttribute("max_file_size", max_file_size_,
    another->max_file_size_);

  SaveChild(&included_urls_, &(another->included_urls_));
  SaveChild(&excluded_urls_, &(another->excluded_urls_));
  SaveChild(&notify_urls_, &(another->notify_urls_));
  
  return xml_node_;
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
    Logger::Log(EVENT_ERROR, "Sitemap name contains path separator (%s).",
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

  if (!included_urls_.Validate() || !excluded_urls_.Validate() ||
    !notify_urls_.Validate()) {
    return false;
  }

  // Validate notify_urls_.
  // We restrict notify URL on port 80.
  const std::vector<UrlSetting>& urls = notify_urls_.items();
  for (int i = 0; i < (int) urls.size(); ++i) {
    if (urls[i].value().port() != 80) {
      Logger::Log(EVENT_ERROR, "Only port 80 is allowed for notify URL.");
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
  // And the hex result is dividable by 13.
  srand(static_cast<unsigned int>(time(NULL)));
  int rem = 0;
  for (int i = 0; i < 7; ++i) {
    int index = rand() % static_cast<int>(randomstring.length());
    name.push_back(randomstring[index]);

    int k = Util::hex_digit_to_int(randomstring[index]);
    rem = ((rem + k) << 4) % 13;

    randomstring.erase(randomstring.begin() + index);
  }
  name.push_back(Util::int_to_hex_digit_low(13 - rem));
  name.append(".xml");
  return name;
}

bool SitemapSetting::ToSystemEncoding() {
  std::string temp;
  if (!Util::UTF8ToSystem(file_name_.c_str(), &temp)) {
    Logger::Log(EVENT_ERROR, "Failed to convert sitemap name [%s] to system.",
      file_name_.c_str());
    return false;
  }
  file_name_ = temp;
  return true;
}

bool SitemapSetting::Equals(const BaseSetting *a) const {
  const SitemapSetting* another = (const SitemapSetting*) a;

  if (update_duration_ != another->update_duration_ ||
    update_start_time_ != another->update_start_time_ ||
    compress_ != another->compress_ ||
    file_name_ != another->file_name_ ||
    max_file_url_number_ != another->max_file_url_number_ ||
    max_file_size_ != another->max_file_size_) {
    return false;
  }

  return included_urls_.Equals(&(another->included_urls_)) &&
    excluded_urls_.Equals(&(another->excluded_urls_)) &&
    notify_urls_.Equals(&(another->notify_urls_));
}





