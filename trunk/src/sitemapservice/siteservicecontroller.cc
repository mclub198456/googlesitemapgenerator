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


#include "sitemapservice/siteservicecontroller.h"
#include "common/util.h"
#include "common/port.h"
#include "sitemapservice/websitemapservice.h"
#include "sitemapservice/newssitemapservice.h"
#include "sitemapservice/videositemapservice.h"
#include "sitemapservice/mobilesitemapservice.h"
#include "sitemapservice/codesearchsitemapservice.h"
#include "sitemapservice/recordfilemanager.h"
#include "sitemapservice/recordfileio.h"

SiteServiceController::SiteServiceController() {
  merger_ = NULL;
  recordtable_ = NULL;
  hosttable_ = NULL;
  time(&last_save_);
}

SiteServiceController::~SiteServiceController() {

  ReleaseResource();
}

void SiteServiceController::ReleaseResource() {
  if (recordtable_ != NULL) {
    delete recordtable_;
    recordtable_ = NULL;
  }

  if (merger_ != NULL) {
    delete merger_;
    merger_ = NULL;
  }

  if (hosttable_ != NULL) {
    delete hosttable_;
  }

  std::vector<ServiceInterface*>::iterator itr = services_.begin();
  while (itr != services_.end()) {
    delete *itr;
    ++itr;
  }
  services_.clear();

  for (int i = 0; i < static_cast<int>(replacers_.size()); ++i) {
    delete replacers_[i];
  }
  replacers_.clear();
}

// set the following fields:
//   site setting
//   record merger
//   record table
//   services for each sitemap (web/news/video/mobile)
//   host table
int SiteServiceController::Initialize(const SiteSetting& setting) {
  setting_ = setting;

  // Create record merger and table.
  merger_ = new RecordMerger();
  recordtable_ = new RecordTable(setting.site_id(),
                                 setting.max_url_in_memory());

  // Create filter for robots.txt.
  std::string robotstxt_path(setting.physical_path());
  robotstxt_path.append("/robots.txt");
  robotstxt_filter_.Initialize(robotstxt_path.c_str());

  // Build URL replacer for this site.
  const std::vector<UrlReplacement>& replacements = setting.url_replacements();
  for (int i = 0; i < static_cast<int>(replacements.size()); ++i) {
    UrlReplacer* replacer = new UrlReplacer();
    if (!replacer->Initialize(replacements[i].find(), replacements[i].replace())) {
      delete replacer;
      Util::Log(EVENT_ERROR, "Failed to initialize replacer. Find:%s, Replace:%s",
                replacements[i].find().c_str(), replacements[i].replace().c_str());
      return false;
    }
    replacers_.push_back(replacer);
  }

  // load old records from current_file, if exist
  RecordfileManager manager;
  if (!manager.Initialize(setting_.site_id().c_str())) {
    Util::Log(EVENT_ERROR, "%s: Initialize file manager failed when starting.",
              setting.site_id().c_str());
    return 1;
  }
  
  std::string currentfile = manager.GetCurrentFile();
  if (recordtable_->Load(currentfile.c_str())) {
    Util::Log(EVENT_NORMAL, "%s: No old record is loaded.",
              setting.site_id().c_str());
  }

  // create common sitemap service
  if (setting.web_sitemap_setting().enabled()) {
    ServiceInterface* service = new WebSitemapService();
    services_.push_back(service);
    int result = service->Initialize(setting);
    if (result != 0) {
      ReleaseResource();
      Util::Log(EVENT_ERROR, "%s: CommonSitemapService can't be initialized.",
                setting.site_id().c_str());
      return result;
    }
  }

  // create news sitemap service
  if (setting.news_sitemap_setting().enabled()) {
    ServiceInterface* service = new NewsSitemapService();
    services_.push_back(service);
    int result = service->Initialize(setting);
    if (result != 0) {
      ReleaseResource();
      Util::Log(EVENT_ERROR, "%s: NewSitemapService can't be initialized.",
                setting.site_id().c_str());
      return result;
    }
  }

  // Create Video sitemap service.
  if (setting.video_sitemap_setting().enabled()) {
    ServiceInterface* service = new VideoSitemapService();
    services_.push_back(service);
    int result = service->Initialize(setting);
    if (result != 0) {
      ReleaseResource();
      Util::Log(EVENT_ERROR, "%s: VideoSitemapService can't be initialized.",
                setting.site_id().c_str());
      return result;
    }
  }

 // Create Mobile sitemap service.
  if (setting.mobile_sitemap_setting().enabled()) {
    ServiceInterface* service = new MobileSitemapService();
    services_.push_back(service);
    int result = service->Initialize(setting);
    if (result != 0) {
      ReleaseResource();
      Util::Log(EVENT_ERROR, "%s: MobileSitemapService can't be initialized.",
                setting.site_id().c_str());
      return result;
    }
  }

  // Create Code Search sitemap service.
  if (setting.codesearch_sitemap_setting().enabled()) {
    ServiceInterface* service = new CodeSearchSitemapService();
    services_.push_back(service);
    int result = service->Initialize(setting);
    if (result != 0) {
      ReleaseResource();
      Util::Log(EVENT_ERROR, "%s: CodeSearchSitemapService can't be initialized.",
                setting.site_id().c_str());
      return result;
    }
  }

  // load host table from the file
  hosttable_ = new HostTable(setting.site_id().c_str());
  std::string hostfile = manager.GetHostFile();
  if (!hosttable_->Load(hostfile.c_str())) {
    Util::Log(EVENT_NORMAL, "%s: Can't load host table.",
              setting.site_id().c_str());
  }
  
  return 0;
}

