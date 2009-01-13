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


#include "common/sitesettings.h"

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include "third_party/tinyxml/tinyxml.h"
#include "common/basesetting.h"
#include "common/logger.h"
#include "common/util.h"
#include "common/accesscontroller.h"

SiteSettings::SiteSettings() : BaseSetting("SiteSettings"),
  global_setting_("GlobalSetting") {
  ResetToDefault();
}

SiteSettings::~SiteSettings() {
}

void SiteSettings::ResetToDefault() {
  // Initialize attributes to default values first.
  xml_node_ = NULL;
  backup_duration_ = 600;
  auto_add_ = true;

  remote_admin_ = false;
  admin_name_ = "admin";

  // User input password and its security salt.
  admin_password_ = "";
  password_salt_ = "";

  logging_level_ = EVENT_IMPORTANT;
  apache_conf_ = "";
  apache_group_ = "";

  global_setting_.ResetToDefault();
  site_settings_.clear();
}

bool SiteSettings::LoadFromFile(const char *xml_file_name) {
  Logger::Log(EVENT_CRITICAL, "Loading setting file from %s", xml_file_name);

  ResetToDefault();

  // Load setting from xml file.
  TiXmlDocument xml_doc;
  if (!xml_doc.LoadFile(xml_file_name)) {
    Logger::Log(EVENT_CRITICAL, "Loading setting file failed. Skip loading.");
    return true;
  } else {
    return LoadFromParent(&xml_doc);
  }
}

bool SiteSettings::ToSystemEncoding() {
  for (int i = 0; i < static_cast<int>(site_settings_.size()); ++i) {
    if (!site_settings_[i].ToSystemEncoding()) {
      Logger::Log(EVENT_ERROR, "Failed to convert site [%s] setting to system encoding.",
                site_settings_[i].site_id().c_str());
      return false;
    }
  }
  return true;
}

bool SiteSettings::LoadFromString(const char *xml_string) {
  Logger::Log(EVENT_CRITICAL, "Loading setting from xml std::string");

  ResetToDefault();

  // Parse setting from xml std::string
  TiXmlDocument xml_doc;
  xml_doc.Parse(xml_string);
  if (xml_doc.RootElement() == NULL) {
    Logger::Log(EVENT_CRITICAL, "Loading xml std::string failed.");
    return false;
  }

  return LoadFromParent(&xml_doc);
}

bool SiteSettings::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;

  // Load attributes from XML if it exists.
  LoadAttribute("backup_duration_in_seconds", backup_duration_);
  LoadAttribute("auto_add", auto_add_);

  // load admin related info.
  LoadAttribute("remote_admin", remote_admin_);
  LoadAttribute("admin_name", admin_name_);
  LoadAttribute("admin_password", admin_password_);
  LoadAttribute("password_salt", password_salt_);

  LoadAttribute("logging_level", logging_level_);
  LoadAttribute("apache_conf", apache_conf_);
  LoadAttribute("apache_group", apache_group_);

  if (!global_setting_.LoadFromParent(xml_node_)) {
    Logger::Log(EVENT_ERROR, "Loading global setting failed!");
    return false;
  }

  // Get all <SiteSetting> nodes.
  for (TiXmlElement* itr = xml_node_->FirstChildElement("SiteSetting");
    itr; itr = itr->NextSiblingElement("SiteSetting")) {
    SiteSetting site(global_setting_);
    site.set_setting_name("SiteSetting");
    site.LoadSetting(itr);
    site_settings_.push_back(site);
  }

  return true;
}

