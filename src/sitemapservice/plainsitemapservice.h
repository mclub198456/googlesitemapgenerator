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


// This class defines a sitemap service which can generate plain sitemap.
// A plain sitemap processes all the historical URLs, and it only discards
// URLs depending or included/excluded url filters. A reversed example is
// news sitemap, which only accepts the newest URLs and thus not a plain
// sitemap.
// This class is NOT thread-safe.

#ifndef SITEMAPSERVICE_PLAINSITEMAPSERVICE_H__
#define SITEMAPSERVICE_PLAINSITEMAPSERVICE_H__

#include <string>

#include "common/sitesetting.h"
#include "sitemapservice/basesitemapservice.h"
#include "sitemapservice/urlfilter.h"
#include "sitemapservice/informer.h"

class PlainSitemapService : public BaseSitemapService {
 public:
  // "sitemap_name" is used to identify what the sitemap is.
  // "writer" is used to write Url records to a file in specific sitemap format.
  PlainSitemapService(const char* sitemap_name, SitemapWriter* writer);

  virtual ~PlainSitemapService();

  // These three methods are three steps to generate a sitemap. They are used
  // in "InternalRun" method. (See method doc for it)
  // The main reason for the existing of these methods is to allow sub-classes
  // to override any of them.
  // These methods are exposed as public only for testing purpose.
  virtual bool Start(const RecordFileStat& stat, const char* host);
  virtual bool ProcessRecord(const VisitingRecord& record);
  virtual bool End();

 protected:
  // Overrides method inherited from base class.
  // It updates url database for the site, reads url record one by one from
  // the database, and feeds the url to Start/ProcessRecord/End methods.
  virtual bool InternalRun();

  // Convert record. All fields of url element is set.
  // If special handling is required, please override this method.
  virtual void ConvertRecord(const VisitingRecord& record, UrlElement* url);

  // Record statistic value.
  // It is used in ConvertRecord method to calculate url priority.
  RecordFileStat recordstat_;

  // Host string used in sitemap file.
  std::string host_;
};


#endif  // SITEMAPSERVICE_PLAINSITEMAPSERVICE_H__

