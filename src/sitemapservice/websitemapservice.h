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


// This service is used to generate web sitemap. It inherits from
// PlainSitemapService and does nothing more than it.
// It takes WebSitemapSetting as its configuration.

#ifndef SITEMAPSERVICE_WEBSITEMAPSERVICE_H__
#define SITEMAPSERVICE_WEBSITEMAPSERVICE_H__

#include "common/sitesettings.h"
#include "common/websitemapsetting.h"
#include "sitemapservice/plainsitemapservice.h"

class WebSitemapService : public PlainSitemapService {
 public:
  WebSitemapService();
  virtual ~WebSitemapService();

  // Overridden method.
  // It extracts web sitemap setting from given site setting, extracts run-time
  // information pointer from RuntimeInfoManager, and passes these two
  // extracted value to base class.
  virtual bool Initialize(SiteDataManager* datamanager,
                          const SiteSetting & setting);

  virtual bool End();

  static bool CleanRobotsTxt();

 private:
  // Update robots.txt if necessary.
  bool UpdateRobotsTxt(const std::string& robotstxt_path);

  WebSitemapSetting    web_sitemap_setting_;
};

#endif  // SITEMAPSERVICE_WEBSITEMAPSERVICE_H__

