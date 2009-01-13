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


#include "common/sitesetting.h"

#include "third_party/tinyxml/tinyxml.h"
#include "common/logger.h"
#include "common/util.h"

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

  url_replacements_.ResetToDefault();
  url_replacements_.AddItem(UrlReplacement("/*[session=*]", "/[]"));

  included_queryfields_.ResetToDefault();

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

bool SiteSetting::LoadSettingForFilter() {
  LoadAttribute("enabled", enabled_);
  LoadAttribute("site_id", site_id_);

  // Load url provider settings.
  webserver_filter_setting_.LoadFromParent(xml_node_);

  return true;
}


bool SiteSetting::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;

  LoadAttribute("enabled", enabled_);
  LoadAttribute("site_id", site_id_);
  LoadAttribute("name", name_);
  LoadAttribute("host", host_url_);
  LoadAttribute("log_path", log_path_);

  LoadAttribute("max_url_in_memory", max_url_in_memory_);
  LoadAttribute("max_url_in_disk", max_url_in_disk_);
  LoadAttribute("max_url_life_in_days", max_url_life_);

  LoadAttribute("add_generator_info", add_generator_info_);

  // Load Url replacements setting.
  url_replacements_.LoadFromParent(xml_node_);
  included_queryfields_.LoadFromParent(xml_node_);

  // Load sitemap specific setting
  web_sitemap_setting_.LoadFromParent(xml_node_);
  news_sitemap_setting_.LoadFromParent(xml_node_);
  video_sitemap_setting_.LoadFromParent(xml_node_);
  mobile_sitemap_setting_.LoadFromParent(xml_node_);
  codesearch_sitemap_setting_.LoadFromParent(xml_node_);

  blogsearch_ping_setting_.LoadFromParent(xml_node_);

  // Load url provider settings.
  webserver_filter_setting_.LoadFromParent(xml_node_);
  file_scanner_setting_.LoadFromParent(xml_node_);
  log_parser_setting_.LoadFromParent(xml_node_);

  return true;
}

bool SiteSetting::ToSystemEncoding() {
  std::string temp;
  if (!Util::UTF8ToSystem(name_.c_str(), &temp)) {
    Logger::Log(EVENT_ERROR, "Failed to convert name of to system encoding.");
    return false;
  }
  name_ = temp;

  if (!Util::UTF8ToSystem(physical_path_.c_str(), &temp)) {
    Logger::Log(EVENT_ERROR, "Failed to convert path of to system encoding.");
    return false;
  }
  physical_path_ = temp;

  if (!Util::UTF8ToSystem(log_path_.c_str(), &temp)) {
    Logger::Log(EVENT_ERROR, "Failed to convert log path of to system encoding.");
    return false;
  }
  log_path_ = temp;

  return web_sitemap_setting_.ToSystemEncoding()
    && news_sitemap_setting_.ToSystemEncoding()
    && video_sitemap_setting_.ToSystemEncoding()
    && mobile_sitemap_setting_.ToSystemEncoding()
    && codesearch_sitemap_setting_.ToSystemEncoding();
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
  if (!url_replacements_.Validate()) return false;
  if (!included_queryfields_.Validate()) return false;

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

TiXmlElement* SiteSetting::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());

  // Non-inheritable settings.
  SaveAttribute("site_id", site_id_);
  SaveAttribute("name", name_);
  SaveAttribute("host", host_url_);
  SaveAttribute("log_path", log_path_);

  // Inheritable settings.
  SaveAttribute("enabled", enabled_);
  SaveAttribute("max_url_in_disk", max_url_in_disk_);
  SaveAttribute("max_url_in_memory", max_url_in_memory_);
  SaveAttribute("max_url_life_in_days", max_url_life_);
  SaveAttribute("add_generator_info", add_generator_info_);

  // Save UrlReplacement setting.
  SaveChild(&url_replacements_);
  SaveChild(&included_queryfields_);

  SaveChild(&web_sitemap_setting_);
  SaveChild(&news_sitemap_setting_);
  SaveChild(&video_sitemap_setting_);
  SaveChild(&mobile_sitemap_setting_);
  SaveChild(&codesearch_sitemap_setting_);

  SaveChild(&blogsearch_ping_setting_);

  SaveChild(&webserver_filter_setting_);
  SaveChild(&log_parser_setting_);
  SaveChild(&file_scanner_setting_);

  return xml_node_;
}

