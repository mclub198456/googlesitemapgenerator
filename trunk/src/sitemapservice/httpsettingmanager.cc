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

#include "sitemapservice/httpsettingmanager.h"

#include "common/logger.h"
#include "common/util.h"
#include "common/timesupport.h"
#include "common/settingmanager.h"
#include "common/fileutil.h"
#include "sitemapservice/settingupdatelistener.h"

HttpSettingManager::HttpSettingManager() {
  SettingManager* setting_manager = SettingManager::default_instance();
  is_settings_loaded_from_web_server_ = setting_manager->ReloadWebserverConfig();
  GetXmlSettingFromFile();

  if (is_settings_loaded_from_web_server_) {
    const std::vector<SiteSetting>& sites = settings_.site_settings();
    for (size_t i = 0; i < sites.size(); i++) {
      init_site_enables_.push_back(sites[i].enabled());
      init_web_server_filter_enables_.push_back(
        sites[i].webserver_filter_setting().enabled());
    }    
  } else {
    Logger::Log(EVENT_ERROR, "failed to load from web server");
  }
}

bool HttpSettingManager::GetXmlString(std::string *xml_string) {
  if (!GetXmlSettingFromFile()) {
    return false;
  }

  time_t last_modified;
  if (!GetLastModifiedTime(&last_modified)) {
    return false;
  }

  // Save setting, hide salt and password, set last_modified.
  TiXmlElement* element = settings_.SaveSetting();
  element->SetAttribute("admin_password", "******");
  element->SetAttribute("password_salt", "******");
  element->SetAttribute("last_modified", FormatHttpDate(last_modified).c_str());

  // Create a xml doc to hold the element.
  TiXmlDocument xml_doc;
  xml_doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
  xml_doc.LinkEndChild(element);

  // Save doc to a xml std::string.
  TiXmlOutStream out;
  xml_doc.StreamOut(&out);
  xml_string->assign(out.c_str());

  return true;
}

std::string HttpSettingManager::GetPassword() {
  GetXmlSettingFromFile();
  return settings_.admin_password();
}

bool HttpSettingManager::Login(const std::string& password) {
  // Try to get lateset setting.
  GetXmlSettingFromFile();
  if (settings_.admin_password().length() == 0) {
    Logger::Log(EVENT_ERROR, "Password has not been set.");
  }

  // Add salt and hash password.
  std::string passwd(password);
  passwd.append(settings_.password_salt());
  std::string encrypted;
  if (!Util::MD5Encrypt(passwd.c_str(), &encrypted)) {
    Logger::Log(EVENT_ERROR, "Failed to hash password.");
    return false;
  }

  if (encrypted != settings_.admin_password()) {
    Logger::Log(EVENT_ERROR, "Invalid password [%s]", password.c_str());
    return false;
  }

  return true;
}

bool HttpSettingManager::AllowRemoteAccess() {
  return GetXmlSettingFromFile() && settings_.remote_admin();
}

bool HttpSettingManager::SaveXml(const std::string& xml_string) {
  // load latest settings from file
  if (!GetXmlSettingFromFile()) {
    return false;
  }

  // record old settings
  SiteSettings oldsettings = settings_;

  // load new settings from client XML string
  if (!settings_.LoadFromString(xml_string.c_str())) {
    Logger::Log(EVENT_CRITICAL, "LoadFromString failed, need restart!!");
    is_settings_loaded_from_file_ = false;
    return false;
  }

  // recover the password and salt
  settings_.set_admin_password(oldsettings.admin_password());
  settings_.set_password_salt(oldsettings.password_salt());

  // inform setting listener
  if (!setting_listener_->PreUpdateSetting(oldsettings, settings_)) {
    Logger::Log(EVENT_ERROR, "Pre-update setting failed.");
    is_settings_loaded_from_file_ = false;
    return false;
  }

  // save settings_ to file
  bool save_result = SaveSettingToFile();
  is_settings_loaded_from_file_ = save_result;
  if (!setting_listener_->PostUpdateSetting(oldsettings, settings_, 
                                           save_result)) {
    Logger::Log(EVENT_ERROR, "Post-update setting failed. (Ignore).");
  }

  return save_result;
}

bool HttpSettingManager::SaveSettingToFile() {
  SettingManager* setting_manager = SettingManager::default_instance();

  // save new settings to file
  if (!setting_manager->SaveSetting(settings_, true)) {
    return false;
  }
  
  return GetLastModifiedTime(&last_modify_time_for_settings_file_);
}

bool HttpSettingManager::GetLastModifiedTime(time_t* ts) {
  std::string filepath = SettingManager::default_instance()->setting_file();
  FileAttribute attr;
  if (!FileUtil::GetFileAttribute(filepath.c_str(), &attr)) {
    Logger::Log(EVENT_ERROR, "failed to get file last modify time");
    return false;
  }

  *ts = attr.last_modified;
  return true;
}

bool HttpSettingManager::GetXmlSettingFromFile() {
  //Begin to create default XML object.
  do {
    if (!is_settings_loaded_from_web_server_) {
      break;
    }

    time_t ts;
    if (!GetLastModifiedTime(&ts)) {
      break;
    }

    // Checks if the setting is up-to-date.
    if (!is_settings_loaded_from_file_ || 
      last_modify_time_for_settings_file_ != ts) {
      last_modify_time_for_settings_file_ = ts;
      is_settings_loaded_from_file_ =
        SettingManager::default_instance()->LoadSetting(&settings_, true);
    }

    if (!is_settings_loaded_from_file_) {
      break;
    }

    return true;

  } while(false);
  Logger::Log(EVENT_ERROR, "cannot load XML settings from file!"); 
  return false;
}

void HttpSettingManager::SetUpdateListener(SettingUpdateListener* listener) {
  setting_listener_ = listener;
}

