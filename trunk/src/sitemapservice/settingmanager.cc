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

#include "sitemapservice/settingmanager.h"

#include "common/util.h"
#include "common/fileutil.h"
#include "sitemapservice/settingupdatelistener.h"

const std::string SettingManager::kXmlPath = "sitesettings.xml";


SettingManager::SettingManager() {
  isSettingsLoadedFromWebServer_ = settings_.LoadWebserverConfig();
  GetXmlSettingFromFile();

  if (isSettingsLoadedFromWebServer_) {
    const std::vector<SiteSetting>& sites = settings_.site_settings();
    for (size_t i = 0; i < sites.size(); i++) {
      initSiteEnables_.push_back(sites[i].enabled());
      initWebserverFilterEnables_.push_back(
        sites[i].webserver_filter_setting().enabled());
    }    
  } else {
    Util::Log(EVENT_ERROR, "failed to load from web server");
  }
}
int SettingManager::ChangePassword(const std::string& password, 
                                    const std::string& new_password) {
  // verify the old password
  std::string enc_password;
  if (!Util::MD5Encrypt(password.c_str(), &enc_password)) {
    // Encrypt failed
    return -1;
  }

  if (enc_password != settings_.admin_password()) {
    // not correct old password
    Util::Log(EVENT_ERROR, "change password failed, old password is invalid!!");
    return -2;
  }

  // set the new password, pass the plain value to the function
  if (!settings_.ChangeAdminPassword(new_password.c_str())) {
    // change password failed
    return -1;
  }

  if (!SaveSettingToFile()) {
    return -1;
  }

  return 0;
}

bool SettingManager::Login(const std::string& username, 
                           const std::string& password) {
  // check if settings is loaded
  if (!GetXmlSettingFromFile()) {
    Util::Log(EVENT_ERROR, "can not load xml for login"); 
    return false;
  }

  // check password MD5 and username
  return settings_.admin_password() == password && 
    settings_.admin_name() == username;
}

bool SettingManager::GetXmlString(std::string *xml_string) {
  return GetXmlSettingFromFile() && settings_.SaveToString(xml_string);
}


std::string SettingManager::GetPassword() {
  return settings_.admin_password();
}

bool SettingManager::AllowRemoteAccess() {
  if (!GetXmlSettingFromFile()) {
    // if can't get the exact value, we assume it is not allowed.
    return false;
  }

  return settings_.remote_admin();
}

bool SettingManager::SaveXml(const std::string& xml_string) {
  // check settings load from file
#ifdef _DEBUG
  FILE* fd = fopen("xml.txt", "w");
  fwrite(xml_string.c_str(), sizeof(char), xml_string.length(), fd);
  fclose(fd);
#endif

  if (!GetXmlSettingFromFile()) {
    return false;
  }

  // load new settings from client XML string
  //SiteSettings newsettings;
  //if (!newsettings.LoadWebserverConfig() ||
  //    !newsettings.LoadFromString(xml_string.c_str())) {
  //  return false;
  //}
  SiteSettings oldsettings = settings_;
  if (!settings_.LoadFromString(xml_string.c_str())) {
    Util::Log(EVENT_CRITICAL, "LoadFromString failed, need restart!!");
    isSettingsLoadedFromFile_ = false;
    return false;
  }

  // check if user change the password
  if (settings_.admin_password() != oldsettings.admin_password()) {
    // since javascript will not encrypt the new password if user change the 
    // password, so check again in case the password is plain-text.   
    if (!settings_.ChangeAdminPassword(settings_.admin_password().c_str())) {
      // Encrypt failed
      isSettingsLoadedFromFile_ = false;
      return false;
    }

    if (settings_.admin_password() !=  oldsettings.admin_password()) {
      // if password has been changed, encrypt plain-text Password to MD5
      Util::Log(EVENT_ERROR, "password has changed in a wrong way!!");
      isSettingsLoadedFromFile_ = false;
      return false;
    }
  }    


  // save new settings to settings_, direct copy will cause problem.
  //settings_ = newsettings;  

  if (!settingListener_->PreUpdateSetting(oldsettings, settings_)) {
    Util::Log(EVENT_ERROR, "Pre-update setting failed.");
    isSettingsLoadedFromFile_ = false;
    return false;
  }

  // save settings_ to file
  bool save_result = SaveSettingToFile();
  isSettingsLoadedFromFile_ = save_result;
  if (!settingListener_->PostUpdateSetting(oldsettings, settings_, save_result)) {
    Util::Log(EVENT_ERROR, "Post-update setting failed. (Ignore).");
  }

  return save_result;
}

bool SettingManager::SaveSettingToFile() {
  // Preupdate
  // Backup old settings first.
  if (!SiteSettings::BackupDefaultSettingFile()) {
    return false;
  }

  // save new settings to file
  const std::string filepath = SiteSettings::GetDefaultFilePath();
  if (!settings_.SaveToFile(filepath.c_str())) {
    return false;
  }

  FileAttribute attr;
  if (FileUtil::GetFileAttribute(filepath.c_str(), &attr)) {
    lastModifyTimeForSettingsFile_ = attr.last_modified;
  } else {
    Util::Log(EVENT_ERROR, "failed to get file last modify time");
  }

  return true;
}

bool SettingManager::GetXmlSettingFromFile() {
  //Begin to create default XML object.

  if (!isSettingsLoadedFromWebServer_) {
    return false;
  }

  const std::string filepath = SiteSettings::GetDefaultFilePath();
  FileAttribute attr;
  if (!FileUtil::GetFileAttribute(filepath.c_str(), &attr)) {
    Util::Log(EVENT_ERROR, "failed to get file last modify time");
    return false;
  }

  // Checks if the setting is up-to-date.
  if (!isSettingsLoadedFromFile_ || 
    lastModifyTimeForSettingsFile_ != attr.last_modified) {
      lastModifyTimeForSettingsFile_ = attr.last_modified;
      isSettingsLoadedFromFile_ = settings_.LoadFromFile(filepath.c_str());
  }

  return isSettingsLoadedFromFile_;
}

void SettingManager::SetUpdateListener(SettingUpdateListener* listener) {
  settingListener_ = listener;
}


bool SettingManager::CheckRestart() {
  const std::vector<SiteSetting>& sites = settings_.site_settings();
  for (size_t i = 0; i < sites.size(); i++) {
    bool oldEnable = initSiteEnables_[i] && initWebserverFilterEnables_[i];
    bool newEnable = sites[i].enabled() && 
      sites[i].webserver_filter_setting().enabled();
    if (oldEnable != newEnable) {
      return true;
    }
  }
  return false;
}