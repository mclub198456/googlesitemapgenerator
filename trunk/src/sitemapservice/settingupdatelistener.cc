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


#include "sitemapservice/settingupdatelistener.h"

#include "common/port.h"
#include "common/logger.h"
#include "common/fileutil.h"

bool SettingUpdateListener::Initialize() {  
  return true;
}

void SettingUpdateListener::AddSite(SiteManager* manager) {
   managers_lock_.Enter(true);
   site_managers_[manager->site_id()] = manager;
   managers_lock_.Leave();
}

void SettingUpdateListener::RemoveSite(const std::string& site_id) {  
  managers_lock_.Enter(true);
  site_managers_.erase(site_id);
  managers_lock_.Leave();
}

bool SettingUpdateListener::PreUpdateSetting(const SiteSettings& oldsetting,
                                             const SiteSettings& newsetting) {
   // does nothing.
   return true;
}

bool SettingUpdateListener::PostUpdateSetting(const SiteSettings& oldsetting,
                                              const SiteSettings& newsetting,
                                              bool successful) {
  // does nothing.
  return true;
}

