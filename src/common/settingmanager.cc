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

#include "common/settingmanager.h"

#include <algorithm>
#include "common/util.h"
#include "common/logger.h"
#include "common/interproclock.h"
#include "common/fileutil.h"
#include "common/cmdlineflags.h"

SettingManager SettingManager::default_instance_;

SettingManager::SettingManager() {
}

SettingManager::~SettingManager() {
}

void SettingManager::Initialize(const std::string& setting_file) {
  setting_file_ = setting_file;
}

bool SettingManager::SetPassword(const char* passwd, const char* salt) {
  TiXmlDocument xmldoc;
  if (!xmldoc.LoadFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to load file (%s) to set password.",
              setting_file_.c_str());
    return false;
  }

  TiXmlElement* root = xmldoc.RootElement();
  if (root == NULL) {
    Logger::Log(EVENT_ERROR, "Setting file is malformed.");
    return false;
  }

  root->SetAttribute("admin_password", passwd);
  root->SetAttribute("password_salt", salt);
  if (!xmldoc.SaveFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to save file (%s) to set password.",
              setting_file_.c_str());
    return false;
  }
  
  return true;
}


std::string SettingManager::GetDefaultFilePath() {
#ifdef WIN32
  return Util::GetApplicationDir().append("\\sitesettings.xml");
#else
  return Util::GetApplicationDir().append("/conf/sitesettings.xml");
#endif
}


bool SettingManager::UpdateSettingFile() {
  // Backup first.
  if (!BackupSettingFile()) {
    Logger::Log(EVENT_ERROR, "Failed to backup when updating setting.");
    return false;
  }

  // Load settings.
  SiteSettings settings;
  if (!ReloadWebserverConfig()) {
    Logger::Log(EVENT_ERROR, "Failed to load webserver config.");
    return false;
  }
  if (!settings.LoadFromFile(setting_file_.c_str()) ) {
    Logger::Log(EVENT_ERROR, "Failed to load file config from %s.",
              setting_file_.c_str());
    return false;
  }
  settings.MergeSetting(&webserver_config_);

  // Update values from command line flags.
  CmdLineFlags* flags = CmdLineFlags::GetInstance();
  if (flags->check_apache_conf()) {
    settings.set_apache_conf(flags->apache_conf());
  }
  if (flags->check_remote_admin()) {
    settings.set_remote_admin(flags->remote_admin());
  }

  // Save settings.
  if (!settings.SaveToFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to save settings to %s.",
              setting_file_.c_str());
    return false;
  }

  return true;
}

bool SettingManager::BackupSettingFile() {
#ifdef WIN32
  std::string backup_dir(Util::GetApplicationDir());
#else
  std::string backup_dir("/etc/google-sitemap-generator");
#endif

  backup_dir.append("/settings_backup");
  if (!FileUtil::CreateDir(backup_dir.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to create settings bakcup dir [%s].",
              backup_dir.c_str());
    return false;
  }

  // No old setting file.
  if (!FileUtil::Exists(setting_file_.c_str())) {
    return true;
  }

  // Generate backup file name.
  time_t now = time(NULL);
  char timestamp[32];
  strftime(timestamp, 32, "%Y%m%d%H%M%S", localtime(&now));
  std::string backup_file(backup_dir);
  backup_file.append("/").append(timestamp).append(".xml");

  // Backup current setting file.
  if (!FileUtil::CopyFile(setting_file_.c_str(), backup_file.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to backup current settings to [%s]",
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
    Logger::Log(EVENT_ERROR, "Failed to list all setting backup. Ignore.");
  }

  return true;
}

bool SettingManager::ReloadWebserverConfig() {
  if (!webserver_config_.Load()) {
    return false;
  } else {
    if (webserver_config_.site_ids().size() == 0) {
      Logger::Log(EVENT_IMPORTANT, "Warning: No site in webserver_config_.");
    }

#ifdef WIN32
    // Replace environment variables in file path.
    std::vector<std::string> values = webserver_config_.physical_paths();
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
      std::string result;
      if (!Util::ReplaceEnvironmentVariable(values[i], &result)) {
        Logger::Log(EVENT_ERROR, "Failed to replace EnvVar in site path.");
        return false;
      }
      values[i] = result;
    }
    webserver_config_.set_physical_paths(values);

    values = webserver_config_.log_paths();
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
      std::string result;
      if (!Util::ReplaceEnvironmentVariable(values[i], &result)) {
        Logger::Log(EVENT_ERROR, "Failed to replace EnvVar in log path.");
        return false;
      }
      values[i] = result;
    }
    webserver_config_.set_log_paths(values);
#endif  // WIN32


    // Remove admin console site.
    webserver_config_.RemoveAdminConsoleSite();

    // Convert system encoding to UTF8.
    std::vector<std::string> strs = webserver_config_.physical_paths();
    if (!Util::ConvertToUTF8(strs)) {
      Logger::Log(EVENT_ERROR, "Failed to convert physical paths to UTF8.");
      return false;
    }
    webserver_config_.set_physical_paths(strs);

    strs = webserver_config_.names();
    if (!Util::ConvertToUTF8(strs)) {
      Logger::Log(EVENT_ERROR, "Failed to convert site names to UTF8.");
      return false;
    }
    webserver_config_.set_names(strs);

    strs = webserver_config_.log_paths();
    if (!Util::ConvertToUTF8(strs)) {
      Logger::Log(EVENT_ERROR, "Failed to convert site paths to UTF8.");
      return false;
    }
    webserver_config_.set_log_paths(strs);

    return true;
  }
}