// called by service controller's OnReceive(), so do not do any heavy work
// add the record to record table, save it if necessary
// access the record table exclusively
int SiteServiceController::ProcessRecord(UrlRecord& record) {
  // Simply ignore URLs containing invalid chars.
  if (!Url::ValidateUrlChars(record.url) || record.url[0] != '/') {
    DLog(EVENT_ERROR, "Url contains valid chars. [%s].", record.url);
    return 0;
  }

  // Ignore url prevented by robots.txt
  if (!robotstxt_filter_.Accept(record.url)) {
    DLog(EVENT_ERROR, "Url ignored by robots.txt. [%s].", record.url);
    return 0;
  }

  // Replace the URLs.
  for (int i = 0; i < static_cast<int>(replacers_.size()); ++i) {
    if (replacers_[i]->Replace(record.url, kMaxUrlLength)) {
      break;
    }
  }

  // Process the url record.
  if (record.statuscode == 200) {
    return AddRecord(record.host, record.url, record.contentHashCode,
                     record.last_modified, record.last_filewrite);
  } else if (record.statuscode == 404) {
    table_criticalsection_.Enter(true);
    if (static_cast<int>(obsoleted_.size()) < setting_.max_url_in_memory()) {
      obsoleted_.insert(Url::FingerPrint(record.url));
    }
    table_criticalsection_.Leave();

    return 0; // always success
  } else {
    return 0; // ignore the other
  }
}
  

int SiteServiceController::AddRecord(const char* host, const char *url, 
                                         int64 contenthash,
                                         const time_t& lastmodified,
                                         const time_t& filewrite) {
  // LockTable(true);
  table_criticalsection_.Enter(true);

  int result = recordtable_->AddRecord(url, contenthash, lastmodified, filewrite);
  hosttable_->VisitHost(host, 1);

  if (result == 0) {
    result = AutomaticallySave(false, false);
  }

  table_criticalsection_.Leave();

  return result;
}

// check the run schedule time, 
// when time is arrived, merge the records, and run the services.
// access the save file exclusively
// access the record table exclusively
int SiteServiceController::RunService() {

  int next_waittime = kMaxWaitTime;

  // Step 1: extract all the services, which are ready to run
  std::vector<ServiceInterface*> available_services;
  std::vector<ServiceInterface*>::iterator itr = services_.begin();

  while (itr != services_.end()) {
    ServiceInterface* service = *itr;

    int waititme = service->GetWaitTime();
    if (waititme <= 0) {
      available_services.push_back(service);

      if (next_waittime > service->GetRunningPeriod()) {
        next_waittime = service->GetRunningPeriod();
      }
    } else {
      if (next_waittime > waititme) {
        next_waittime = waititme;
      }
    }
    ++itr;
  }

  // no service is ready, simply return.
  if (available_services.size() == 0) {
    return next_waittime;
  }

  // Step 2: save all the records, and pick up the most popular hostname.
  std::string besthost = setting_.host_url().host_url();
  table_criticalsection_.Enter(true);  
  if (besthost.length() == 0) {
    // try to figure out the most popular hostname that can represents the site
    besthost = hosttable_->GetBestHost();
  }

  // Is it OK to save the records if we failed to find the best host?
  // It's not a big deal.
  AutomaticallySave(true, true);
  table_criticalsection_.Leave();

  if (besthost.length() == 0) {
    Util::Log(EVENT_ERROR, "%s: Host can't be determined.",
              setting_.site_id().c_str());
    return next_waittime;
  }

  // Step 3: merge the record files.
  RecordFileStat filestat;
  file_criticalsection_.Enter(true);
  time_t cutdown = time(NULL);

  //max_url_life() return the num of days
  cutdown -= setting_.max_url_life() * 24 * 3600;
  int mergeresult = merger_->Merge(setting_.site_id().c_str(),
                                   obsoleted_,
                                   setting_.max_url_in_disk(),
                                   cutdown,
                                   &filestat);
  file_criticalsection_.Leave();

  if (mergeresult != 0) {
    Util::Log(EVENT_ERROR, "%s: record can't be merged.",
              setting_.site_id().c_str());
    return next_waittime;
  }

  // clear obsoleted table, if merge succeed.
  obsoleted_.clear();

  // Step 4: Actually run the service
  RunRegisteredServices(available_services, filestat, besthost.c_str());

  return next_waittime;
}

