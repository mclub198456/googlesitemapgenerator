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


#ifndef SITEMAPSERVICE_SITESERVICECONTROLLER_H__
#define SITEMAPSERVICE_SITESERVICECONTROLLER_H__

#include <vector>
#include <set>

#include "common/sitesetting.h"
#include "common/criticalsection.h"
#include "common/urlrecord.h"
#include "common/urlreplacer.h"
#include "sitemapservice/hosttable.h"
#include "sitemapservice/recordtable.h"
#include "sitemapservice/serviceinterface.h"
#include "sitemapservice/recordmerger.h"
#include "sitemapservice/urlrecordprocessor.h"
#include "sitemapservice/robotstxtfilter.h"

class SiteServiceController : public UrlRecordProcessor {
 public:
  // Represents the max waiting time between two service cycle (in seconds).
  // The maximum waiting time is one day.
  static const int kMaxWaitTime = 3600 * 24;

  SiteServiceController();
  ~SiteServiceController();

  // Initialize all the services for the specific site
  int Initialize(const SiteSetting& setting);

  // run all services for the site
  // Returns: waiting time to run service next time (in seconds)
  int RunService();

  // Process given record.
  // The url is filtered by robots.txt and replaced before being added.
  virtual int ProcessRecord(UrlRecord& record);

  // Add the record to record table.
  int AddRecord(const char* host, const char* url, int64 contenthash,
                const time_t& lastmodified, const time_t& filewrite);


  // If waitWhenBlocking is true, we will wait to lock the file and write records to disk. 
  // Otherwise, the method returns directly without writing records to disk, 
  // if the file can't be locked immediately.
  int AutomaticallySave(bool forceflush, bool waitWhenBlocking);

 private:

  void ReleaseResource();
  int RunRegisteredServices(
    std::vector<ServiceInterface*>& available_services,
    const RecordFileStat& stat, const char* host);

 private:
  SiteSetting setting_;
  HostTable* hosttable_;
  RecordTable* recordtable_;
  RecordMerger* merger_;
  std::vector<ServiceInterface*> services_;
  time_t last_save_;

  // The filter constructed from robots.txt.
  RobotsTxtFilter robotstxt_filter_;

  // Replacer to act on the coming URLs.
  std::vector<UrlReplacer*> replacers_;
  
  // Can you explain how to use the CS?
  CriticalSection table_criticalsection_; //read and write, for hosttable_ and recordtable_
  CriticalSection file_criticalsection_;  //read and write, for file system, include current/historical/base/hostdata files

  // set is used to hold obsoleted URLs.
  // 1. Max obsoleted number is limited by memory.
  // 2. We assume that when iterate through std::set, element value is in
  //    increasing order. (This is true under both VC8 and GCC).
  std::set<UrlFprint> obsoleted_;
};

#endif // SITEMAPSERVICE_SITESERVICECONTROLLER_H__

