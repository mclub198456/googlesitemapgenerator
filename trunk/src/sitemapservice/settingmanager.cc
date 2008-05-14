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
  is_settings_loaded_from_web_server_ = settings_.LoadWebserverConfig();
  GetXmlSettingFromFile();

  if (is_settings_loaded_from_web_server_) {
    const std::vector<SiteSetting>& sites = settings_.site_settings();
    for (size_t i = 0; i < sites.size(); i++) {
      init_site_enables_.push_back(sites[i].enabled());
      init_web_server_filter_enables_.push_back(
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
    is_settings_loaded_from_file_ = false;
    return false;
  }

  // check if user change the password
  if (settings_.admin_password() != oldsettings.admin_password()) {
    // since javascript will not encrypt the new password if user change the 
    // password, so check again in case the password is plain-text.   
    if (!settings_.ChangeAdminPassword(settings_.admin_password().c_str())) {
      // Encrypt failed
      is_settings_loaded_from_file_ = false;
      return false;
    }

    if (settings_.admin_password() !=  oldsettings.admin_password()) {
      // if password has been changed, encrypt plain-text Password to MD5
      Util::Log(EVENT_ERROR, "password has changed in a wrong way!!");
      is_settings_loaded_from_file_ = false;
      return false;
    }
  }    


  // save new settings to settings_, direct copy will cause problem.
  //settings_ = newsettings;  

  if (!setting_listener_->PreUpdateSetting(oldsettings, settings_)) {
    Util::Log(EVENT_ERROR, "Pre-update setting failed.");
    is_settings_loaded_from_file_ = false;
    return false;
  }

  // save settings_ to file
  bool save_result = SaveSettingToFile();
  is_settings_loaded_from_file_ = save_result;
  if (!setting_listener_->PostUpdateSetting(oldsettings, settings_, 
                                           save_result)) {
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
  
  return GetLastModifiedTime(&last_modify_time_for_settings_file_);
}

bool SettingManager::GetLastModifiedTime(time_t* ts) {
  const std::string filepath = SiteSettings::GetDefaultFilePath();
  FileAttribute attr;
  if (!FileUtil::GetFileAttribute(filepath.c_str(), &attr)) {
    Util::Log(EVENT_ERROR, "failed to get file last modify time");
    return false;
  }

  *ts = attr.last_modified;
  return true;
}

bool SettingManager::GetXmlSettingFromFile() {
  //Begin to create default XML object.

  if (!is_settings_loaded_from_web_server_) {
    return false;
  }

  time_t ts;
  if (!GetLastModifiedTime(&ts)) {
    return false;
  }

  // Checks if the setting is up-to-date.
  if (!is_settings_loaded_from_file_ || 
      last_modify_time_for_settings_file_ != ts) {
    last_modify_time_for_settings_file_ = ts;
    is_settings_loaded_from_file_ = 
        settings_.LoadFromFile(SiteSettings::GetDefaultFilePath().c_str());
  }

  return is_settings_loaded_from_file_;
}

void SettingManager::SetUpdateListener(SettingUpdateListener* listener) {
  setting_listener_ = listener;
}


bool SettingManager::CheckRestart() {
  const std::vector<SiteSetting>& sites = settings_.site_settings();
  for (size_t i = 0; i < sites.size(); i++) {
    bool old_enable = init_site_enables_[i] && init_web_server_filter_enables_[i];
    bool new_enable = sites[i].enabled() && 
      sites[i].webserver_filter_setting().enabled();
    if (old_enable != new_enable) {
      return true;
    }
  }
  return false;
}