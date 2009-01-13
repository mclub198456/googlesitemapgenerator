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


// SiteDataManager is used to manage different kinds of data belonging to a
// specific site. It contains a RecordTable to hold visting records in memory,
// and a HostTable to hold host name in memory. And it also provides mechanisms
// to write in memory data to disk file. For the data on disk, it provides
// methods to merge and update them.

#ifndef SITEMAPSERVICE_SITEDATAMANAGER_H__
#define SITEMAPSERVICE_SITEDATAMANAGER_H__

#include <string>
#include <vector>
#include <set>

#include "common/sitesetting.h"
#include "common/criticalsection.h"
#include "common/urlrecord.h"
#include "common/urlreplacer.h"

#include "sitemapservice/robotstxtfilter.h"
#include "sitemapservice/querystringfilter.h"
#include "sitemapservice/recordtable.h"
#include "sitemapservice/hosttable.h"
#include "sitemapservice/recordfilemanager.h"
#include "sitemapservice/recordmerger.h"
#include "sitemapservice/recordfileio.h"
#include "sitemapservice/siteinfo.h"

class NewsDataManager;

class SiteDataManager {
 public:
  SiteDataManager() {}
  virtual ~SiteDataManager() {}

  // Initialize this manager with setting for a specific site.
  virtual bool Initialize(const SiteSetting& setting) = 0;

  // Update the database.
  virtual bool UpdateDatabase() = 0;
  // Get last updating time of database.
  virtual time_t GetLastUpdate() = 0;
  // Get statistics for database.
  // This value is updated after every invoking of UpdateDatabase method.
  virtual RecordFileStat GetRecordFileStata() = 0;
  
  // Save memory data to database on disk.
  // "flush": whether the data in memory should be cleared after saving.
  // "block": whether this method should be blocked to wait resources.
  virtual bool SaveMemoryData(bool flush, bool block) = 0;
  // The last time when memory data is saved.
  virtual time_t GetLastSave() = 0;

  // Get host name for this site.
  virtual bool GetHostName(std::string* host) = 0;

  // Lock disk data. This should be called before process files on disk.
  virtual bool LockDiskData(bool block) = 0;
  // Unlock disk data. This is the inverse method of LockDiskData.
  virtual void UnlockDiskData() = 0;

  // Get the file manager instance used by this data manager.
  virtual RecordfileManager* GetFileManager() = 0;

  virtual NewsDataManager* GetNewsDataManager() = 0;

  // Process a new record.
  // This record will be added into in memory table or on disk database.
  virtual int ProcessRecord(UrlRecord& record) = 0;
};


class SiteDataManagerImpl : public SiteDataManager {
 public:
  SiteDataManagerImpl();
  virtual ~SiteDataManagerImpl();

  // All public methods are overridden methods.
  // See base class for details.
  virtual bool Initialize(const SiteSetting& setting);
  
  virtual bool UpdateDatabase();
  virtual RecordFileStat GetRecordFileStata();
  virtual time_t GetLastUpdate();

  virtual bool SaveMemoryData(bool flush, bool block);
  virtual time_t GetLastSave();

  virtual bool LockDiskData(bool block);
  virtual void UnlockDiskData();

  virtual bool GetHostName(std::string* host);

  virtual RecordfileManager* GetFileManager();
  
  virtual NewsDataManager* GetNewsDataManager();

  virtual int ProcessRecord(UrlRecord& record);

 private:
  // Max number of obsoleted URLs, which can be held in memory.
  static const int kMaxObsoletedUrl = 1000;

  // Add an status=200 URL to record_table_.
  // If the record_table_ is full, it will be flushed to disk.
  bool AddRecord(const char* host, const char* url, int64 contenthash,
                const time_t& lastmodified, const time_t& filewrite);

  // Last time when record_table_ is saved.
  ThreadSafeVar<time_t> last_table_save_;

  // Last time when data files on disk are merged.
  // A.K.A last time when database is updated.
  ThreadSafeVar<time_t> last_file_merge_;

  // Setting for site.
  SiteSetting setting_;

  // Database statistics.
  // It is updated whenever database is updated.
  ThreadSafeVar<RecordFileStat> recordfile_stat_;

  // Table used to hold records in memory.
  RecordTable* recordtable_;

  // Table used to hold host names in memory.
  HostTable* hosttable_;

  // Used to lock data on disk.
  CriticalSection disk_cs_;

  // Used to lock data in memory.
  CriticalSection memory_cs_;

  // Record file manager for the site.
  // It is used to manage data files on disk.
  RecordfileManager filemanager_;

  // Record merger used merge records from different files.
  RecordMerger* recordmerger_;

  // Last time when runtime information is updated.
  time_t last_update_info_;

  // Runtime information for the site.
  SiteInfo* siteinfo_;

  // set is used to hold obsoleted URLs.
  // 1. Max obsoleted number is limited by memory.
  // 2. We assume that when iterate through std::set, element value is in
  //    increasing order. (This is true under both VC8 and GCC).
  std::set<UrlFprint> obsoleted_;

  // The filter constructed from robots.txt.
  // All coming URLs will be filtered by it before sent to data manager.
  RobotsTxtFilter robotstxt_filter_;

  // Used to filter querystring in url.
  // All coming URLs will be filtered before sent to data manager.
  QueryStringFilter querystring_filter_;

  // Replacer to act on the coming URLs.
  std::vector<UrlReplacer*> replacers_;

  // Data manager for news data.
  NewsDataManager* news_data_manager_;
};


#endif // SITEMAPSERVICE_SITEDATAMANAGER_H__
