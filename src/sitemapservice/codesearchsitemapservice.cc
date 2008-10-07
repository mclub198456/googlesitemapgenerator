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


#include "sitemapservice/codesearchsitemapservice.h"

#include "common/logger.h"
#include "sitemapservice/sitemapwriter.h"
#include "sitemapservice/runtimeinfomanager.h"

CodeSearchSitemapService::CodeSearchSitemapService(): 
    PlainSitemapService("Code Search Sitemap", new XmlCodeSearchSitemapWriter()) {
  // does nothing.
}

CodeSearchSitemapService::~CodeSearchSitemapService() {
  // does nothing.
}

bool CodeSearchSitemapService::Initialize(SiteDataManager* datamanager,
                                          const SiteSetting& setting) {
  if (!PlainSitemapService::Initialize(datamanager, setting)) {
    return false;
  }

  codesearch_sitemap_setting_ = setting.codesearch_sitemap_setting();

  // Get the runtime info structure from runtime info tree.
  SitemapServiceInfo* info = NULL;

  SiteInfo* site_info = RuntimeInfoManager::application_info()
    ->site_info(setting.site_id().c_str());
  if (site_info != NULL) {
    info = site_info->codesearch_sitemapservice_info();
  }

  return PlainSitemapService::Initialize(&codesearch_sitemap_setting_, info);
}

bool CodeSearchSitemapService::ProcessRecord(const VisitingRecord& record) {
  // Return immediately for un-acceptable url.
  if (!BaseSitemapService::FilterUrl(record.url())) {
    return true;
  }

  // Try to find matched suffix for given url.
  const CodeSearchSitemapSetting::FileTypeMap& types =
    codesearch_sitemap_setting_.suffix_to_filetype();
  CodeSearchSitemapSetting::FileTypeMap::const_iterator itr = types.begin();
  size_t urllen = record.url_length();

  for (; itr != types.end(); ++itr) {
    size_t suffixlen = itr->first.length();
    if (suffixlen > urllen) continue;

    if (strcmp(record.url() + urllen - suffixlen, itr->first.c_str()) == 0) {
      break;
    }
  }

  // Not a supported file type.
  if (itr == types.end()) {
    Logger::Log(EVENT_NORMAL, "[%s] is not supported by codesearch sitemap.",
              record.url());
    return true;
  }

  // Build UrlElement to add to sitemap file.
  UrlElement url;
  std::string location(host_);
  location.append(record.url());
  url.set_loc(location);

  url.SetAttribute("filetype", itr->second);

  return BaseSitemapService::AddUrl(url);
}

