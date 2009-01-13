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


#include "sitemapservice/newssitemapservice.h"

#include <errno.h>
#include "common/logger.h"
#include "common/util.h"
#include "common/fileutil.h"
#include "common/port.h"
#include "common/timesupport.h"
#include "sitemapservice/sitemapwriter.h"
#include "sitemapservice/runtimeinfomanager.h"

const std::string NewsSitemapService::kDataDir = "news_sitemap";


NewsSitemapService::NewsSitemapService(): 
    BaseSitemapService(new XmlNewsSitemapWriter(), "NewsSitemap") {
  newsdata_manager_ = NULL;
}

NewsSitemapService::~NewsSitemapService() {
  // does nothing.
}


bool NewsSitemapService::Initialize(SiteDataManager* datamanager,
                                    const SiteSetting& setting) {

  if (!BaseSitemapService::Initialize(datamanager, setting)) {
    return false;
  }

  news_sitemap_setting_ = setting.news_sitemap_setting();
  newsdata_manager_ = datamanager->GetNewsDataManager();

  // Get the runtime info structure from runtime info tree.
  SitemapServiceInfo* info = NULL;
  SiteInfo* site_info = RuntimeInfoManager::application_info()
    ->site_info(setting.site_id().c_str());
  if (site_info != NULL) {
    info = site_info->news_sitemapservice_info();
  }

  return BaseSitemapService::Initialize(&news_sitemap_setting_, info);
}

// "stat" is ignored.
bool NewsSitemapService::Start(const RecordFileStat& stat, const char* host) {
  // Get cut-down time according to expire duration.
  time(&cutdown_);
  cutdown_ -= news_sitemap_setting_.expire_duration();

  host_.clear();
  host_.append(host);

  return true;
}


bool NewsSitemapService::ProcessRecord(const VisitingRecord& record) {
  time_t pub_date = GetPublicationDate(record);

  // Kick out the expired URL.
  if (pub_date < cutdown_) {
    return true;
  }

  // Only newest (2 * max_file_url_number) URLs are kept.
  // Why we keep (2 * max_file_url_number) instead of (max_file_url_number)?
  // Because in "End" method we'll check last modified time of file on disk.
  // This process may exclude some of the URLs.
  int size = static_cast<int>(records_.size());
  if (size >= 2 * news_sitemap_setting_.max_file_url_number()
    && GetPublicationDate(records_.top()) >= pub_date) {
    return true;
  }

  // Return immediately if it is not an accepted url.
  if (!BaseSitemapService::FilterUrl(record.url())) {
    return true;
  }

  // Add the record, and keep the heap size.
  records_.push(record);
  while (static_cast<int>(records_.size())
    > 2 * news_sitemap_setting_.max_file_url_number()) {
    records_.pop();
  }

  return true;
}

bool NewsSitemapService::End() {
  // Pick up the most newest records set from the twice-large candidate heap.
  std::vector<VisitingRecord> records;

  while (records_.size() != 0) {
    records.push_back(records_.top());
    records_.pop();

    // Use Last Modified time for static page.
    VisitingRecord& record = records.back();
    if (IsStaticPage(record.url())) {
      time_t last_mod = GetFileModifiedTime(record.url());
      if (last_mod > 0 &&
        (record.last_change < 0 || record.last_change > last_mod)) {
        record.last_change = last_mod;
      }
    }
    // If too old, kick it out.
    if (GetPublicationDate(records.back()) < cutdown_) {
      records.pop_back();
    }
  }

  // Sort according to their publication time.
  // Only the newest ones are desired.
  std::sort(records.begin(), records.end(), RecordComparer());

  // Actually generate the news sitemap.
  BaseSitemapService::StartGenerating(host_.c_str());

  int maxsize = std::min<int>(news_sitemap_setting_.max_file_url_number(),
                              static_cast<int>(records.size()));
  for (int i = 0; i < maxsize; ++i) {
    UrlElement url = ConvertRecord(records[i]);
    BaseSitemapService::AddUrl(url);
  }

  return BaseSitemapService::EndGenerating(false, true);
}

// for news sitemap url, two fields are required:
// (1) loc (2) publication_date
UrlElement NewsSitemapService::ConvertRecord(const VisitingRecord &record) {
  UrlElement url;

  // build <loc> element
  url.set_loc(host_ + record.url());

  // These attributes are not use by news sitemap file. But we should set
  // them so code analysis tool will not complain they're not initialized. 
  // TODO: we should have a better solution for this in future. 
  url.set_changefreq(UrlElement::NEVER);
  url.set_priority(1);
  url.set_lastmod(record.last_change);

  // build <news:publication_date> element
  url.SetAttribute("publication_date",
    ::FormatW3CTime(GetPublicationDate(record)));

  return url;
}


bool NewsSitemapService::InternalRun() {
  // For backward comptibility.
  if (data_manager_ == NULL) {
    return false;
  }

  std::string hostname;
  if (!data_manager_->GetHostName(&hostname)) {
    Logger::Log(EVENT_ERROR, "Failed to get host name for news sitemap.");
    return false;
  }

  bool result = false;
  RecordFileReader* reader = NULL;
  do {
    if (!newsdata_manager_->UpdateData()) {
      Logger::Log(EVENT_ERROR, "Failed to get new record for news sitemap.");
      break;
    }

    if (!Start(data_manager_->GetRecordFileStata(), hostname.c_str())) {
      Logger::Log(EVENT_ERROR, "Failed to start generate news sitemap.");
      break;
    }

    reader = RecordFileIOFactory::CreateReader(newsdata_manager_->GetDataFile());
    if (reader == NULL) {
      Logger::Log(EVENT_CRITICAL, "No new record in news URL database.");
    } else {
      VisitingRecord record;
      while (reader->Read(&record) == 0) {
        this->ProcessRecord(record);
      }
      delete reader;
    }

    result = End();
  } while (false);

  if (result) {
    BaseSitemapService::InformSearchEngine();
    return true;
  } else {
    return false;
  }
}

time_t NewsSitemapService::GetFileModifiedTime(const char* url) {
  // We don't want query string appears after path.
  const char* question_mark = url;
  while (*question_mark != '?' && *question_mark != '\0') {
    ++question_mark;
  }

  // Try to un-escape the url (removing %)
  std::string unescaped_url;
  int len = static_cast<int>(question_mark - url);
  if (!Url::UnescapeUrlComponent(url, len, &unescaped_url)) {
    return -1;
  }

  // Convert utf-8 path to system coding page.
  std::string url_path;
  if (!Util::UTF8ToSystem(unescaped_url.c_str(), &url_path)) {
    return -1;
  }

  // Recalculate the last_modified time.
  // TODO: last write time is already retrieved in filter
  std::string filepath(sitesetting_.physical_path());
  filepath.append("/").append(url_path);
  FileAttribute file_attr;
  if (FileUtil::GetFileAttribute(filepath.c_str(), &file_attr)) {
    return file_attr.last_modified;
  }
  return -1;
}

bool NewsSitemapService::IsStaticPage(const char* page) {
  // Suffix for static pages.
  static const char* kStaticTypes[] = {
    "shtml", "html", "htm", NULL
  };

  // Find the '.' for extension.
  const char* p = strlen(page) + page;
  while (p != page && *p != '.') --p;
  if (p == page) return false;
  ++p; // skip '.'

  // Find the matched type.
  const char** type = kStaticTypes;
  for (; *type != NULL; ++type) {
    if (stricmp(*type, p) == 0) {
      break;
    }
  }

  return *type != NULL;
}
