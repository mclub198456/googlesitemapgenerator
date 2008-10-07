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

#include "common/logger.h"
#include "sitemapservice/runtimeinfomanager.h"

bool UrlReceiveThread::Initialize() {
  time(&last_update_info_);

  // Initialize Url Pipe.
  if (!pipe_.Initialize(true)) {
    Logger::Log(EVENT_ERROR, "Failed to create pipe for url receive thread.");
    return false;
  }

  return true;
}

void UrlReceiveThread::RemoveSite(const std::string& site_id) {
  sites_lock_.Enter(true);
  AutoLeave leave_sites_lock(&sites_lock_);

  sites_.erase(site_id);
}

void UrlReceiveThread::AddSite(SiteManager* manager) {
  sites_lock_.Enter(true);
  AutoLeave leave_sites_lock(&sites_lock_);

  // Create SiteEntry for site.
  SiteEntry newsite;
  newsite.urls_count = 0;
  newsite.site_manager = manager;

  SiteInfo* siteinfo =
    RuntimeInfoManager::application_info()
    ->site_info(manager->site_id().c_str());
  if (siteinfo != NULL) {
    newsite.filter_info = NULL;
  } else {
    newsite.filter_info = siteinfo->webserverfilter_info();
  }

  sites_[manager->site_id()] = newsite;
}

void UrlReceiveThread::Run() {
  while (true) {
    UrlRecord* records;
    int count = pipe_.Receive(&records);
    if (count != -1) {
      ProcessRecords(records, count);
    } else {
      Logger::Log(EVENT_ERROR, "Error while receiving records.");
    }
    WaitOrDie(0);
  }
}

int UrlReceiveThread::ProcessRecords(UrlRecord *records, int count) {
  sites_lock_.Enter(true);
  AutoLeave leave_sites_lock(&sites_lock_);

  for (int i = 0; i < count; ++i) {
    UrlRecord& record = *(records + i);

    Logger::Log(EVENT_NORMAL, "UrlRecieved: [%s][%s]",
               record.host, record.url);

    // Use siteid to find the sub-controller
    std::map<std::string, SiteEntry>::iterator itr =
      sites_.find(record.siteid);

    if (itr != sites_.end()) {
      itr->second.site_manager->ProcessRecord(record);
      itr->second.urls_count += 1;
    } else {
      Logger::Log(EVENT_NORMAL, "Unrecognized siteid: %s.", record.siteid);
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