TiXmlElement* SiteSettings::SaveSetting() {
  Logger::Log(EVENT_CRITICAL, "Saving setting to xml document.");
  xml_node_ = new TiXmlElement(setting_name_.c_str());

  SaveAttribute("auto_add", auto_add_);
  SaveAttribute("backup_duration_in_seconds", backup_duration_);

  SaveAttribute("remote_admin", remote_admin_);
  SaveAttribute("admin_name", admin_name_);
  SaveAttribute("admin_password", admin_password_);
  SaveAttribute("password_salt", password_salt_);

  SaveAttribute("logging_level", logging_level_);

#if defined(__linux__) || defined(__unix__)
  SaveAttribute("apache_conf", apache_conf_);
  SaveAttribute("apache_group", apache_group_);
#endif

  SaveChild(&global_setting_);
  for (int i = 0; i < (int) site_settings_.size(); ++i) {
    SaveChild(&site_settings_[i], &global_setting_);
  }

  return xml_node_;
}

TiXmlElement* SiteSettings::SaveSetting(const BaseSetting* setting) {
  // Useless.
  assert(false);
  return NULL;
}

bool SiteSettings::SaveToFile(const char *xml_file_name) {
  TiXmlDocument xml_doc;
  xml_doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
  xml_doc.LinkEndChild(SaveSetting());

  // Save doc to a local file.
  if (!xml_doc.SaveFile(xml_file_name)) {
    Logger::Log(EVENT_ERROR, "Saving setting file %s failed.", xml_file_name);
    return false;
  }

  if (!AccessController::AllowWebserverAccess(xml_file_name,
                                              AccessController::kAllowRead)) {
    Logger::Log(EVENT_ERROR, "Failed to change setting file [%s] permission.",
                xml_file_name);
    return false;
  }

  return true;
}

bool SiteSettings::SaveToString(std::string *xml_string) {
  TiXmlDocument xml_doc;
  xml_doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
  xml_doc.LinkEndChild(SaveSetting());

  // Save doc to a xml std::string.
  TiXmlOutStream out;
  xml_doc.StreamOut(&out);
  xml_string->assign(out.c_str());
  return true;
}

bool SiteSettings::Validate() const {
  if (backup_duration_ <= 0)
    return false;

  if (logging_level_ < 0) {
    return false;
  }

  // this field should be ignored under windows
#if defined(__linux__) || defined(__unix__)
  if (apache_conf_.length() == 0 || apache_group_.length() == 0) {
    return false;
  }
#endif

  // Validate each site setting.
  for (std::vector<SiteSetting>::const_iterator it = site_settings_.begin();
    it != site_settings_.end(); ++it) {
    if (!it->Validate())
      return false;
  }

  return true;
}

void SiteSettings::MergeSetting(const WebserverConfig* webserver_config) {
  std::map<std::string, SiteSetting> sites_map;
  for (int i = 0; i < (int) site_settings_.size(); ++i) {
    // Skip empty site id.
    if (site_settings_[i].site_id().length() == 0) continue;

    sites_map[site_settings_[i].site_id()] = site_settings_[i];
  }

  site_settings_.clear();
  for (int i = 0; i < (int) webserver_config->site_ids().size(); ++i) {
    std::string site_id = webserver_config->site_ids()[i];
    if (sites_map.find(site_id) != sites_map.end()) {
      site_settings_.push_back(sites_map[site_id]);
    } else if (auto_add_ = true) {
      SiteSetting new_site(global_setting_);
      new_site.set_setting_name("SiteSetting");
      new_site.set_site_id(site_id);
      site_settings_.push_back(new_site);
    } else {
      continue;
    }

    SiteSetting& site = site_settings_.back();
    site.set_physical_path(webserver_config->physical_paths()[i]);
    site.set_name(webserver_config->names()[i]);
    if (webserver_config->host_urls()[i].length() != 0
      && site.host_url().url().length() == 0) {
      site.set_host_url(Url(webserver_config->host_urls()[i].c_str()));
    }
    if (webserver_config->log_paths()[i].length() != 0
      && site.log_path().length() == 0) {
      site.set_log_path(webserver_config->log_paths()[i]);
    }
  }
}

bool SiteSettings::Equals(const BaseSetting* another) const {
  // Useless
  assert(false);
  return false;
}
