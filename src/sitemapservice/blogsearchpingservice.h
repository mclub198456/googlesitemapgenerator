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


// This service is a blog search ping service.
// It runs the service periodically. When this service is running, it tries
// to filter the URL, and sent the url address to blog service ping server.
// This class is thread-safe after it is initialized.

#ifndef SITEMAPSERVICE_BLOGSEARCHPINGSERVICE_H__
#define SITEMAPSERVICE_BLOGSEARCHPINGSERVICE_H__

#include "common/thread.h"
#include "common/sitesetting.h"
#include "common/blogsearchpingsetting.h"
#include "sitemapservice/serviceinterface.h"
#include "sitemapservice/urlfilter.h"
#include "sitemapservice/sitedatamanager.h"
#include "sitemapservice/blogsearchpingserviceinfo.h"


class BlogSearchPingService : public ServiceInterface {
 public:
  BlogSearchPingService();
  virtual ~BlogSearchPingService();

  // Initialize the service with setting.
  bool Initialize(SiteDataManager* datamgr, const SiteSetting& setting);

  // Overriden methods from base class.
  virtual int GetWaitTime();
  virtual void Run();
  virtual int GetRunningPeriod();

  // Check whether the visting record is an acceptable blog URL.
  // record whose last change is older than cut_down is not acceptable.
  bool Check(const VisitingRecord& record, time_t cut_down);

  // Ping the server with given url.
  bool Ping(const char* url);

 private:
  // Max number of pings per minutes.
  static const int kMaxPingPerMin;

  int max_ping_per_run_;

  // Setting for this service.
  BlogSearchPingSetting ping_setting_;

  // Url of blog ping service server.
  Url ping_url_;

  // Data manager used to manage all URLs belonging to a site.
  SiteDataManager* data_manager_;

  // Last running time of this service.
  time_t last_run_;

  // Filters used to check the url.
  UrlFilter* includefilter_;
  UrlFilter* excludefilter_;

  // Runtime information assocciated with this service.
  BlogSearchPingServiceInfo* runtime_info_;
};

#endif // SITEMAPSERVICE_BLOGSEARCHPINGSERVICE_H__