TiXmlElement* SiteSetting::SaveSetting(const BaseSetting* global) {
  const SiteSetting* another = (const SiteSetting*) global;
  xml_node_ = new TiXmlElement(setting_name_.c_str());

  // Non-inheritable settings.
  SaveAttribute("site_id", site_id_);
  SaveAttribute("name", name_);
  SaveAttribute("host", host_url_);
  SaveAttribute("log_path", log_path_);

  // Inheritable settings.
  SaveAttribute("enabled", enabled_, another->enabled_);
  SaveAttribute("max_url_in_disk", max_url_in_disk_, another->max_url_in_disk_);
  SaveAttribute("max_url_in_memory", max_url_in_memory_,
    another->max_url_in_memory_);
  SaveAttribute("max_url_life_in_days", max_url_life_,
    another->max_url_life_);
  SaveAttribute("add_generator_info", add_generator_info_,
    another->add_generator_info_);

  SaveChild(&url_replacements_, &(another->url_replacements_));
  SaveChild(&included_queryfields_, &(another->included_queryfields_));

  // TODO: remove empty element here.
  // TODO: current ugly code is used for compatibility with current UI.
  SaveChild(&web_sitemap_setting_, &(another->web_sitemap_setting_));
  if (xml_node_->FirstChildElement("WebSitemapSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("WebSitemapSetting"));
  }
  SaveChild(&news_sitemap_setting_, &(another->news_sitemap_setting_));
  if (xml_node_->FirstChildElement("NewsSitemapSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("NewsSitemapSetting"));
  }
  SaveChild(&video_sitemap_setting_, &(another->video_sitemap_setting_));
  if (xml_node_->FirstChildElement("VideoSitemapSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("VideoSitemapSetting"));
  }
  SaveChild(&mobile_sitemap_setting_, &(another->mobile_sitemap_setting_));
  if (xml_node_->FirstChildElement("MobileSitemapSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("MobileSitemapSetting"));
  }
  SaveChild(&codesearch_sitemap_setting_, &(another->codesearch_sitemap_setting_));
  if (xml_node_->FirstChildElement("CodeSearchSitemapSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("CodeSearchSitemapSetting"));
  }
  SaveChild(&blogsearch_ping_setting_, &(another->blogsearch_ping_setting_));
  if (xml_node_->FirstChildElement("BlogSearchPingSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("BlogSearchPingSetting"));
  }
  SaveChild(&webserver_filter_setting_, &(another->webserver_filter_setting_));
  if (xml_node_->FirstChildElement("WebserverFilterSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("WebserverFilterSetting"));
  }
  SaveChild(&log_parser_setting_, &(another->log_parser_setting_));
  if (xml_node_->FirstChildElement("LogParserSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("LogParserSetting"));
  }
  SaveChild(&file_scanner_setting_, &(another->file_scanner_setting_));
  if (xml_node_->FirstChildElement("FileScannerSetting") == NULL) {
    xml_node_->LinkEndChild(new TiXmlElement("FileScannerSetting"));
  }

  return xml_node_;
}

bool SiteSetting::Equals(const BaseSetting* a) const {
  const SiteSetting& another = *((const SiteSetting*) a);

  if (
    enabled_ != another.enabled_ ||
    site_id_ != another.site_id_ ||
    max_url_in_disk_ != another.max_url_in_disk_ ||
    max_url_in_memory_ != another.max_url_in_memory_ ||
    max_url_life_ != another.max_url_life_ ||
    host_url_.url() != another.host_url_.url() ||
    physical_path_ != another.physical_path_ ||
    log_path_ != another.log_path_ ||
    name_ != another.name_ ||
    add_generator_info_ != another.add_generator_info_) {
    return false;
  }

  if (!web_sitemap_setting_.Equals(&another.web_sitemap_setting_) ||
    !news_sitemap_setting_.Equals(&another.news_sitemap_setting_) ||
    !video_sitemap_setting_.Equals(&another.video_sitemap_setting_) ||
    !mobile_sitemap_setting_.Equals(&another.mobile_sitemap_setting_) ||
    !codesearch_sitemap_setting_.Equals(&another.codesearch_sitemap_setting_) ||
    !blogsearch_ping_setting_.Equals(&another.blogsearch_ping_setting_) ||
    !webserver_filter_setting_.Equals(&another.webserver_filter_setting_) ||
    !file_scanner_setting_.Equals(&another.file_scanner_setting_) ||
    !log_parser_setting_.Equals(&another.log_parser_setting_)) {
    return false;
  }

  // Check url replacements array
  if (!url_replacements_.Equals(&another.url_replacements_) ||
    !included_queryfields_.Equals(&another.included_queryfields_)) {
    return false;
  }

  return true;
}
