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


// This is the top level setting class. It contains all the setting values for
// this application. Besides site specific settings, this class also includes
// application level configuration, like back-up duration, remote admin port,
// admin account, and etc. Especially, there is global setting field, which
// contains default values for site settings. Please see the member fields
// doc for details.
// Besides the xml setting load/save/validate functions, it provides functions
// to load values from file, as well as save value to a file.
// This class is not thread-safe.

#ifndef COMMON_SITESETTINGS_H__
#define COMMON_SITESETTINGS_H__

#include <string>
#include <vector>

#include "common/sitesetting.h"
#include "common/basesetting.h"
#include "common/websitemapsetting.h"
#include "common/newssitemapsetting.h"

#ifdef WIN32
#include "common/iisconfig.h"
typedef IisConfig LocalWebServerConfig;

#elif defined(__linux__)
#include "common/apacheconfig.h"
typedef ApacheConfig LocalWebServerConfig;

#endif

class TiXmlDocument;

class SiteSettings : public BaseSetting {
public:
  SiteSettings();
  virtual ~SiteSettings();

  // Reset the value to default values.
  virtual void ResetToDefault();

  // Validates if current settings are valid settings.
  virtual bool Validate() const;

  // Load settings from xml element and web server.
  // If xml document is empty, default value will be used.
  // Returns false if loading failed.
  // If LoadWebserverConfig hasn't been called before this method, only global
  // setting could be loaded, and no site setting could be loaded.
  virtual bool LoadSetting();

  // Save settings to xml_node_.
  virtual void SaveSetting();

  // Get default setting file path.
  // Please use this method if it is used by main service.
  static std::string GetDefaultFilePath();

  // Get default setting file, which is in given "dir".
  static std::string GetDefaultFilePath(const std::string& dir);

  // Update given setting file.
  // If "auto_add" is true, and some new sites have been added into webserver,
  // a new site setting would also occur in the updated setting file.
  static bool UpdateSiteSettings(const char* path);

  // Update sitesettings at default path.
  // Note, other than update setting file, this method also backs up setting
  // file in "back_up" directory in the same path as setting file.
  static bool UpdateSiteSettings();

  // Back up default setting file.
  // The setting file will be copied into "back_up" directory, which is in the
  // same directory as the default setting file. The name of the setting file
  // is renamed to a time stamp, like 20001231235959.xml.
  static bool BackupDefaultSettingFile();

  // Load settings from xml_file_name file from disk.
  // Only sites contained in webserver_config_ can be loaded.
  // Note, LoadWebserverConfig must be called before this one. Otherwise, only
  // global setting could be loaded from file. Sites specific settings will
  // be ignored.
  bool LoadFromFile(const char *xml_file_name);

  // Load settings from xml string.
  // Only sites contained in webserver_config_ can be loaded.
  // Note, LoadWebserverConfig must be called before this one. Otherwise, only
  // global setting could be loaded from file. Sites specific settings will
  // be ignored.
  bool LoadFromString(const char *xml_string);

  // Save current settings to a file.
  // Returns false if saving failed.
  bool SaveToFile(const char *xml_file_name);

  // Save current settings to a string.
  // Returns false if saving failed.
  bool SaveToString(std::string *xml_string);

  // Getter/setter for setting values.
  const std::vector<SiteSetting>& site_settings() const {
    return site_settings_;
  }
  void set_site_settings(const std::vector<SiteSetting>& settings) {
    site_settings_ = settings;
  }

  // get/set back duration.
  const int backup_duration() const { return backup_duration_; }
  void set_backup_duration(int backup_duration) {
    backup_duration_ = backup_duration;
    SaveAttribute("backup_duration", backup_duration_);
  }

  // get/set auto_add.
  const bool auto_add() const { return auto_add_; }
  void set_auto_add(bool auto_add) {
    auto_add_ = auto_add;
    SaveAttribute("auto_add", auto_add_);
  }

  const int setting_port() const { return setting_port_; }
  void set_setting_port(int setting_port) {
    setting_port_ = setting_port;
    SaveAttribute("setting_port", setting_port_);
  }

  // get/set global setting.
  const SiteSetting &global_setting() const { return global_setting_; }
  void set_global_setting(const SiteSetting &global_setting) {
    global_setting_ = global_setting;
  }

  const bool remote_admin() { return remote_admin_; }
  void set_remote_admin(bool remote_admin) {
    remote_admin_ = remote_admin;
  }

  const std::string& admin_name() { return admin_name_; }
  void set_admin_name(const std::string& admin_name) {
    admin_name_ = admin_name;
    SaveAttribute("admin_name", admin_name_);
  }

  // set_admin_password accepts already encrypted pwd
  // ChangeAdminPassword can encrypt pwd for you, but may fail.
  const std::string& admin_password() { return admin_password_; }
  void set_admin_password(const std::string& admin_password) {
    admin_password_ = admin_password;
    SaveAttribute("admin_password", admin_password_);
  }
  bool ChangeAdminPassword(const char* password);

  int logging_level() const { return logging_level_; }
  void set_logging_level(int logging_level) {
    logging_level_ = logging_level;
  }

  const std::string& apache_conf() const { return apache_conf_; }
  void set_apache_conf(const std::string& apache_conf) {
    apache_conf_ = apache_conf;
  }

  // Load webserver configuration
  bool LoadWebserverConfig();

  // Get/set webserver configuration.
  const LocalWebServerConfig webserver_config() const { return webserver_config_; }
  void set_webserver_config(const LocalWebServerConfig& webserver_config) {
    webserver_config_ = webserver_config;
  }

protected:
  // Defines the most backup copies of default setting file.
  static const int kMaxBackupSettings = 10;

  // Save current settings to xml document
  // Returns false if saving failed.
  bool SaveToXml(TiXmlDocument &doc, const bool save_all) const;

  TiXmlDocument                 xml_document_;

  // The time duration when service program should backup data to disk
  // in case of power outage and system crash.
  // Unit is second.
  int                           backup_duration_;

  // Whether automatically add new website even if it's not defined in
  // setting file, but exists in web server configuration file. 
  bool                          auto_add_;

  // Setting http server port number. 
  int                           setting_port_;

  // the logging level to control logging output
  int                           logging_level_;

  // path of apache configuration file
  std::string                   apache_conf_;

  // admin related config
  bool                          remote_admin_;
  std::string                   admin_name_;
  std::string                   admin_password_; // encrypted.

  // Global setting for all SiteSetting objects
  SiteSetting                   global_setting_;

  // Stores individual SiteSetting object.
  // SiteSetting is initialized copied from global_setting_, and then
  // updated with data read from web server and setting file.
  std::vector<SiteSetting>      site_settings_;

  // Store webserver configurations.
  LocalWebServerConfig                     webserver_config_;

#ifdef __linux__
private:
  static const char* kDefaultFilePath;
#endif
};


#endif  // COMMON_SITESETTINGS_H__
