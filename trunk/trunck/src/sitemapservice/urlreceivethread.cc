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


#include "sitemapservice/urlreceivethread.h"

#include "common/util.h"
#include "sitemapservice/runtimeinfomanager.h"

bool UrlReceiveThread::Initialize(const SiteSettings& settings,
       const std::map<std::string, SiteDataManager*>& managers) {

  time(&last_update_info_);

  std::vector<SiteSetting> site_settings = settings.site_settings();
  for (int i = 0; i < static_cast<int>(site_settings.size()); ++i) {
    // Check whether webserver filter for specific site is enabled.
    if (!site_settings[i].enabled()
      || !site_settings[i].webserver_filter_setting().enabled()) {
      continue;
    }
    if (managers.find(site_settings[i].site_id()) == managers.end()) {
      continue;
    }
    
    // Create SiteEntry for site.
    SiteEntry newsite;
    newsite.urls_count = 0;
    newsite.data_manager = managers.find(site_settings[i].site_id())->second;

    SiteInfo* siteinfo =
      RuntimeInfoManager::application_info()
      ->site_info(site_settings[i].site_id().c_str());
    if (siteinfo != NULL) {
      newsite.filter_info = NULL;
    } else {
      newsite.filter_info = siteinfo->webserverfilter_info();
    }

    sites_[site_settings[i].site_id()] = newsite;
  }

  // Initialize Url Pipe.
  return pipe_.Initialize(true);
}

void UrlReceiveThread::Run() {
  while (true) {
    UrlRecord* records;
    int count = pipe_.Receive(&records);
    if (count != -1) {
      ProcessRecords(records, count);
    } else {
      Util::Log(EVENT_ERROR, "Error while receiving records.");
    }
    WaitOrDie(0);
  }
}

int UrlReceiveThread::ProcessRecords(UrlRecord *records, int count) {
  for (int i = 0; i < count; ++i) {
    UrlRecord& record = *(records + i);

    Util::Log(EVENT_NORMAL, "UrlRecieved: [%s][%s]",
               record.host, record.url);

    // Use siteid to find the sub-controller
    std::map<std::string, SiteEntry>::iterator itr =
      sites_.find(record.siteid);

    if (itr != sites_.end()) {
      itr->second.data_manager->ProcessRecord(record);
      itr->second.urls_count += 1;
    } else {
      Util::Log(EVENT_NORMAL, "Unrecognized siteid: %s.", record.siteid);
    }
  }

  // Update runtime info every 30 seconds.
  time_t now = time(NULL);
  if (last_update_info_ + 30 <= now) {
    if (RuntimeInfoManager::Lock(true)) {
      std::map<std::string, SiteEntry>::iterator itr = sites_.begin();
      for (; itr != sites_.end(); ++itr) {
        WebServerFilterInfo* info = itr->second.filter_info;
        if (info != NULL) {
          info->set_urls_count(itr->second.urls_count);
        }
      }
      RuntimeInfoManager::Unlock();
    }
    last_update_info_ = now;
  }

  return 0;
}
