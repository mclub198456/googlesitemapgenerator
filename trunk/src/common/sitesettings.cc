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


#include "common/sitesettings.h"

#include <vector>
#include <string>
#include <set>
#include <algorithm>

#include "third_party/tinyxml/tinyxml.h"
#include "common/basesetting.h"
#include "common/util.h"
#include "common/fileutil.h"

#ifdef __linux__
#include "common/apacheconfig.h"
const char* SiteSettings::kDefaultFilePath =
  "/etc/google-sitemap-generator/sitesettings.xml";
#endif

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
  setting_port_ = 8181;
  auto_add_ = true;

  remote_admin_ = false;
  admin_name_ = "admin";

  // Default password is "admin". Here is the encrypted string.
#ifdef WIN32
  admin_password_ = "21232f297a57a5a743894a0e4a801fc3";
#else
  admin_password_ = "4f0386056e1b2f0052a60100250d0000";
#endif

  logging_level_ = EVENT_IMPORTANT;
  apache_conf_ = "";

  global_setting_.ResetToDefault();
  site_settings_.clear();

  xml_document_.Clear();
}

std::string SiteSettings::GetDefaultFilePath() {
#ifdef WIN32
  return Util::GetApplicationDir().append("/").append("sitesettings.xml");
#elif defined(__linux__)
  return SiteSettings::kDefaultFilePath;
#endif
}

std::string SiteSettings::GetDefaultFilePath(const std::string& dir) {
  std::string file(dir);
  file.append("/sitesettings.xml");
  return file;
}

bool SiteSettings::UpdateSiteSettings() {
  // Backup first.
  if (!BackupDefaultSettingFile()) {
    Util::Log(EVENT_ERROR, "Failed to backup when updating default setting.");
    return false;
  }

  // Do the actual saving.
  return UpdateSiteSettings(GetDefaultFilePath().c_str());
}

bool SiteSettings::BackupDefaultSettingFile() {
#ifdef WIN32
  std::string backup_dir(Util::GetApplicationDir());
#else
  std::string backup_dir("/etc/google-sitemap-generator");
#endif
  backup_dir.append("/settings_backup");
  if (!FileUtil::CreateDir(backup_dir.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to create settings bakcup dir [%s].",
      backup_dir.c_str());
    return false;
  }

  // No old setting file.
  if (!FileUtil::Exists(GetDefaultFilePath().c_str())) {
    return true;
  }

  // Generate backup file name.
  time_t now = time(NULL);
  char timestamp[32];
  strftime(timestamp, 32, "%Y%m%d%H%M%S", localtime(&now));
  std::string backup_file(backup_dir);
  backup_file.append("/").append(timestamp).append(".xml");

  // Backup current setting file.
  if (!FileUtil::CopyFile(GetDefaultFilePath().c_str(), backup_file.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to backup current settings to [%s]",
      backup_file.c_str());
    return false;
  }

  // Remove the obsoleted back up files.
  std::vector<std::string> all_backup;
  if (FileUtil::ListDir(backup_dir.c_str(), false, NULL, &all_backup)) {
    if (all_backup.size() > kMaxBackupSettings) {
      std::sort(all_backup.begin(), all_backup.end());
      while (all_backup.size() > kMaxBackupSettings) {
        FileUtil::DeleteFile(all_backup[0].c_str());
        all_backup.erase(all_backup.begin());
      }
    }
  } else {
    Util::Log(EVENT_ERROR, "Failed to list all setting backup. Ignore.");
  }

  return true;
}

bool SiteSettings::UpdateSiteSettings(const char* path) {
  SiteSettings settings;

  if (!settings.LoadWebserverConfig()) {
    Util::Log(EVENT_ERROR, "Failed to load webserver config.");
    return false;
  }

  if (!settings.LoadFromFile(path) ) {
    Util::Log(EVENT_ERROR, "Failed to load file config from %s.", path);
    return false;
  }

  // apache_conf from xml setting and ApacheConfig::GetConfFilePath
  // are usually the same value, except sometimes the apache_conf is set via
  // command line option explicitly.
#ifdef __linux__
  settings.set_apache_conf(ApacheConfig::GetConfFilePath());
#endif

  if (!settings.SaveToFile(path)) {
    Util::Log(EVENT_ERROR, "Failed to save settings to %s.", path);
    return false;
  }

  return true;
}

bool SiteSettings::LoadFromFile(const char *xml_file_name) {
  Util::Log(EVENT_CRITICAL, "Loading setting file from %s", xml_file_name);

  ResetToDefault();

  // Load setting file if it's not empty.
  if (!xml_document_.LoadFile(xml_file_name)) {
    Util::Log(EVENT_CRITICAL, "Loading setting file failed. Skip loading.");
    xml_document_.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
  }

  return Load(&xml_document_);
}

bool SiteSettings::LoadWebserverConfig() {
  if (!webserver_config_.Load()) {
    return false;
  } else {
    if (webserver_config_.site_ids().size() == 0) {
      Util::Log(EVENT_IMPORTANT, "Warning: No site in webserver_config_.");
    }
    return true;
  }
}

bool SiteSettings::LoadFromString(const char *xml_string) {
  Util::Log(EVENT_CRITICAL, "Loading setting from xml std::string");

  ResetToDefault();

  // Parse setting from xml std::string
  xml_document_.Parse(xml_string);

  if (xml_document_.FirstChild() == NULL) {
    Util::Log(EVENT_CRITICAL, "Loading xml std::string failed.");
    return false;
  }

  return Load(&xml_document_);
}

