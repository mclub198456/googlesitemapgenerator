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


// UrlProviderService is a base class for potential services which can provide
// url to SiteDataManager. It hiddes all the details to send url to site data
// manager, to update runtime information and to implement ServiceInterface.
// The only thing the sub-clas requires to do is to implement InternalRun
// method and use ProvideRecord method to provide url records.
// Besides that, this class also persists last running timestamp into disk. It
// provide method to load timestamp from disk, as well as method to save
// timestamp to disk.

#ifndef SITEMAPSERVICE_URLPROVIDERSERVICE_H__
#define SITEMAPSERVICE_URLPROVIDERSERVICE_H__

#include <string>

#include "common/basictypes.h"
#include "common/sitesetting.h"
#include "sitemapservice/serviceinterface.h"
#include "sitemapservice/sitedatamanager.h"
#include "sitemapservice/siteinfo.h"

class UrlProviderService : public ServiceInterface {
 public:
  UrlProviderService();
  virtual ~UrlProviderService() {}

  // Initialize method. Simply save the args for later use.
  virtual bool Initialize(SiteDataManager* data_manager,
                          const SiteSetting& setting);

  // Overriden.
  // It delegates the actual task to InternalRun, which is a template method.
  // Besides that, this method also persists the running timestamp, and updates
  // runtime information.
  virtual void Run();

  virtual int GetWaitTime();

  virtual int GetRunningPeriod() {
    return running_period_;
  }

  // Try to reload timestamp from file.
  bool RefreshTimeStamp();

  // Get last running value.
  const time_t& last_access_limit() const { return last_access_limit_; }
  void set_last_access_limit(const time_t& t) {
    last_access_limit_ = t;
  }

  // Save last access limit into file on disk.
  void SaveLastAccessLimit();

  // Set the dir to hold time stamp.
  // The default directory is Util::GetApplicationDir().
  static bool ChangeStampDir(const char* dir);

 protected:
  // Method used by base-class.
  bool Initialize(const std::string suffix,
    UrlProviderInfo* runtime_info, int running_period);

  // Template method doing the actual work.
  virtual bool InternalRun() = 0;

  // Sub-class should use this method to provide record to site data manager.
  void ProvideRecord(UrlRecord& record);

  // The runtime info belongs to this task.
  UrlProviderInfo* runtime_info_;

  SiteSetting sitesetting_;

  SiteDataManager* data_manager_;

 private:
  // The limitation on last acess value of url record.
  // Url record which is visited before this value will be discarded.
  time_t last_access_limit_;

  // Temporary value for last_access_limit_.
  time_t last_access_limit_tmp_;

  // Last running time of this service.
  time_t last_run_;

  // Running period.
  int running_period_;

  // Url counter for a Run.
  int url_counter_;

  // The file used to hold time stamp.
  std::string timestamp_file_;

  DISALLOW_EVIL_CONSTRUCTORS(UrlProviderService);
};

#endif // SITEMAPSERVICE_URLPROVIDERSERVICE_H__