// run the on-time services
int SiteServiceController::RunRegisteredServices(std::vector<ServiceInterface*>& available_services,
                                                 const RecordFileStat& stat, const char* host) {
  // load base file
  RecordfileManager filemanager;
  filemanager.Initialize(setting_.site_id().c_str());

  std::string basefile = filemanager.GetBaseFile();

  RecordFileReader* reader = RecordFileIOFactory::CreateReader();
  int result = reader->Open(basefile.c_str());
  if (result != 0) {
    delete reader;
    return result;
  }

  // start services before run them.
  for (int i = 0, n = static_cast<int>(available_services.size()); i < n; ++i) {
    available_services[i]->Start(stat, host);
  }

  // run the services -- process all the records
  VisitingRecord record;
  UrlFprint fprint;
  int counter = 0;
  while (reader->Read(&fprint, &record) == 0) {

    // TODO: Improve the code here.
    if ((counter & ((1 << 13) - 1)) == 0) Sleep(1);
    ++counter;

    for (int i = 0, n = static_cast<int>(available_services.size()); i < n; ++i) {
      available_services[i]->ProcessRecord(record);
    }
  }

  // finish services' run
  for (int i = 0, n = static_cast<int>(available_services.size()); i < n; ++i) {
    available_services[i]->End();
  }

  delete reader;
  return 0;
}

// save the record table to current file.
// save host table to host file.
// save condition: 
//   if forceflush, save it
//   if full, save it
//   if reach the save time, save it
// access the file system (current file, historical files, base file) exclusively.
// if forceflush, save current file to historical file.
// must get record table mutex when call this function.
int SiteServiceController::AutomaticallySave(bool forceflush, bool waitWhenBlocking) {
  // if the records exceeds max size, we must force it to flush!
  if (recordtable_->Size() >= setting_.max_url_in_memory()) {
    forceflush = true;
  }

  time_t current = time(NULL);
  if (last_save_ + setting_.backup_duration() > current
    && forceflush == false) {
    return 0;
  }

  // no more new record is added since last visit
  if (recordtable_->Size() == 0) {
    return 0;
  }

  // access the file
  if (!file_criticalsection_.Enter(waitWhenBlocking)) {
    return 0;
  }

  RecordfileManager filemanager;
  if (!filemanager.Initialize(setting_.site_id().c_str())) {
    Util::Log(EVENT_ERROR, "%s: Initialize file manager failed when saving.",
              setting_.site_id().c_str());
    file_criticalsection_.Leave();
    return 1;
  }  

  // save host table
  std::string hostfile = filemanager.GetHostFile();
  if (!hosttable_->Save(hostfile.c_str())) {
    Util::Log(EVENT_ERROR, "%s: Save host table failed when flushing data.",
              setting_.site_id().c_str());
  }

  // save current file
  std::string currentfile = filemanager.GetCurrentFile();
  int result = recordtable_->Save(currentfile.c_str());
  if (result == 0) {
    if (forceflush) {
      result = filemanager.CompleteCurrentFile();
      if (result == 0) recordtable_->Clear();
    }
    last_save_ = current;// current time
  }
  else {
    Util::Log(EVENT_ERROR, "%s: Save records failed when flushing data.",
              setting_.site_id().c_str());
  }

  file_criticalsection_.Leave();

  return result;
}
