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


#include "sitemapservice/backupservice.h"

#include "common/logger.h"

ThreadSafeVar<int> BackupService::backup_duration_(600);

bool BackupService::Initialize(SiteDataManager* datamgr,
                               const SiteSetting& setting) {
  sitedata_manager_ = datamgr;
  site_id_ = setting.site_id();

  time(&last_run_);
  return true;
}

int BackupService::GetWaitTime() {
  time_t next = last_run_ + backup_duration_, now;
  time(&now);

  if (next <= now) {
    return 0;
  } else {
    return static_cast<int>(next - now);
  }
}

int BackupService::GetRunningPeriod() {
  return backup_duration_;
}

void BackupService::SetBackupDuration(int duration) {
  backup_duration_ = duration;
}

void BackupService::Run() {
  // Check whether data was saved during last running and now.
  // If it is saved, last_run_ is set as the saving time.
  time_t last_save = sitedata_manager_->GetLastSave(), now;
  time(&now);

  if (last_save + backup_duration_ > now) {
    Logger::Log(EVENT_NORMAL, "Data for [%s] is already saved.",
              site_id_.c_str());
    last_run_ = last_save;
    return;
  }

  // Do actual saving, and change last_run_ time.
  if (!sitedata_manager_->SaveMemoryData(false, true)) {
    Logger::Log(EVENT_ERROR, "Failed to save memory data for [%s].",
              site_id_.c_str());
  } else {
    Logger::Log(EVENT_IMPORTANT, "Save memory data for [%s] successfully.",
              site_id_.c_str());
  }
  time(&last_run_);
}
