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


#include "sitemapservice/sitedatamanager.h"

#include <fstream>

#include "common/logger.h"
#include "common/fileutil.h"
#include "sitemapservice/runtimeinfomanager.h"
#include "sitemapservice/newsdatamanager.h"

SiteDataManagerImpl::SiteDataManagerImpl() {
  recordtable_ = NULL;
  hosttable_ = NULL;
  recordmerger_ = NULL;
  siteinfo_ = NULL;
  news_data_manager_ = NULL;
}

SiteDataManagerImpl::~SiteDataManagerImpl() {
  if (recordtable_ != NULL) delete recordtable_;
  if (recordmerger_ != NULL) delete recordmerger_;
  if (hosttable_ != NULL) delete hosttable_;
  if (news_data_manager_ != NULL) delete news_data_manager_;

  for (int i = 0; i < static_cast<int>(replacers_.size()); ++i) {
    delete replacers_[i];
  }
  replacers_.clear();

}

bool SiteDataManagerImpl::Initialize(const SiteSetting& setting) {
  setting_ = setting;

  // Create filter for robots.txt.
  std::string robotstxt_path(setting.physical_path());
  robotstxt_path.append("/robots.txt");
  robotstxt_filter_.Initialize(robotstxt_path.c_str());

  // Initialize query string filter.
  querystring_filter_.Initialize(setting_.included_queryfields());

  // Build URL replacer for this site.
  const std::vector<UrlReplacement>& replacements = setting.url_replacements().items();
  for (int i = 0; i < static_cast<int>(replacements.size()); ++i) {
    UrlReplacer* replacer = new UrlReplacer();
    if (!replacer->Initialize(replacements[i].find(), replacements[i].replace())) {
      delete replacer;
      Logger::Log(EVENT_ERROR, "Failed to initialize replacer. Find:%s, Replace:%s",
                replacements[i].find().c_str(), replacements[i].replace().c_str());
      return false;
    }
    replacers_.push_back(replacer);
  }

 
  // Create record merger and table.
  recordmerger_ = new RecordMerger();
  recordtable_ = new RecordTable(setting.site_id(),
                                 setting.max_url_in_memory());
  
  int64 max_tempfiles_size = setting_.max_url_in_disk();
  max_tempfiles_size *= (sizeof(VisitingRecord) + sizeof(UrlFprint));
  if (!filemanager_.Initialize(setting_.site_id().c_str(), max_tempfiles_size)) {
    Logger::Log(EVENT_ERROR, "%s: Initialize file manager failed.",
              setting_.site_id().c_str());
    return false;
  }

  // Load old records from current_file, if exist.
  std::string currentfile = filemanager_.GetCurrentFile();
  if (recordtable_->Load(currentfile.c_str())) {
    Logger::Log(EVENT_NORMAL, "%s: No old visting record is loaded.",
              setting.site_id().c_str());
  }

  // Load host table from the file.
  hosttable_ = new HostTable(setting.site_id().c_str());
  std::string hostfile = filemanager_.GetHostFile();
  if (!hosttable_->Load(hostfile.c_str())) {
    Logger::Log(EVENT_NORMAL, "%s: No old host table is loaded.",
              setting.site_id().c_str());
  }

  // Retrieve site info.
  siteinfo_ = RuntimeInfoManager::application_info()
    ->site_info(setting.site_id().c_str());
  if (siteinfo_ != NULL) {
    // Get urls in database.
    if (FileUtil::Exists(filemanager_.GetFPFile().c_str())) {
      FileAttribute attr;
      if (FileUtil::GetFileAttribute(filemanager_.GetFPFile().c_str(), &attr)) {
        siteinfo_->set_url_in_database(attr.size / sizeof(UrlFprint));
      } else {
        Logger::Log(EVENT_ERROR, "Failed to retrieve attribute of [%s].",
                  filemanager_.GetFPFile().c_str());
      }
    }

    // Get urls in temp files (approximately).
    siteinfo_->set_url_in_tempfile(filemanager_.GetTempFilesSize()
      / (sizeof(VisitingRecord) + sizeof(UrlFprint)));
    siteinfo_->set_url_in_memory(recordtable_->Size());

    // Try to push the latest host name.
    std::string host_name;
    GetHostName(&host_name);

    time(&last_update_info_);
  }

  news_data_manager_ = new NewsDataManager();
  if (!news_data_manager_->Initialize(this, "news")) {
    Logger::Log(EVENT_ERROR, "Failed to initialize news data manager.");
    return false;
  }

  last_table_save_ = time(NULL);

  last_file_merge_ = time(NULL) - 60 * 60 * 24;

  return true;
}

