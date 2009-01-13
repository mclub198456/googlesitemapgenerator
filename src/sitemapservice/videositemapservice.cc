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


#include "sitemapservice/videositemapservice.h"

#include "sitemapservice/sitemapwriter.h"
#include "sitemapservice/runtimeinfomanager.h"

VideoSitemapService::VideoSitemapService(): 
    PlainSitemapService("Video Sitemap", new XmlVideoSitemapWriter()) {
  // does nothing.
}

VideoSitemapService::~VideoSitemapService() {
  // does nothing
}

bool VideoSitemapService::Initialize(SiteDataManager* datamanager,
                                     const SiteSetting& setting) {
  if (!PlainSitemapService::Initialize(datamanager, setting)) {
    return false;
  }
  video_sitemap_setting_ = setting.video_sitemap_setting();

  // Get the runtime info structure from runtime info tree.
  SitemapServiceInfo* info = NULL;
  SiteInfo* site_info = RuntimeInfoManager::application_info()
    ->site_info(setting.site_id().c_str());
  if (site_info != NULL) {
    info = site_info->video_sitemapservice_info();
  }

  return PlainSitemapService::Initialize(&video_sitemap_setting_, info);
}

// Only url location is converted.
void VideoSitemapService::ConvertRecord(const VisitingRecord &record,
                                        UrlElement* url) {
  std::string location(host_);
  location.append(record.url());
  url->set_loc(location);
}
