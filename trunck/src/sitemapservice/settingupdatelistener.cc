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


#include "sitemapservice/settingupdatelistener.h"

#include "common/port.h"
#include "common/util.h"
#include "common/fileutil.h"

bool SettingUpdateListener::Initialize(
       const std::map<std::string, SiteDataManager*>& managers) {

  data_managers_ = managers;
  
  return true;
}

bool SettingUpdateListener::PreUpdateSetting(const SiteSettings& oldsetting,
                                             const SiteSettings& newsetting) {
   // Update robots.txt
   const std::vector<SiteSetting>& new_sites = newsetting.site_settings();
   for (int i = 0; i < static_cast<int>(new_sites.size()); ++i) {
     if (data_managers_.find(new_sites[i].site_id()) == data_managers_.end()) {
       Util::Log(EVENT_CRITICAL, "Site [%s] is not running, and robots.txt can't be updated.",
         new_sites[i].site_id().c_str());
       continue;
     }

     std::string filename(new_sites[i].web_sitemap_setting().file_name());
     if (new_sites[i].web_sitemap_setting().compress()) {
       if (filename.length() >= 3 &&
         stricmp(filename.substr(filename.length() - 3).c_str(), ".gz") != 0) {
         filename.append(".gz");
       }
     }

     bool result = data_managers_[new_sites[i].site_id()]->UpdateRobotsTxt(
       new_sites[i].web_sitemap_setting().included_in_robots_txt(),
       filename.c_str());
     if (!result) {
       return false;
     }
   }

   return true;
}

bool SettingUpdateListener::PostUpdateSetting(const SiteSettings& oldsetting,
                                              const SiteSettings& newsetting,
                                              bool successful) {
  if (successful) {
    // does nothing.
    return true;
  }

  // Roll back robots.txt
  // WARN!!!
  // This is incorrect if logic in PreUpdateSetting is changed.
  return PreUpdateSetting(newsetting, oldsetting);
}

