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


#include "common/sitesetting.h"

#include "third_party/tinyxml/tinyxml.h"

SiteSetting::SiteSetting()
  : BaseSetting("SiteSetting") {
  ResetToDefault();
}

SiteSetting::SiteSetting(const char *setting_name)
  : BaseSetting(setting_name) {
  ResetToDefault();
}

void SiteSetting::ResetToDefault() {
  xml_node_ = NULL;
  enabled_ = true;
  max_url_in_disk_ = 5 * 1000 * 1000;
  max_url_in_memory_ = 100 * 1000;
  max_url_life_ = 365;
  add_generator_info_ = true;

  url_replacements_.clear();
  UrlReplacement url_replacement("/*[session=*]", "/[]");
  url_replacements_.push_back(url_replacement);

  site_id_.clear();
  name_.clear();
  physical_path_.clear();
  host_url_.Clear();
  log_path_.clear();

  web_sitemap_setting_.ResetToDefault();
  news_sitemap_setting_.ResetToDefault();
  mobile_sitemap_setting_.ResetToDefault();
  video_sitemap_setting_.ResetToDefault();
  codesearch_sitemap_setting_.ResetToDefault();

  blogsearch_ping_setting_.ResetToDefault();

  webserver_filter_setting_.ResetToDefault();
  file_scanner_setting_.ResetToDefault();
  log_parser_setting_.ResetToDefault();
}

bool SiteSetting::LoadSetting() {
  LoadAttribute("enabled", enabled_);
  LoadAttribute("site_id", site_id_);
  LoadAttribute("host", host_url_);
  LoadAttribute("max_url_in_memory", max_url_in_memory_);
  LoadAttribute("max_url_in_disk", max_url_in_disk_);
  LoadAttribute("max_url_life_in_days", max_url_life_);
  LoadAttribute("log_path", log_path_);

  LoadAttribute("host", host_url_);

  LoadAttribute("add_generator_info", add_generator_info_);

  // Load Url replacements setting
  UrlReplacement::LoadUrls(xml_node_, url_replacements_);

  // Load sitemap specific setting
  web_sitemap_setting_.Load(xml_node_);
  news_sitemap_setting_.Load(xml_node_);
  video_sitemap_setting_.Load(xml_node_);
  mobile_sitemap_setting_.Load(xml_node_);
  codesearch_sitemap_setting_.Load(xml_node_);

  blogsearch_ping_setting_.Load(xml_node_);

  // Load url provider settings.
  webserver_filter_setting_.Load(xml_node_);
  file_scanner_setting_.Load(xml_node_);
  log_parser_setting_.Load(xml_node_);

  return true;
}

bool SiteSetting::Validate() const {
  // Skips validation if enabled_ is false.
  if (!enabled_)
    return true;

  // physical_path_ must not be empty
  if (physical_path_.length() == 0)
    return false;

  if (max_url_in_disk_ <= 0)
    return false;

  if (max_url_in_memory_ <= 0)
    return false;

  if (max_url_life_ <= 0)
    return false;

  // Validate UrlReplacement vector.
  for (std::vector<UrlReplacement>::const_iterator it = url_replacements_.begin();
    it != url_replacements_.end(); ++it) {
    if (!it->Validate())
      return false;
  }

  // Validate siteamp setting.
  if (!web_sitemap_setting_.Validate()) return false;
  if (!news_sitemap_setting_.Validate()) return false;
  if (!video_sitemap_setting_.Validate()) return false;
  if (!mobile_sitemap_setting_.Validate()) return false;
  if (!codesearch_sitemap_setting_.Validate()) return false;

  if (!blogsearch_ping_setting_.Validate()) return false;

  if (!webserver_filter_setting_.Validate()) return false;
  if (!file_scanner_setting_.Validate()) return false;
  if (!log_parser_setting_.Validate()) return false;

  // Log_path shouldn't be empty when log_parser_setting is enabled.
  if (log_parser_setting_.enabled() && log_path_.length() == 0) {
    return false;
  }

  return true;
}

void SiteSetting::SaveSetting() {
  SaveAttribute("site_id", site_id_);
  SaveAttribute("name", name_);
  SaveAttribute("host", host_url_);
  SaveAttribute("log_path", log_path_);
  SaveAttribute("enabled", enabled_);
  SaveAttribute("max_url_in_disk", max_url_in_disk_);
  SaveAttribute("max_url_in_memory", max_url_in_memory_);
  SaveAttribute("max_url_life_in_days", max_url_life_);
  SaveAttribute("add_generator_info", add_generator_info_);

  // Save UrlReplacement setting.
  UrlReplacement::SaveUrls(xml_node_, url_replacements_);

  web_sitemap_setting_.SaveSetting();
  news_sitemap_setting_.SaveSetting();
  video_sitemap_setting_.SaveSetting();
  mobile_sitemap_setting_.SaveSetting();
  codesearch_sitemap_setting_.SaveSetting();

  blogsearch_ping_setting_.SaveSetting();

  webserver_filter_setting_.SaveSetting();
  log_parser_setting_.SaveSetting();
  file_scanner_setting_.SaveSetting();
}
