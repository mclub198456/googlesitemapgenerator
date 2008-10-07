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

// This file defines a class that manages the settings.
// It manages the settings object in the memory, as well as synchronizing with 
// the configuration XML file on the disk. It also provides some functions to 
// access information of the settings.

#ifndef WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_HTTPSETTINGMANAGER_H__
#define WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_HTTPSETTINGMANAGER_H__

#include "common/sitesettings.h"

class SettingUpdateListener;

class HttpSettingManager {
public:
  HttpSettingManager();

  // Saves XML configuration, does some preprocess
  bool SaveXml(const std::string& xml_string);

  // Saves XML configuration to file
  bool SaveSettingToFile();

  // Gets XML configuration from file
  bool GetXmlSettingFromFile();

  // Checks if remote access is allowed in config file.
  bool AllowRemoteAccess();

  // Get password string hashed with salt.
  std::string GetPassword();

  // Try to login with password from client.
  bool Login(const std::string& password);

  // Set the XML config file update listener, so that the Generator service
  // can be informed online when the XML configuration is updated.
  void SetUpdateListener(SettingUpdateListener* listener);

  // Gets the XML config string, "xml_string" is the output.
  bool GetXmlString(std::string *xml_string);

  // Gets the XML config file's timestamp
  bool GetLastModifiedTime(time_t* ts);

private:
  // The XML configuration content
  // Each HTTP request will trigger a load in case the XML config file has been
  // modified manually.
  SiteSettings settings_; // since the web server is running in single 
  // thread, so we don't need to lock it.

  // True if loading configuration info from webserver succeed.
  bool is_settings_loaded_from_web_server_;

  // True if the configuration 'settings_' is up-to-date compared to the file
  bool is_settings_loaded_from_file_;

  // The timestamp for the configuration 'settings_'
  time_t last_modify_time_for_settings_file_;

  // The listener for the setting change
  SettingUpdateListener* setting_listener_;

  // These two set of settings will be used to judge the necessary to restart
  // the IIS/Apache server.

  // The initial site enable settings
  std::vector<bool> init_site_enables_;

  // The initial site filter settings
  std::vector<bool> init_web_server_filter_enables_;
};

#endif // WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_HTTPSETTINGMANAGER_H__