bool SiteSettings::LoadSetting() {
  // Load attributes from XML if it exists.
  LoadAttribute("backup_duration_in_seconds", backup_duration_);
  LoadAttribute("auto_add", auto_add_);
  LoadAttribute("setting_port", setting_port_);

  // load admin related info.
  LoadAttribute("remote_admin", remote_admin_);
  LoadAttribute("admin_name", admin_name_);
  LoadAttribute("admin_password", admin_password_);
  if (admin_name_.length() == 0 || admin_password_.length() == 0) {
    Util::Log(EVENT_ERROR, "Admin name or password can't be empty.");
    return false;
  }

  LoadAttribute("logging_level", logging_level_);
  LoadAttribute("apache_conf", apache_conf_);

  if (!global_setting_.Load(xml_node_)) {
    Util::Log(EVENT_ERROR, "Loading global setting failed!");
    return false;
  }

  // Get all <SiteSetting> nodes.
  std::set<TiXmlElement*> obsoleted_sitenode;
  for (TiXmlElement* itr = xml_node_->FirstChildElement("SiteSetting");
    itr; itr = itr->NextSiblingElement("SiteSetting")) {
      obsoleted_sitenode.insert(itr);
  }

  for (size_t i = 0; i < webserver_config_.site_ids().size(); i ++) {
    SiteSetting site_setting("SiteSetting");
    bool found = false;

    // Find the SiteSetting which matches site_id.
    for (TiXmlElement* itr = xml_node_->FirstChildElement("SiteSetting");
      itr; itr = itr->NextSiblingElement("SiteSetting")) {
        // Copy global value from global_setting.
        site_setting = global_setting_;

        site_setting.set_xml_node(itr);
        if (!site_setting.LoadSetting()) {
          Util::Log(EVENT_ERROR, "Loading SiteSetting (site_id=%s) failed!",
            site_setting.site_id().c_str());
          return false;
        }

        // Check if site_id matches
        if (site_setting.site_id() == webserver_config_.site_ids()[i]) {
          found = true;
          obsoleted_sitenode.erase(itr);
          break;
        }
    }

    // If site_id is found or auto_add_ is true, we should add this site.
    if (auto_add_ || found) {
      if (!found) {
        TiXmlElement* node = new TiXmlElement("SiteSetting");
        if (node == NULL)
          return false;

        xml_node_->LinkEndChild(node);
        site_setting = global_setting_;
        site_setting.set_xml_node(node);
        site_setting.LoadSetting();
      }

      site_setting.set_name(webserver_config_.names()[i]);
      site_setting.set_physical_path(webserver_config_.physical_paths()[i]);
      site_setting.set_site_id(webserver_config_.site_ids()[i]);

      // If the log_path is not provided, the value from webserver is used.
      if (site_setting.log_path().length() == 0) {
        site_setting.set_log_path(webserver_config_.log_paths()[i]);
      }

      if (site_setting.host_url().host().size() == 0
        && webserver_config_.host_urls()[i].size() != 0)
        site_setting.set_host_url(
        Url(webserver_config_.host_urls()[i].c_str()));
      site_settings_.push_back(site_setting);
    }
  }

  // Remove all obsoleted <SiteSetting> node.
  while (!obsoleted_sitenode.empty()) {
    xml_node_->RemoveChild(*obsoleted_sitenode.begin());
    obsoleted_sitenode.erase(obsoleted_sitenode.begin());
  }

  return true;
}

void SiteSettings::SaveSetting() {
  Util::Log(EVENT_CRITICAL, "Saving setting to xml document.");

  SaveAttribute("auto_add", auto_add_);
  SaveAttribute("backup_duration_in_seconds", backup_duration_);
  SaveAttribute("setting_port", setting_port_);

  SaveAttribute("remote_admin", remote_admin_);
  SaveAttribute("admin_name", admin_name_);
  SaveAttribute("admin_password", admin_password_);

  SaveAttribute("logging_level", logging_level_);

#ifdef __linux__
  SaveAttribute("apache_conf", apache_conf_);
#endif

  // Save global setting.
  global_setting_.SaveSetting();
}

bool SiteSettings::SaveToFile(const char *xml_file_name) {
  SaveSetting();

  // Save doc to a local file.
  if (!xml_document_.SaveFile(xml_file_name)) {
    Util::Log(EVENT_ERROR, "Saving setting file %s failed.", xml_file_name);
    return false;
  }

  return true;
}

bool SiteSettings::SaveToString(std::string *xml_string) {
  TIXML_OSTREAM out;

  SaveSetting();
  // Save doc to a xml std::string.
  xml_document_.StreamOut(&out);

  xml_string->assign(out.c_str());
  return true;
}

bool SiteSettings::Validate() const {
  if (backup_duration_ <= 0)
    return false;

  if (setting_port_ <=0 || setting_port_ >= 65536)
    return false;

  if (admin_name_.length() == 0 || admin_password_.length() == 0) {
    return false;
  }

  if (logging_level_ < 0) {
    return false;
  }

  // this field should be ignored under windows
#ifdef __linux__
  if (apache_conf_.length() == 0) {
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


bool SiteSettings::ChangeAdminPassword(const char* password) {
  if (password == NULL || *password == '\0') {
    return false;
  }

  std::string encrypted;
  if (Util::MD5Encrypt(password, &encrypted)) {
    set_admin_password(encrypted);
    return true;
  } else {
    return false;
  }
}
