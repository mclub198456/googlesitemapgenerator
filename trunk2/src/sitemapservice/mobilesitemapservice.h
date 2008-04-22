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


// This service is used to generate mobile sitemap. It inherits from
// PlainSitemapService and does nothing more than it.
// It takes MobileSitemapSetting as its configuration.

#ifndef SITEMAPSERVICE_MOBILESITEMAPSERVICE_H__
#define SITEMAPSERVICE_MOBILESITEMAPSERVICE_H__

#include "common/mobilesitemapsetting.h"
#include "sitemapservice/plainsitemapservice.h"

class MobileSitemapService : public PlainSitemapService {
 public:
  MobileSitemapService();
  virtual ~MobileSitemapService();

  // Overridden method.
  // It extracts mobile sitemap setting from given site setting, extracts
  // run-time information pointer from RuntimeInfoManager, and passes these two
  // extracted value to base class.
  virtual bool Initialize(SiteDataManager* datamgr,
                          const SiteSetting & setting);

 private:
  MobileSitemapSetting    mobile_sitemap_setting_;
};

#endif  // SITEMAPSERVICE_VIDEOSITEMAPSERVICE_H__

