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


#include "sitemapservice/urlproviderservice.h"

#include <errno.h>
#include <assert.h>

#include "common/logger.h"
#include "common/util.h"
#include "common/fileutil.h"
#include "common/timesupport.h"
#include "sitemapservice/runtimeinfomanager.h"

UrlProviderService::UrlProviderService() {
  runtime_info_ = NULL;
  url_counter_ = 0;
}

bool UrlProviderService::Initialize(SiteDataManager* data_manager,
                                    const SiteSetting& setting) {
  data_manager_ = data_manager;
  sitesetting_ = setting;
  return true;
}

bool UrlProviderService::Initialize(const std::string suffix,
                                 UrlProviderInfo* runtime_info,
                                 int running_period) {
  // Get the file to store timestamp value.
  timestamp_file_ = sitesetting_.site_id();
  for (int i = 0; i < static_cast<int>(timestamp_file_.length()); ++i) {
    if (!isalnum(timestamp_file_[i])) {
      timestamp_file_[i] = '_';
    }
  }

  std::string stamp_dir(Util::GetApplicationDir());
  stamp_dir.push_back(FileUtil::kPathSeparator);
  stamp_dir.append("run").push_back(FileUtil::kPathSeparator);
  if (!FileUtil::CreateDir(stamp_dir.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to create timestamp dir [%s]. Skip.",
                stamp_dir.c_str());
  }
  timestamp_file_ = stamp_dir + "timestamp_" + timestamp_file_ + suffix;

  runtime_info_ = runtime_info;
  running_period_ = running_period;

  time(&last_run_);
  last_run_ -= running_period;

  return true;
}

void UrlProviderService::SaveLastAccessLimit() {
  FILE* file = fopen(timestamp_file_.c_str(), "w");
  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open timestamp file to write %s. (%d).",
              timestamp_file_.c_str(), errno);
    return;
  }
  if (fwrite(&last_access_limit_, sizeof(time_t), 1, file) == 0) {
    Logger::Log(EVENT_ERROR, "Failed to write time stamp file %s. (%d).",
              timestamp_file_.c_str(), errno);
  }
  Logger::Log(EVENT_IMPORTANT, "Save last access limit [%s].",
              FormatW3CTime(last_access_limit_).c_str());
  fclose(file);
}

bool UrlProviderService::RefreshTimeStamp() {
  FILE* file = fopen(timestamp_file_.c_str(), "r");
  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open timestamp file to read %s. (%d).",
              timestamp_file_.c_str(), errno);
    return false;
  }
  if (fread(&last_access_limit_, sizeof(time_t), 1, file) == 0) {
    Logger::Log(EVENT_ERROR, "Failed to read time stamp file %s. (%d).",
              timestamp_file_.c_str(), errno);
    fclose(file);
    return false;
  }

  Logger::Log(EVENT_CRITICAL, "Timestamp [%s] loaded from [%s].",
    FormatW3CTime(last_access_limit_).c_str(), timestamp_file_.c_str());
  fclose(file);
  return true;
}

int UrlProviderService::GetWaitTime() {
  time_t next = last_run_ + running_period_, now;
  time(&now);
  if (next <= now) {
    return 0;
  } else {
    return static_cast<int>(next - now);
  }
}

void UrlProviderService::ProvideRecord(UrlRecord& record) {
  if (record.last_access <= last_access_limit_) return;

  data_manager_->ProcessRecord(record);
  ++url_counter_;
  if (last_access_limit_tmp_ < record.last_access) {
    last_access_limit_tmp_ = record.last_access;
  }
}

void UrlProviderService::Run() {
  url_counter_ = 0;

  last_access_limit_tmp_ = last_access_limit_;
  bool result = InternalRun();
  if (result) {
    last_access_limit_ = last_access_limit_tmp_;
    SaveLastAccessLimit();
  } else {
    Logger::Log(EVENT_ERROR, "Error occurs while running task. [%s]",
      timestamp_file_.c_str());
  }

  time(&last_run_);

  if (runtime_info_ == NULL) return;

  if (!RuntimeInfoManager::Lock(true)) return;
  runtime_info_->set_success(result);
  runtime_info_->set_urls_count(url_counter_);
  runtime_info_->set_last_update(last_run_);
  RuntimeInfoManager::Unlock();
}

