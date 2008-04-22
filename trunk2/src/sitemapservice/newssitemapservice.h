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


// This service is used to generate news sitemap. News sitemap is a special
// sitemap, which is different from other sitemaps.
// From the view of time, only the latest news is included in sitemap.
// From the view of size, only 1000 URLs can be included, and no sitemap index
// file can be generated.

#ifndef SITEMAPSERVICE_NEWSSITEMAPSERVICE_H__
#define SITEMAPSERVICE_NEWSSITEMAPSERVICE_H__

#include <algorithm>
#include <queue>
#include <string>
#include <vector>

#include "common/sitesetting.h"
#include "sitemapservice/basesitemapservice.h"
#include "sitemapservice/newsdatamanager.h"

class NewsSitemapService : public BaseSitemapService {
 public:
  NewsSitemapService();
  virtual ~NewsSitemapService();

  // Overridden method.
  // It extracts news sitemap setting from given site setting, extracts
  // run-time information pointer from RuntimeInfoManager, and passes these two
  // extracted values to base class.
  virtual bool Initialize(SiteDataManager* datamanager,
                          const SiteSetting& setting);

  
  // These three methods are three steps to generate a sitemap. They are used
  // in "InternalRun" method. (See method doc for it)
  // These methods are exposed as public only for testing purpose.
  virtual bool Start(const RecordFileStat& stat, const char* host);
  virtual bool ProcessRecord(const VisitingRecord& record);
  virtual bool End();

 protected:
  // Overrides method inherited from base class.
  // It updates news database for the site, reads url record one by one from
  // the database, and feeds the url to Start/ProcessRecord/End methods.
  virtual bool InternalRun();

 private:
  // Comparer used to sort news records.
  class RecordComparer {
  public:
    bool operator()(const VisitingRecord& a, const VisitingRecord& b) const {
      return GetPublicationDate(a) > GetPublicationDate(b);
    }
  };

  typedef std::priority_queue<VisitingRecord, std::vector<VisitingRecord>,
                              RecordComparer> Heap;

  // Get publication date of news.
  static time_t GetPublicationDate(const VisitingRecord& record) {
    return std::min<time_t>(record.first_appear, record.last_change);
  }

  // The directory name used to store news database.
  // It is a sub-directory of site's data directory.
  static const std::string kDataDir;

  // Convert a visiting record to a UrlElement for news sitemap.
  UrlElement ConvertRecord(const VisitingRecord& record);

  // It is a min heap on news publication date.
  // Only the latest 2 * news_sitemap_setting.max_url_number() news are kept
  // in this heap.
  Heap records_;

  // Host value used for news sitemap.
  std::string host_;

  // Setting for this service.
  NewsSitemapSetting  news_sitemap_setting_;

  // Cutdown time for news.
  // News whose publication date is older than this value would be excluded.
  time_t              cutdown_;

  // Manager used to manage news data.
  NewsDataManager* newsdata_manager_;
};


#endif // SITEMAPSERVICE_NEWSSITEMAPSERVICE_H__


