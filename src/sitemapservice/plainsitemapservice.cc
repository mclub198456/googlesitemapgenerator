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


#include "sitemapservice/plainsitemapservice.h"

#include "sitemapservice/sitemapwriter.h"
#include "sitemapservice/urlfilterbuilder.h"
#include "common/logger.h"

PlainSitemapService::PlainSitemapService(const char* sitemap_name,
                                         SitemapWriter* writer)
: BaseSitemapService(writer, sitemap_name) {
  // does nothing
}

PlainSitemapService::~PlainSitemapService() {
  // does nothing.
}

bool PlainSitemapService::InternalRun() {
  // For backward comptibility.
  if (data_manager_ == NULL) {
    return false;
  }

  // Update the database.
  if (!data_manager_->UpdateDatabase()) {
    Logger::Log(EVENT_ERROR, "Failed to update database.");
    return false;
  }

  // Get host name for the site.
  std::string hostname;
  if (!data_manager_->GetHostName(&hostname)) {
    Logger::Log(EVENT_ERROR, "Failed to get host name.");
    return false;
  }

  // Lock data base to read.
  if (!data_manager_->LockDiskData(true)) {
    Logger::Log(EVENT_ERROR, "Failed to lock site data.");
    return false;
  }

  bool result = false;
  RecordFileReader* reader = NULL;
  do {
    // Open data base to read.
    std::string data_base = data_manager_->GetFileManager()->GetBaseFile();
    reader = RecordFileIOFactory::CreateReader(data_base);
    if (reader == NULL) {
      Logger::Log(EVENT_ERROR, "Failed to open data base to read.");
      break;
    }

    // Start to generate sitemap file.
    if (!Start(data_manager_->GetRecordFileStata(), hostname.c_str())) {
      Logger::Log(EVENT_ERROR, "Failed to start generate sitemap.");
      break;
    }

    // Process url one by one.
    VisitingRecord record;
    while (reader->Read(&record) == 0) {
      if (!ProcessRecord(record)) {
        Logger::Log(EVENT_NORMAL, "Failed to process url [%s] for sitemap, ignore.",
                  record.url());
      }
    }

    result = true;
  } while (false);

  // Release resources.
  if (reader != NULL) delete reader;
  data_manager_->UnlockDiskData();

  if (result == true) {
    return End();
  } else {
    return false;
  }
}


bool PlainSitemapService::Start(const RecordFileStat& stat, const char* host) {
  recordstat_ = stat;
  host_ = host;

  BaseSitemapService::StartGenerating(host);
  return true;
}


bool PlainSitemapService::ProcessRecord(const VisitingRecord& record) {
  // Return immediately for un-acceptable url.
  if (!BaseSitemapService::FilterUrl(record.url())) {
    return true;
  }

  // Convert VisitingRecord to UrlElement, and add it to sitemap.
  UrlElement url;
  ConvertRecord(record, &url);
  return BaseSitemapService::AddUrl(url);
}

// End generation and inform search engine.
bool PlainSitemapService::End() {
  if (BaseSitemapService::EndGenerating(true, false)) {
    InformSearchEngine();
    return true;
  } else {
    return false;
  }
}


void PlainSitemapService::ConvertRecord(const VisitingRecord &record, UrlElement* url) {
  // "lastmod" is the same with "last_change".
  url->set_lastmod(record.last_change);

  // <loc> = host_ + record.url.
  std::string location(host_);
  location.append(record.url());  
  url->set_loc(location);

  
  // Calculate <changefreq>.
  if (record.count_change * 2 >= record.count_access
      && record.count_access > 10) {
    url->set_changefreq(UrlElement::ALWAYS);
  } else {
    // "change_ratio" is average change interval.
    double change_ratio =
      static_cast<double>(record.last_access - record.first_appear);
    change_ratio = change_ratio / record.count_change / 3600;

    if (change_ratio <= 1.0) {
      url->set_changefreq(UrlElement::HOURLY);
    } else if (change_ratio <= 24.0) {
      url->set_changefreq(UrlElement::DAILY);
    } else if (change_ratio <= 24.0 * 7) {
      url->set_changefreq(UrlElement::WEEKLY);
    } else if (change_ratio <= 24.0 * 30) {
      url->set_changefreq(UrlElement::MONTHLY);
    } else if (change_ratio <= 24.0 * 365) {
      url->set_changefreq(UrlElement::YEARLY);
    } else {
      url->set_changefreq(UrlElement::NEVER);
    }
  }

  // <priority> is retrieved from RecordStat.
  url->set_priority(recordstat_.GetPriority(record));
}

