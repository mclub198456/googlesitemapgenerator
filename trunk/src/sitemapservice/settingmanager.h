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

#ifndef WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_SETTINGMANAGER_H__
#define WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_SETTINGMANAGER_H__

#include "common/sitesettings.h"

class SettingUpdateListener;

class SettingManager {
public:
  SettingManager();

  // Changes password for login
  int ChangePassword(const std::string& password, 
    const std::string& new_password);

  // Saves XML configuration, does some preprocess
  bool SaveXml(const std::string& xml_string);

  // Saves XML configuration to file
  bool SaveSettingToFile();

  // Gets XML configuration from file
  bool GetXmlSettingFromFile();

  // Checks if remote access is allowed in config file.
  bool AllowRemoteAccess();

  // Checks if the username and password match
  bool Login(const std::string& username, const std::string& password);

  // Returns the password MD5 string
  std::string GetPassword();

  // Set the XML config file update listener, so that the Generator service
  // can be informed online when the XML configuration is updated.
  void SetUpdateListener(SettingUpdateListener* listener);

  // Gets the XML config string, "xml_string" is the output.
  bool GetXmlString(std::string *xml_string);

  // Returns true if the IIS/apache server need to restart to let the setting
  // change take effect.
  bool CheckRestart();

private:
  // file paths
  static const std::string kXmlPath;  

  // The XML configuration content
  // Each HTTP request will trigger a load in case the XML config file has been
  // modified manually.
  SiteSettings settings_; // since the web server is running in single 
  // thread, so we don't need to lock it.

  // True if loading configuration info from webserver succeed.
  bool isSettingsLoadedFromWebServer_;

  // True if the configuration 'settings_' is up-to-date compared to the file
  bool isSettingsLoadedFromFile_;

  // The timestamp for the configuration 'settings_'
  time_t lastModifyTimeForSettingsFile_;

  // The listener for the setting change
  SettingUpdateListener* settingListener_;

  // These two set of settings will be used to judge the necessary to restart
  // the IIS/Apache server.

  // The initial site enable settings
  std::vector<bool> initSiteEnables_;

  // The initial site filter settings
  std::vector<bool> initWebserverFilterEnables_;
};

#endif
