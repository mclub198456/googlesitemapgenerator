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


// This service is used to generate code-search sitemap. It inherits from
// PlainSitemapService and provides new "ProcessRecord" method, because
// code-search sitemap requires different XML elements.
// It takes CodeSearchSitemapSetting as its configuration.

#ifndef SITEMAPSERVICE_CODESEARCHSITEMAPSERVICE_H__
#define SITEMAPSERVICE_CODESEARCHSITEMAPSERVICE_H__

#include "common/codesearchsitemapsetting.h"
#include "sitemapservice/plainsitemapservice.h"

class CodeSearchSitemapService : public PlainSitemapService {
 public:
  CodeSearchSitemapService();
  virtual ~CodeSearchSitemapService();

  // Overridden method.
  // It extracts code-search sitemap setting from given site setting, extracts
  // run-time information pointer from RuntimeInfoManager, and passes these two
  // extracted values to base class.
  virtual bool Initialize(SiteDataManager* datamanager,
                          const SiteSetting & setting);

  // Override.
  // Determine programming language type according to url suffix.
  // Url with un-supported suffix is ignored.
  virtual bool ProcessRecord(const VisitingRecord& record);

 private:
  CodeSearchSitemapSetting    codesearch_sitemap_setting_;
};

#endif  // SITEMAPSERVICE_CODESEARCHSITEMAPSERVICE_H__

