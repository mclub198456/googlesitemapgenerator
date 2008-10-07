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
#include "common/webserverconfig.h"

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
  virtual bool LoadSetting(TiXmlElement* element);

  // Save settings to xml_node_.
  virtual TiXmlElement* SaveSetting();

  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  virtual bool Equals(const BaseSetting* another) const;
  
  // Convert values from UTF8 to system encoding.
  // Only values related to system file name are converted.
  // e.g. site physical path, log path, sitemap file names, and etc.
  bool ToSystemEncoding();

    // Load settings from an xml file from disk.
  // If merge_webserver_config is true, configuration from webserver will
  // be merged according to auto_add flag.
  bool LoadFromFile(const char *file);

  // Load settings from xml string.
  // If merge_webserver_config is true, configuration from webserver will
  // be merged according to auto_add flag.
  bool LoadFromString(const char *xml_string);

  // Save current settings to a file.
  // Returns false if saving failed.
  bool SaveToFile(const char *file);

  // Save current settings to a string.
  // Returns false if saving failed.
  bool SaveToString(std::string *xml_string);

  // Merge site settings from webserver.
  void MergeSetting(const WebserverConfig* webserver_config);

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
  }

  // get/set auto_add.
  const bool auto_add() const { return auto_add_; }
  void set_auto_add(bool auto_add) {
    auto_add_ = auto_add;
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
  }

  // set_admin_password accepts already encrypted pwd,
  // which is hash(raw_password + salt).
  const std::string& admin_password() { return admin_password_; }
  void set_admin_password(const std::string& admin_password) {
    admin_password_ = admin_password;
  }

  const std::string& password_salt() { return password_salt_; }
  void set_password_salt(const std::string& password_salt) {
    password_salt_ = password_salt;
  }
  
  int logging_level() const { return logging_level_; }
  void set_logging_level(int logging_level) {
    logging_level_ = logging_level;
  }

  const std::string& apache_conf() const { return apache_conf_; }
  void set_apache_conf(const std::string& apache_conf) {
    apache_conf_ = apache_conf;
  }

 private:

  // The time duration when service program should backup data to disk
  // in case of power outage and system crash.
  // Unit is second.
  int                           backup_duration_;

  // Whether automatically add new website even if it's not defined in
  // setting file, but exists in web server configuration file. 
  bool                          auto_add_;

  // the logging level to control logging output
  int                           logging_level_;

  // path of apache configuration file
  std::string                   apache_conf_;

  // admin related config
  bool                          remote_admin_;
  std::string                   admin_name_;
  std::string                   admin_password_; // encrypted.
  std::string                   password_salt_;

  // Global setting for all SiteSetting objects
  SiteSetting                   global_setting_;

  // Stores individual SiteSetting object.
  // SiteSetting is initialized copied from global_setting_, and then
  // updated with data read from web server and setting file.
  std::vector<SiteSetting>      site_settings_;
};

#endif  // COMMON_SITESETTINGS_H__
