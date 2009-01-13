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


#include "sitemapservice/websitemapservice.h"

#include "common/fileutil.h"
#include "common/logger.h"
#include "common/settingmanager.h"
#include "sitemapservice/sitemapwriter.h"
#include "sitemapservice/runtimeinfomanager.h"

WebSitemapService::WebSitemapService(): 
    PlainSitemapService("Web Sitemap", new XmlSitemapWriter()) {
  // does nothing.
}

WebSitemapService::~WebSitemapService() {
}

bool WebSitemapService::Initialize(SiteDataManager* datamanager,
                                   const SiteSetting& setting) {
  if (!PlainSitemapService::Initialize(datamanager, setting)) {
    return false;
  }

  web_sitemap_setting_ = setting.web_sitemap_setting();

  // Get the runtime info structure from runtime info tree.
  SitemapServiceInfo* info = NULL;
  SiteInfo* site_info = RuntimeInfoManager::application_info()
    ->site_info(setting.site_id().c_str());
  if (site_info != NULL) {
    info = site_info->web_sitemapservice_info();
  }

  return PlainSitemapService::Initialize(&web_sitemap_setting_, info);
}

bool WebSitemapService::End() {
  bool result = PlainSitemapService::End();

  // Construct robots.txt full path.
  std::string robotstxt_path(sitesetting_.physical_path());
  robotstxt_path.append("/robots.txt");
  if (!UpdateRobotsTxt(robotstxt_path)) {
    Logger::Log(EVENT_ERROR, "Ignore update robots.txt error. [%s]",
      robotstxt_path.c_str());
  }

  return result;
}

bool WebSitemapService::UpdateRobotsTxt(const std::string& robotstxt_path) {
  const char* kAddFlag = "# Added by Google Sitemap Generator";

  
  // Load old content of robots.txt.
  std::vector<std::string> robots_lines;
  bool robots_exist = FileUtil::Exists(robotstxt_path.c_str());
  if (robots_exist) {
    if (!FileUtil::LoadLines(robotstxt_path.c_str(), &robots_lines)) {
      return false;
    }
  }

  // Find old sitemap submission lines.
  int sitemap_line = -1;
  for (int i = 0; i < (int) robots_lines.size(); ++i) {
    if (robots_lines[i].find(kAddFlag) != std::string::npos) {
      sitemap_line = i;
      break;
    }
  }

  // Check whether the lines need updating.
  bool updated = false;
  if (web_sitemap_setting_.included_in_robots_txt()) {
    std::string new_sitemap_line(BuildSitemapUrl());
    new_sitemap_line.append(" ").append(kAddFlag);
    if (sitemap_line != -1) {
      if (robots_lines[sitemap_line] != new_sitemap_line) {
        robots_lines[sitemap_line] = new_sitemap_line;
        updated = true;
      }
    } else {
      robots_lines.push_back(new_sitemap_line);
      updated = true;
    }
  } else {
    if (sitemap_line != -1) {
      robots_lines.erase(robots_lines.begin() + sitemap_line);
      updated = true;
    }
  }

  if (updated) {
    if (!FileUtil::SaveLines(robotstxt_path.c_str(), robots_lines)) {
      return false;
    }
  }
  
  return true;
}

bool WebSitemapService::CleanRobotsTxt() {
  const char* kAddFlag = "# Added by Google Sitemap Generator";
  SettingManager* setting_manager = SettingManager::default_instance();
  SiteSettings settings;
  if (!setting_manager->LoadSetting(&settings, true)) {
    Logger::Log(EVENT_ERROR, "Failed to load setting when clear robots.txt.");
    return false;
  }

  const std::vector<SiteSetting>& site_settings = settings.site_settings();
  for (int i = 0; i < (int) site_settings.size(); ++i) {
    std::string robots_txt(site_settings[i].physical_path());
    robots_txt.append("/robots.txt");
    if (!FileUtil::Exists(robots_txt.c_str())) {
      continue;
    }

    std::vector<std::string> robots_lines;
    if (!FileUtil::LoadLines(robots_txt.c_str(), &robots_lines)) {
      Logger::Log(EVENT_ERROR, "Failed to load robots.txt to clear. Skip.");
      continue;
    }

    for (int j = 0; j < (int) robots_lines.size(); ++j) {
      if (robots_lines[i].find(kAddFlag) != std::string::npos) {
        robots_lines[i].clear();
        Logger::Log(EVENT_CRITICAL, "Clear (%s)", robots_txt.c_str());
        break;
      }
    }

    if (!FileUtil::SaveLines(robots_txt.c_str(), robots_lines)) {
      Logger::Log(EVENT_ERROR, "Failed to save robots.txt to clear. Skip.");
      continue;
    }
  }

  return true;
}

