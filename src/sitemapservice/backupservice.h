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


// BackupService is used to save visiting records in memory into disk. After
// saving data in memory, it doesn't clear it from memory. This service is
// used to avoid abnormal application exiting.
// This service runns periodically. If data is saved by other part of program
// since last running, this service would skip saving this time. But it is
// ensured that the data in memory is saved at least once for each backup
// duration.
// It can be only used for one site. Thus different BackService objects should
// be created for different sites.
// This class is thread-safe.

#ifndef SITEMAPSERVICE_BACKUPSERVICE_H__
#define SITEMAPSERVICE_BACKUPSERVICE_H__

#include "sitemapservice/serviceinterface.h"
#include "sitemapservice/sitedatamanager.h"

class BackupService : public ServiceInterface {
 public:
  BackupService() {}
  virtual ~BackupService() {}

  // Initialize this service.
  // "data_manager" and "setting" should belong to same site.
  bool Initialize(SiteDataManager* data_manager, const SiteSetting& setting);

  // Get waiting time (in seconds) to run this service next time.
  virtual int GetWaitTime();

  // Get running period (in seconds) of this service.
  virtual int GetRunningPeriod();

  // Run the backup service.
  // If the data is saved by other part of application between last running and
  // this running, it skips to save this time, and adjusts waiting time to wait
  // this service to run again.
  virtual void Run();

  static void SetBackupDuration(int duration);

private:
  // Represents data should be saved at most every "backup_duration_" seconds.
  static ThreadSafeVar<int> backup_duration_;

  // The actual saving call is delegated to this SiteDataManager object.
  // This service itself just schedules the saving task.
  SiteDataManager* sitedata_manager_;

  // "site_id_" represents the site for which this service is used.
  std::string site_id_;

  // Represents the last running time of this service.
  // It is used to calculate the waiting time of this service.
  time_t last_run_;
};

#endif // SITEMAPSERVICE_BACKUPSERVICE_H__