bool SettingManager::LoadApplicationSetting(SiteSettings* setting) {
  TiXmlDocument xmldoc;
  if (!xmldoc.LoadFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to load setting file (%s).",
              setting_file_.c_str());
    return false;
  }

  if (xmldoc.RootElement() != NULL) {
    xmldoc.RootElement()->Clear();
    setting->LoadSetting(xmldoc.RootElement());
  } else {
    setting->ResetToDefault();
  }

  return true;
}

bool SettingManager::LoadSetting(SiteSettings *settings, bool with_webserver_config) {
  settings->ResetToDefault();
  if (!settings->LoadFromFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to load setting from file (%s).",
              setting_file_.c_str());
    return false;
  }

  if (with_webserver_config) {
    settings->MergeSetting(&webserver_config_);
  }

  return true;
}

bool SettingManager::SaveSetting(SiteSettings &settings, bool backup) {

  if (backup) {
    if (!BackupSettingFile()) {
      Logger::Log(EVENT_ERROR, "Failed to backup setting file before saving.");
      return false;
    }
  }

  if (!settings.SaveToFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to save setting to file (%s).",
              setting_file_.c_str());
    return false;
  }

  return true;
}

bool SettingManager::LoadSiteSetting(const std::string &site_id,
                                     TiXmlElement **element) {
  TiXmlDocument xmldoc;
  if (!xmldoc.LoadFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to load file (%s) to load site setting.",
              setting_file_.c_str());
    return false;
  }

  SiteSetting site_setting;
  if (xmldoc.RootElement() == NULL) {
    *element = site_setting.SaveSetting();
    return true;
  }

  TiXmlElement* global_element =
    xmldoc.RootElement()->FirstChildElement("GlobalSetting");
  if (global_element != NULL) {
    site_setting.LoadSetting(global_element);
  }

  TiXmlElement* site_element =
    xmldoc.RootElement()->FirstChildElement("SiteSetting");
  while (site_element != NULL) {
    const char* id_attr = site_element->Attribute("site_id");
    if (id_attr != NULL && site_id == std::string(id_attr)) {
      site_setting.LoadSetting(site_element);
      break;
    }
    site_element = site_element->NextSiblingElement("SiteSetting");
  }

  *element = site_setting.SaveSetting();
  return true;
}


bool SettingManager::SaveSiteSetting(TiXmlElement *element) {
  InterProcLock lock("sitesettings.xml");
  if (!lock.Lock(1000)) {
    Logger::Log(EVENT_ERROR, "Failed to get lock when saving site setting.");
    return false;
  }
  
  TiXmlDocument xmldoc;
  if (!xmldoc.LoadFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to load file (%s) to save site setting.",
              setting_file_.c_str());
    return false;
  }

  if (xmldoc.RootElement() == NULL) {
    Logger::Log(EVENT_ERROR, "Malformed (%s) (no root) to save site setting.",
              setting_file_.c_str());
    return false;
  }

  // Load global setting.
  SiteSetting global_setting("GlobalSetting");
  TiXmlElement* global_element =
    xmldoc.RootElement()->FirstChildElement("GlobalSetting");
  if (global_element != NULL) {
    global_setting.LoadSetting(global_element);
  } else {
    Logger::Log(EVENT_ERROR, "Malformed (%s) (no global) to save site setting.",
              setting_file_.c_str());
    return false;
  }

  // Find old site setting element.
  std::string site_id = element->Attribute("site_id");
  TiXmlElement* site_element =
    xmldoc.RootElement()->FirstChildElement("SiteSetting");
  while (site_element != NULL) {
    const char* id_attr = site_element->Attribute("site_id");
    if (id_attr != NULL && site_id == std::string(id_attr)) {
      break;
    }
    site_element = site_element->NextSiblingElement("SiteSetting");
  }
  if (site_element == NULL) {
    Logger::Log(EVENT_ERROR, "No old site setting with id [%s].",
              site_id.c_str());
    return false;
  }

  // Load setting from given element.
  // Load global setting.
  SiteSetting site_setting(global_setting);
  site_setting.set_setting_name("SiteSetting");
  // Load old site setting.
  site_setting.LoadSetting(site_element);
  // Load new site setting.
  site_setting.LoadSetting(element);

  // Save site_setting to an xml file.
  TiXmlElement* new_element = site_setting.SaveSetting(&global_setting);

  // Replace old setting with new one.
  xmldoc.RootElement()->ReplaceChild(site_element, *new_element);

  if (!xmldoc.SaveFile(setting_file_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to save file (%s) to save site setting.",
              setting_file_.c_str());
    return false;
  }

  lock.Unlock();
  return true;
}


