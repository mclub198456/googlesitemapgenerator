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

// SettingManager is used to manage settings in a setting file.
// It provides various operations on the setting file.
// It can load setting from the file, save setting to the file,
// backup the setting file, and etc.

#ifndef COMMON_SETTINGMANAGER_H__
#define COMMON_SETTINGMANAGER_H__

#include <string>

#include "third_party/tinyxml/tinyxml.h"

#ifdef WIN32
#include "common/iisconfig.h"
typedef IisConfig LocalWebServerConfig;
#elif defined(__linux__) || defined(__unix__)
#include "common/apacheconfig.h"
typedef ApacheConfig LocalWebServerConfig;
#endif

#include "common/sitesettings.h"

class SettingManager {
public:
  SettingManager();
  ~SettingManager();

  // Initialize this setting manager.
  // Afterwards, all the operations are against to given "setting_file".
  void Initialize(const std::string& setting_file);

  // Load settings from setting file.
  // "with_webserver_config" indicates that whether webserver config should
  // be merged into the result setting or not.
  bool LoadSetting(SiteSettings* settings, bool with_webserver_config);

  // Save settings to setting file.
  // "backup" indicates whether the old setting file should be backed up or not.
  bool SaveSetting(SiteSettings& settings, bool backup);

  // Update sitesettings at default path.
  // Note, other than update setting file, this method also backs up setting
  // file in "back_up" directory in the same path as setting file.
  bool UpdateSettingFile();

  // Set the password and assocciated security salt in default setting file.
  // The passwd should be in encrypted form.
  bool SetPassword(const char* passwd, const char* salt);

  // Set application level attribute in setting file.
  bool SetApplicationAttribute(const std::string& name,
                               const std::string& value);

  // Set notify status in global setting.
  bool SetGlobalNotifyStatus(bool enabled);

  // Back up default setting file.
  // The setting file will be copied into "back_up" directory, which is in the
  // same directory as the default setting file. The name of the setting file
  // is renamed to a time stamp, like 20001231235959.xml.
  bool BackupSettingFile();

  // Only load applicatoin level setting.
  bool LoadApplicationSetting(SiteSettings* setting);

  // Load webserver configuration
  bool ReloadWebserverConfig();

  // Load setting for specific site.
  // All values, including inherited values, are contained in returned element.
  bool LoadSiteSetting(const std::string& site_id, TiXmlElement** element);

  // Save setting for a site.
  // The given element may only contain some settings values. In this case,
  // default value from GlobalSetting will be used for missing values.
  bool SaveSiteSetting(TiXmlElement* element);

  // Get/set webserver configuration.
  const LocalWebServerConfig& webserver_config() const {
    return webserver_config_;
  }
  void set_webserver_config(const LocalWebServerConfig& webserver_config) {
    webserver_config_ = webserver_config;
  }

  // Get setting file, which this setting manager operates on.
  const std::string setting_file() const {
    return setting_file_;
  }

  // Get default instance.
  // This instance is shared all over this application.
  static SettingManager* default_instance() {
    return &default_instance_;
  }

  // Get default setting file path.
  // Please use this method if it is used by main service.
  static std::string GetDefaultFilePath();

private:
  // Defines the most backup copies of default setting file.
  static const int kMaxBackupSettings = 10;

  // Default global instance.
  static SettingManager default_instance_;

  // Cached webserver configurations.
  LocalWebServerConfig webserver_config_;

  // Which this setting manager operates on.
  std::string setting_file_;
};

#endif // COMMON_SETTINGMANAGER_H__
