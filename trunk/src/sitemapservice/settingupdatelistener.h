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


// This class is an listener on setting updating event.
// It provides two methods, PreUpdateSetting and PostUpdateSetting.
// These two methods should be called before and after setting is updated.

#ifndef SITEMAPSERVICE_SETTINGUPDATELISTENER_H__
#define SITEMAPSERVICE_SETTINGUPDATELISTENER_H__

#include "common/sitesettings.h"
#include "sitemapservice/sitemanager.h"

class SettingUpdateListener {
 public:
  SettingUpdateListener() {}
  ~SettingUpdateListener() {}

  bool Initialize();

  void RemoveSite(const std::string& site_id);

  void AddSite(SiteManager* manager);

  // Method that should be called before update setting.
  bool PreUpdateSetting(const SiteSettings& oldsetting,
                        const SiteSettings& newsetting);

  // Method that should be called after update setting.
  bool PostUpdateSetting(const SiteSettings& oldsetting,
                         const SiteSettings& newsetting,
                         bool successful);

 private:
  std::map<std::string, SiteManager*> site_managers_;

  // Access lock for data_managers_ value.
  CriticalSection managers_lock_;
};

#endif // SITEMAPSERVICE_SETTINGUPDATELISTENER_H__

