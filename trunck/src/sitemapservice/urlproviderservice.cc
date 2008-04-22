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


#include "sitemapservice/urlproviderservice.h"

#include <errno.h>
#include <assert.h>

#include "common/util.h"
#include "common/fileutil.h"
#include "sitemapservice/runtimeinfomanager.h"

std::string UrlProviderService::stamp_dir_ = Util::GetApplicationDir();

bool UrlProviderService::ChangeStampDir(const char *dir) {
  if (!FileUtil::CreateDir(dir)) {
    Util::Log(EVENT_ERROR, "Failed to change stamp dir to [%s].",
              dir);
    return false;
  }

  stamp_dir_ = dir;
  return true;
}

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
  timestamp_file_ = stamp_dir_ + "/timestamp" + timestamp_file_ + suffix;

  runtime_info_ = runtime_info;
  running_period_ = running_period;
  return true;
}

void UrlProviderService::UpdateLastRun(time_t last_run) {
  last_run_ = last_run;

  FILE* file = fopen(timestamp_file_.c_str(), "w");
  if (file == NULL) {
    Util::Log(EVENT_ERROR, "Failed to open timestamp file to write %s. (%d).",
              timestamp_file_.c_str(), errno);
    return;
  }
  if (fwrite(&last_run_, sizeof(time_t), 1, file) == 0) {
    Util::Log(EVENT_ERROR, "Failed to write time stamp file %s. (%d).",
              timestamp_file_.c_str(), errno);
  }
  fclose(file);
}

bool UrlProviderService::RefreshTimeStamp() {
  FILE* file = fopen(timestamp_file_.c_str(), "r");
  if (file == NULL) {
    Util::Log(EVENT_ERROR, "Failed to open timestamp file to read %s. (%d).",
              timestamp_file_.c_str(), errno);
    return false;
  }
  if (fread(&last_run_, sizeof(time_t), 1, file) == 0) {
    Util::Log(EVENT_ERROR, "Failed to read time stamp file %s. (%d).",
              timestamp_file_.c_str(), errno);
    fclose(file);
    return false;
  }

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
  data_manager_->ProcessRecord(record);
  ++url_counter_;
}

void UrlProviderService::Run() {
  url_counter_ = 0;

  bool result = InternalRun();

  if (result) {
    UpdateLastRun(time(NULL));
  } else {
    Util::Log(EVENT_ERROR, "Error occurs while running task. [%s]",
      timestamp_file_.c_str());
  }

  if (runtime_info_ == NULL) return;

  if (!RuntimeInfoManager::Lock(true)) return;
  runtime_info_->set_success(result);
  runtime_info_->set_urls_count(url_counter_);
  runtime_info_->set_last_update(time(NULL));
  RuntimeInfoManager::Unlock();
}