bool SiteDataManagerImpl::SaveMemoryData(bool flush, bool block) {
  if (!memory_cs_.Enter(block)) {
    Logger::Log(EVENT_IMPORTANT, "Failed to lock memory when saving data.");
    return false;
  }
  AutoLeave memory_autoleave(&memory_cs_);

  // No new record is added since last visit.
  if (recordtable_->Size() == 0) {
    return true;
  }

  // If the records exceeds max size, we must force it to flush.
  if (recordtable_->Size() >= setting_.max_url_in_memory()) {
    flush = true;
  }

  // Save host table.
  std::string hostfile = filemanager_.GetHostFile();
  if (!hosttable_->Save(hostfile.c_str())) {
    Logger::Log(EVENT_ERROR, "%s: Save host table failed when flushing data.",
              setting_.site_id().c_str());
    return false;
  }

  // Save current file.
  std::string currentfile = filemanager_.GetCurrentFile();
  int result = recordtable_->Save(currentfile.c_str());
  if (result == 0) {
    if (flush) {
      result = filemanager_.CompleteCurrentFile() ? 0 : 466453;
      if (result == 0) recordtable_->Clear();
    }
  }
  else {
    Logger::Log(EVENT_ERROR, "%s: Save records failed when flushing data.",
              setting_.site_id().c_str());
  }
  memory_autoleave.LeaveAhead();

  // Update status.
  if (result == 0) {
    if (flush) {
      // A new temp file is generated, we may need to remove old ones.
      if (filemanager_.GetTempFilesSize() > filemanager_.GetMaxTempFilesSize()) {
        disk_cs_.Enter(true);
        filemanager_.CleanUpTempFile();
        disk_cs_.Leave();
      }

     // Update runtime info.
      if (siteinfo_ != NULL && RuntimeInfoManager::Lock(true)) {
        siteinfo_->set_url_in_tempfile(filemanager_.GetTempFilesSize()
          / (sizeof(VisitingRecord) + sizeof(UrlFprint)));
        siteinfo_->set_url_in_memory(0);
        RuntimeInfoManager::Unlock();
      }
    }

    last_table_save_ = time(NULL);
  }

  return result == 0;
}

time_t SiteDataManagerImpl::GetLastUpdate() {
  return last_file_merge_;
}

time_t SiteDataManagerImpl::GetLastSave() {
  return last_table_save_;
}

RecordFileStat SiteDataManagerImpl::GetRecordFileStata() {
  return recordfile_stat_;
}


bool SiteDataManagerImpl::UpdateDatabase() {
  // First, update news entries.
  if (!news_data_manager_->UpdateData()) {
    Logger::Log(EVENT_ERROR, "Failed to update news data.");
    return false;
  }

  // Copy the oboleted set to ensure thread safe.
  if (!memory_cs_.Enter(true)) {
    return false;
  }
  std::set<UrlFprint> obsoleted_copy(obsoleted_);
  obsoleted_.clear();
  memory_cs_.Leave();

  // Update the database.
  if (!disk_cs_.Enter(true)) {
    return false;
  }
  
  time_t cutdown = time(NULL);
  cutdown -= setting_.max_url_life() * 24 * 3600;
  RecordFileStat tmpstat;
  int mergeresult = recordmerger_->Merge(
    &filemanager_, obsoleted_copy,
    setting_.max_url_in_disk(), cutdown, &tmpstat);

  disk_cs_.Leave();

  // Save current status.
  if (mergeresult != 0) {
    Logger::Log(EVENT_ERROR, "%s: record can't be merged.",
              setting_.site_id().c_str());
    return false;
  } else {
    recordfile_stat_ = tmpstat;
    
    // Update runtime info.
    if (siteinfo_ != NULL && RuntimeInfoManager::Lock(true)) {
      siteinfo_->set_url_in_database(tmpstat.GetTotalCount());
      siteinfo_->set_url_in_tempfile(0);
      siteinfo_->set_url_in_memory(0);
      RuntimeInfoManager::Unlock();
    }
  }

  last_file_merge_ = time(NULL);
  return true;
}

