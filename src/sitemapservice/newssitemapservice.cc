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


#include "sitemapservice/newssitemapservice.h"

#include <errno.h>
#include "common/logger.h"
#include "common/util.h"
#include "common/fileutil.h"
#include "common/timesupport.h"
#include "sitemapservice/sitemapwriter.h"
#include "sitemapservice/runtimeinfomanager.h"

const std::string NewsSitemapService::kDataDir = "news_sitemap";


NewsSitemapService::NewsSitemapService(): 
    BaseSitemapService(new XmlNewsSitemapWriter(), "NewsSitemap") {
  newsdata_manager_ = NULL;
}

NewsSitemapService::~NewsSitemapService() {
  if (newsdata_manager_ != NULL) delete newsdata_manager_;
}


bool NewsSitemapService::Initialize(SiteDataManager* datamanager,
                                    const SiteSetting& setting) {

  if (!BaseSitemapService::Initialize(datamanager, setting)) {
    return false;
  }

  news_sitemap_setting_ = setting.news_sitemap_setting();

  // Get the runtime info structure from runtime info tree.
  SitemapServiceInfo* info = NULL;
  SiteInfo* site_info = RuntimeInfoManager::application_info()
    ->site_info(setting.site_id().c_str());
  if (site_info != NULL) {
    info = site_info->news_sitemapservice_info();
  }

  newsdata_manager_ = new NewsDataManager();
  if (!newsdata_manager_->Initialize(datamanager, "news_sitemap")) {
    Logger::Log(EVENT_ERROR, "Failed to initialize news data manager [%s].",
      setting.site_id().c_str());
    return false;
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

  FileAttribute file_attr;
  std::string unescaped_url;
  std::string url_path;
  while (records_.size() != 0) {
    records.push_back(records_.top());
    records_.pop();

    // We don't want query string appears after path.
    const char* question_mark = records.back().url();
    while (*question_mark != '?' && *question_mark != '\0') {
      ++question_mark;
    }

    // Try to un-escape the url (removing %)
    unescaped_url.clear();
    int len = static_cast<int>(question_mark - records.back().url());
    if (!Url::UnescapeUrlComponent(records.back().url(), len, &unescaped_url)) {
      records.pop_back();
      continue;
    }

    // Convert utf-8 path to system coding page.
    if (!Util::UTF8ToSystem(unescaped_url.c_str(), &url_path)) {
      records.pop_back();
      continue;
    }

    // Recalculate the last_modified time.
    // TODO: last write time is already retrieved in filter
    std::string filepath(sitesetting_.physical_path());
    filepath.append("/").append(url_path);
    if (FileUtil::GetFileAttribute(filepath.c_str(), &file_attr)) {
      if (file_attr.last_modified < records.back().last_change) {
        records.back().last_change = file_attr.last_modified;
      }
    }

    // If too old, kick it out.
    if (GetPublicationDate(records.back()) < cutdown_) {
      records.pop_back();
    }
  }

  // Add the old news to the list.
  std::string old_news(newsdata_manager_->GetDirectory());
  old_news.append("old_news");
  if (FileUtil::Exists(old_news.c_str())) {
    VisitingRecord record;
    RecordFileReader* reader = RecordFileIOFactory::CreateReader(old_news);
    if (reader != NULL) {
      while (reader->Read(&record) == 0) {
        if (GetPublicationDate(record) >= cutdown_) {
          records.push_back(record);
        }
      }
      delete reader;
    } else {
      Logger::Log(EVENT_IMPORTANT, "Failed to create reader for old news [%s].",
                old_news.c_str());
    }
  } else {
    Logger::Log(EVENT_IMPORTANT, "No old news file exist [%s].", old_news.c_str());
  }

  // Sort according to their publication time.
  // Only the newest ones are desired.
  std::sort(records.begin(), records.end(), RecordComparer());


  // Actually generate the news sitemap, and write to back up at the same time.
  BaseSitemapService::StartGenerating(host_.c_str());

  RecordFileWriter* writer = RecordFileIOFactory::CreateWriter(old_news);
  if (writer == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to create writer for news record to write [%s].",
              old_news.c_str());
  }
  int maxsize = std::min<int>(news_sitemap_setting_.max_file_url_number(),
                              static_cast<int>(records.size()));
  for (int i = 0; i < maxsize; ++i) {
    writer->Write(records[i]);
    UrlElement url = ConvertRecord(records[i]);
    BaseSitemapService::AddUrl(url);
  }
  delete writer;

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

