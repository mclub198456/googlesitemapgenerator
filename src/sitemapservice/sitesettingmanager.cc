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

#include "sitemapservice/sitesettingmanager.h"

#include "common/logger.h"
#include "common/fileutil.h"
#include "common/settingmanager.h"

bool SiteSettingManager::GetSiteSetting(const std::string& site_id, std::string* xml) {
  SettingManager* setting_manager = SettingManager::default_instance();
  TiXmlElement* element;
  if (!setting_manager->LoadSiteSetting(site_id, &element)) {
    return false;
  }

  TiXmlDocument xml_doc;
  xml_doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
  xml_doc.LinkEndChild(element);

  // Save document to result string.
  TIXML_OSTREAM out_stream;
  xml_doc.StreamOut(&out_stream);
  xml->assign(out_stream.c_str());

  return true;
}

bool SiteSettingManager::SetSiteSetting(const std::string& site_id,
                                        const std::string& xml) {
  // Load from given xml.
  TiXmlDocument xml_doc;
  xml_doc.Parse(xml.c_str());
  TiXmlElement* node = xml_doc.FirstChildElement("SiteSetting");
  if (node == NULL) {
    Logger::Log(EVENT_ERROR, "RootElement is not SiteSetting [%s].", xml.c_str());
    return false;
  }
  const char* xml_id = node->Attribute("site_id");
  if (xml_id != NULL) {
    if (site_id != std::string(xml_id)) {
      Logger::Log(EVENT_ERROR, "Site id conflict.[%s|%s]", xml_id, site_id.c_str());
      return false;
    }
  } else {
    node->SetAttribute("site_id", site_id.c_str());
  }

  SettingManager* setting_manager = SettingManager::default_instance();
  if (!setting_manager->SaveSiteSetting(node)) {
    return false;
  }

  return true;
}

bool SiteSettingManager::GetSiteSettingToFile(const std::string& site_id,
                                              const std::string& file) {
  std::string xml;
  if (!GetSiteSetting(site_id, &xml)) {
    Logger::Log(EVENT_ERROR, "Failed to get setting for site [%s].",
      site_id.c_str());
    return false;
  }

  if (!FileUtil::WriteFile(file.c_str(), xml)) {
    Logger::Log(EVENT_ERROR, "Failed to save site [%s] setting to file [%s].",
      site_id.c_str(), file.c_str());
    return false;
  }

  return true;
}

bool SiteSettingManager::SetSiteSettingFromFile(const std::string& site_id,
                                                const std::string& file) {
  std::string xml;
  if (!FileUtil::LoadFile(file.c_str(), &xml)) {
    Logger::Log(EVENT_ERROR, "Failed get site [%s] setting from file [%s].",
      site_id.c_str(), file.c_str());
    return false;
  }

  if (!SetSiteSetting(site_id, xml)) {
    Logger::Log(EVENT_ERROR, "Failed to set setting for site [%s].",
      site_id.c_str());
    return false;
  }

  return true;
}