bool SiteDataManagerImpl::GetHostName(std::string* host) {
  *host = setting_.host_url().host_url();

  // If host name is not defined in setting, try to determine it automatically.
  if (host->length() == 0) {
    memory_cs_.Enter(true);
    *host = hosttable_->GetBestHost();
    memory_cs_.Leave();
  }

  if (host->length() == 0) {
    Logger::Log(EVENT_ERROR, "%s: Host can't be determined.",
              setting_.site_id().c_str());
    return false;
  } else {
    // Push the newest host name to runtime info.
    if (siteinfo_ != NULL && RuntimeInfoManager::Lock(true)) {
      siteinfo_->set_host_name(*host);
      RuntimeInfoManager::Unlock();
    }
    return true;
  }
}


int SiteDataManagerImpl::ProcessRecord(UrlRecord& record) {
  // Keep it safe!
  record.url[kMaxUrlLength - 1] = '\0';

  // Simply ignore URLs containing invalid chars.
  if (!Url::ValidateUrlChars(record.url) || record.url[0] != '/') {
    DLog(EVENT_CRITICAL, "Url contains invalid chars. [%s].", record.url);
    return 0;
  }

  // Ignore url prevented by robots.txt
  if (!robotstxt_filter_.Accept(record.url)) {
    DLog(EVENT_CRITICAL, "Url ignored by robots.txt. [%s].", record.url);
    return 0;
  }

  // Replace the URLs.
  for (int i = 0; i < static_cast<int>(replacers_.size()); ++i) {
    if (replacers_[i]->Replace(record.url, kMaxUrlLength)) {
      break;
    }
  }

  // Filter the query string.
  querystring_filter_.Filter(record.url);

  // Process the url record.
  if (record.statuscode == 200) {
    bool result = AddRecord(record.host, record.url, record.contentHashCode,
                            record.last_modified, record.last_filewrite);

    // Update runtime info every 60 seconds
    if (result && siteinfo_ != NULL) {
      time_t now = time(NULL);
      if (now <= last_update_info_ + 60)
        if (memory_cs_.Enter(true)) {
          if (RuntimeInfoManager::Lock(true)) {
            siteinfo_->set_url_in_memory(recordtable_->Size());
            RuntimeInfoManager::Unlock();
          }
          last_update_info_ = now;
          memory_cs_.Leave();
        }
    }

    return result ? 0 : 1;
  } else if (record.statuscode == 404 || record.statuscode == 301
             || record.statuscode == 302 || record.statuscode == 307) {
    memory_cs_.Enter(true);
    if (static_cast<int>(obsoleted_.size()) < kMaxObsoletedUrl) {
      obsoleted_.insert(Url::FingerPrint(record.url));
    }
    memory_cs_.Leave();

    return 0;
  } else {
    return 0;
  }
}

bool SiteDataManagerImpl::AddRecord(const char* host, const char *url, 
                                int64 contenthash,
                                const time_t& lastmodified,
                                const time_t& filewrite) {

  memory_cs_.Enter(true);

  bool result = recordtable_->AddRecord(
    url, contenthash, lastmodified, filewrite) == 0;
  hosttable_->VisitHost(host, 1);
  bool is_full = recordtable_->Size() >= setting_.max_url_in_memory();

  memory_cs_.Leave();

  if (is_full) {
    SaveMemoryData(true, false);
  }
  
  return result;
}

bool SiteDataManagerImpl::LockDiskData(bool block) {
  return disk_cs_.Enter(block);
}

void SiteDataManagerImpl::UnlockDiskData() {
  disk_cs_.Leave();
}

RecordfileManager* SiteDataManagerImpl::GetFileManager() {
  return &filemanager_;
}

NewsDataManager* SiteDataManagerImpl::GetNewsDataManager() {
  return news_data_manager_;
}

