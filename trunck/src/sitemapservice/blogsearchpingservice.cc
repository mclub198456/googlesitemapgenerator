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


#include "sitemapservice/blogsearchpingservice.h"

#include "common/util.h"
#include "sitemapservice/urlfilterbuilder.h"
#include "sitemapservice/httpgetter.h"
#include "sitemapservice/recordfileio.h"
#include "sitemapservice/runtimeinfomanager.h"

BlogSearchPingService::BlogSearchPingService() {
  time(&last_run_);

  newsdata_manager_ = NULL;
  includefilter_ = NULL;
  excludefilter_ = NULL;

  runtime_info_ = NULL;
}

BlogSearchPingService::~BlogSearchPingService() {
  if (includefilter_ != NULL) delete includefilter_;
  if (excludefilter_ != NULL) delete excludefilter_;
  if (newsdata_manager_ != NULL) delete newsdata_manager_;
}

bool BlogSearchPingService::Initialize(SiteDataManager* mgr, const SiteSetting& setting) {
  data_manager_ = mgr;
  ping_setting_ = setting.blogsearch_ping_setting();

  // Build url filters.
  if (ping_setting_.blog_url().length() == 0) {
    includefilter_ = UrlFilterBuilder::Build(ping_setting_.included_urls());
    excludefilter_ = UrlFilterBuilder::Build(ping_setting_.excluded_urls());
  }

  ping_url_ = Url(BlogSearchPingSetting::kPingServiceUrl);

  newsdata_manager_ = new NewsDataManager();
  if (!newsdata_manager_->Initialize(mgr, "blogsearch_ping")) {
    Util::Log(EVENT_ERROR, "Failed to initialize new data manager for blog ping service.");
    return false;
  }

  // Get the runtime info structure from runtime info tree.
  SiteInfo* site_info = RuntimeInfoManager::application_info()
    ->site_info(setting.site_id().c_str());
  if (site_info != NULL) {
    runtime_info_ = site_info->blogsearch_pingservice_info();
  }

  return  true;
}

int BlogSearchPingService::GetWaitTime() {
  time_t next = last_run_ + ping_setting_.update_duration();
  time_t now = time(NULL);
  if (next <= now) {
    return 0;
  } else {
    return static_cast<int>(next - now);
  }
}

int BlogSearchPingService::GetRunningPeriod() {
  return ping_setting_.update_duration();
}


void BlogSearchPingService::Run() {
  time(&last_run_);

  std::string hostname;
  if (!data_manager_->GetHostName(&hostname)) {
    Util::Log(EVENT_ERROR, "Failed to get host name for blogsearch ping.");
    return;
  }

  if (!newsdata_manager_->UpdateData()) {
    Util::Log(EVENT_ERROR, "Failed to update new record for blogsearch ping.");
    return;
  }

  RecordFileReader* reader = RecordFileIOFactory::CreateReader();
  if (reader->Open(newsdata_manager_->GetDataFile().c_str()) != 0) {
    Util::Log(EVENT_ERROR, "Failed to open [%s] to read for blogsearch ping.",
      newsdata_manager_->GetDataFile().c_str());
    delete reader;
    return;
  }

  VisitingRecord record;
  UrlFprint fprint;
  while (reader->Read(&fprint, &record) == 0) {
    // Next round is coming, exit this round.
    if (GetWaitTime() <= 0) {
      break;
    }

    if (!Check(record)) continue;

    std::string full_url(hostname);
    full_url.append(record.url);
    bool success = Ping(full_url.c_str());

    // Update runtime info
    if (runtime_info_ != NULL && RuntimeInfoManager::Lock(true)) {
      runtime_info_->set_success(success);
      runtime_info_->set_last_ping(time(NULL));
      runtime_info_->set_last_url(full_url);
      RuntimeInfoManager::Unlock();
    }

    // This requires exact match, so the left URLs are ignored.
    if (ping_setting_.blog_url().length() != 0) {
      break;
    }
  }

  reader->Close();
  delete reader;
}

bool BlogSearchPingService::Check(const VisitingRecord& record) {
  // 1) Check whether the url is acceptable.
  if (ping_setting_.blog_url().length() != 0) {
    Url temp(ping_setting_.blog_url().c_str());
    if (strcmp(record.url, temp.path_url().c_str()) != 0) return false;
  } else {
    int urllen = static_cast<int>(strlen(record.url));
    if (includefilter_ != NULL && !includefilter_->Accept(record.url, urllen)) {
      // not an included url
      return false;
    }
    if (excludefilter_ != NULL && excludefilter_->Accept(record.url, urllen)) {
      // an excluded url
      return false;
    }
  }

  // 2) Check whether the time is acceptable.
  if (record.last_change <= 0) {
    return false;
  } else {
    return record.last_change > last_run_;
  }
}

bool BlogSearchPingService::Ping(const char *url) {
  // Construct ping path like,
  // /ping?name=MyBlog&url=blogspot.com&changesURL=atom.xml
  std::string path(ping_url_.path());
  if (ping_setting_.blog_name().length() != 0) {
    std::string escaped_name;
    Url::EscapeUrlComponent(
      ping_setting_.blog_name().c_str(), &escaped_name);
    path.append("name=").append(escaped_name).append("&");
  }

  std::string escaped_url;
  Url::EscapeUrlComponent(url, &escaped_url);
  path.append("url=").append(escaped_url);


  if (ping_setting_.blog_changes_url().length() != 0) {
    std::string escaped_changesurl;
    Url::EscapeUrlComponent(
      ping_setting_.blog_changes_url().c_str(), &escaped_changesurl);
    path.append("&changesURL=").append(escaped_changesurl);
  }

  // Try to ping
  HttpGetter http_getter;
  if (!http_getter.Get(ping_url_.host().c_str(),
    ping_url_.port(), path.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to get http result from ping.");
    return false;
  }

  if (http_getter.status() != 200) {
    Util::Log(EVENT_ERROR, "Ping service returns status [%d].",
              http_getter.status());
    return false;
  }

  if (http_getter.content().find("Thanks for the ping.")
      == std::string::npos) {
    Util::Log(EVENT_ERROR, "Ping service returns error. [%s].",
              http_getter.content().c_str());
    return false;
  }

  Util::Log(EVENT_NORMAL, "Ping succeeded for [%s].", path.c_str());
  return true;
}

