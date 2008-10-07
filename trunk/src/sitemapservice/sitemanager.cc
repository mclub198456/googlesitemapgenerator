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

#include "sitemapservice/sitemanager.h"

#include <fstream>

#include "common/port.h"
#include "common/logger.h"
#include "common/fileutil.h"

#include "sitemapservice/websitemapservice.h"
#include "sitemapservice/newssitemapservice.h"
#include "sitemapservice/videositemapservice.h"
#include "sitemapservice/codesearchsitemapservice.h"
#include "sitemapservice/mobilesitemapservice.h"
#include "sitemapservice/blogsearchpingservice.h"
#include "sitemapservice/backupservice.h"

#include "sitemapservice/filescanner.h"
#include "sitemapservice/logparser.h"

#include "sitemapservice/runtimeinfomanager.h"


SiteManager::SiteManager() {
  data_manager_ = NULL;
  service_queue_ = NULL;
}

SiteManager::~SiteManager() {
  if (data_manager_ != NULL) {
    delete data_manager_;
  }

  while (services_.size() > 0) {
    delete services_.front();
    services_.pop_front();
  }

  for (int i = 0; i < static_cast<int>(replacers_.size()); ++i) {
    delete replacers_[i];
  }
  replacers_.clear();
}

bool SiteManager::Initialize(const std::string& site_id,
                             ServiceQueue *service_queue) {
  site_id_ = site_id;
  service_queue_ = service_queue;
  return true;
}

void SiteManager::Unload() {
  Logger::Log(EVENT_CRITICAL, "Unload setting for site [%s].", site_id_.c_str());

  lock_.Enter(true);
  AutoLeave leave_lock(&lock_);

  // Remove services in waiting list.
  service_queue_->RemoveWaitingServices(services_);

  // Check whether there is any running service.
  while (true) {
    bool in_queue = false;
    std::list<ServiceInterface*>::iterator itr = services_.begin();
    for (; itr != services_.end(); ++itr) {
      if (service_queue_->Contains(*itr)) {
        in_queue = true;
        break;
      }
    }

    if (in_queue) {
      Sleep(1000);
    } else {
      break;
    }
  }

  // Delete all services instance.
  while (services_.size() > 0) {
    delete services_.back();
    services_.pop_back();
  }

  // Save data and delete data manager.
  data_manager_->SaveMemoryData(true, true);
  delete data_manager_;
  data_manager_ = NULL;

  // Delete replacers.
  for (int i = 0; i < static_cast<int>(replacers_.size()); ++i) {
    delete replacers_[i];
  }
  replacers_.clear();

  // Remove robotstxt_filter_. (not necessary)
  // Remove querystring_filter_. (not necessary)

  RuntimeInfoManager::Lock(true);
  RuntimeInfoManager::application_info()->RemoveSiteInfo(setting_.site_id());
  RuntimeInfoManager::Unlock();
}

bool SiteManager::Load(const SiteSetting& setting) {
  Logger::Log(EVENT_CRITICAL, "Load setting for site [%s].", site_id_.c_str());

  lock_.Enter(true);
  AutoLeave leave_lock(&lock_);

  setting_ = setting;
  if (setting_.enabled() == false) {
    return false;
  }

  // Add site to runtime info manager.
  RuntimeInfoManager::Lock(true);
  RuntimeInfoManager::application_info()->AddSiteInfo(setting_.site_id());
  RuntimeInfoManager::Unlock();

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

  // Create and initialize data_manager_.
  data_manager_ = new SiteDataManagerImpl();
  if (!data_manager_->Initialize(setting_)) {
    Logger::Log(EVENT_ERROR, "Failed to initialize for site. [%s].",
              setting_.site_id().c_str());
    return false;
  }

  // Initialize backup service, which is a must-have.
  BackupService* bkservice = new BackupService();
  if (!bkservice->Initialize(data_manager_, setting_)) {
    Logger::Log(EVENT_ERROR, "Failed to initialize backup service. [%s].",
      setting_.site_id().c_str());
      delete bkservice;
    } else {
      services_.push_back(bkservice);
  }

  // Initialize sitemap services.
  if (setting_.web_sitemap_setting().enabled()) {
    BaseSitemapService* service = new WebSitemapService();
    if (!service->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize web sitemap service. [%s]",
                setting_.site_id().c_str());
      delete service;
    } else {
      services_.push_back(service);
    }
  }

  if (setting_.news_sitemap_setting().enabled()) {
    BaseSitemapService* service = new NewsSitemapService();
    if (!service->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize news sitemap service. [%s]",
                setting_.site_id().c_str());
      delete service;
    } else {
      services_.push_back(service);
    }
  }

  if (setting_.video_sitemap_setting().enabled()) {
    BaseSitemapService* service = new VideoSitemapService();
    if (!service->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize video sitemap service. [%s]",
                setting_.site_id().c_str());
      delete service;
    } else {
      services_.push_back(service);
    }
  }

  if (setting_.mobile_sitemap_setting().enabled()) {
    BaseSitemapService* service = new MobileSitemapService();
    if (!service->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize mobile sitemap service. [%s]",
                setting_.site_id().c_str());
      delete service;
    } else {
      services_.push_back(service);
    }
  }

  if (setting_.codesearch_sitemap_setting().enabled()) {
    BaseSitemapService* service = new CodeSearchSitemapService();
    if (!service->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize code search sitemap service. [%s]",
                setting_.site_id().c_str());
      delete service;
    } else {
      services_.push_back(service);
    }
  }

  if (setting_.blogsearch_ping_setting().enabled()) {
    BlogSearchPingService* service = new BlogSearchPingService();
    if (!service->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to intialize blogsearch ping service. [%s]",
        setting_.site_id().c_str());
      delete service;
    } else {
      services_.push_back(service);
    }
  }

  // Create file scanner.
  if (setting_.file_scanner_setting().enabled()) {
    FileScanner* scanner = new FileScanner();
    if (!scanner->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize file scannder for [%s].",
                setting_.site_id().c_str());
      delete scanner;
    } else {
      services_.push_back(scanner);
    }
  }

  // Create log parser.
  if (setting_.log_parser_setting().enabled()) {
    LogParser* parser = new LogParser();
    if (!parser->Initialize(data_manager_, setting_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize log parser for [%s].",
                setting_.site_id().c_str());
      delete parser;
    } else {
      services_.push_back(parser);
    }
  }

  // Try to update robots.txt
   std::string filename(setting.web_sitemap_setting().file_name());
   if (setting.web_sitemap_setting().compress()) {
     filename.append(".gz");
   }
   UpdateRobotsTxt(setting.web_sitemap_setting().included_in_robots_txt(),
                   filename.c_str());

  return true;
}

int SiteManager::ScheduleService() {
  if (lock_.Enter(false) == false) {
    return 60;
  }
  AutoLeave lock_autoleave(&lock_);

  int next_waittime = 7 * 24 * 3600;  // at most one week.
  std::list<ServiceInterface*>::iterator itr = services_.begin();
  for (; itr != services_.end(); ++itr) {
    if (service_queue_->Contains(*itr)) {
      next_waittime = std::min<int>(next_waittime, (*itr)->GetRunningPeriod());
    } else {
      int t = (*itr)->GetWaitTime();
      if (t <= 0) {
        t = (*itr)->GetRunningPeriod();
        service_queue_->PushService(*itr);
      }
      next_waittime = std::min<int>(next_waittime, t);
    }
  }

  return next_waittime;
}

int SiteManager::ProcessRecord(UrlRecord& record) {
  if (data_manager_ != NULL) {
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

    return data_manager_->ProcessRecord(record);
  } else {
    return 0;
  }
}

// This method may harm robots.txt, please take care.
bool SiteManager::UpdateRobotsTxt(bool include_sitemap, const char* sitemap) {
  if (data_manager_ == NULL) return false;

  // Create a temporary file to store new robots.txt.
  std::string temp_file;
  if (!FileUtil::MakeTemp(&temp_file)) {
    Logger::Log(EVENT_ERROR, "Failed to create temp file for robots.txt.");
    return false;
  }

  // Construct old robots.txt name and a temp file name.
  std::string robotstxt_path(setting_.physical_path());
  robotstxt_path.append("/robots.txt");

  bool robots_exist = FileUtil::Exists(robotstxt_path.c_str());

  // Copy lines in old robots.txt into a temp file.
  bool old_flag = false;
  std::string old_name;
  std::ofstream fout(temp_file.c_str());
  std::ifstream fin(robotstxt_path.c_str());

  const char* kAddFlag = "# Added by Google Sitemap Generator";
  if (robots_exist) {
    std::string line;
    while (getline(fin, line) != NULL && fout.good()) {
      std::string::size_type pos = line.find(kAddFlag);
      if (pos != std::string::npos) {
        line.erase(pos);
        pos = line.find_last_not_of(" \t");
        if (pos != std::string::npos) {
          line.erase(pos + 1);
        }

        pos = line.find_last_of("/");
        if (pos != std::string::npos) {
          line.erase(line.begin(), line.begin() + pos + 1);
        }

        old_name = line;
        old_flag = true;
      } else {
        fout << line << std::endl;
      }
    }
  }

  // Same value as old one.
  if (old_flag == include_sitemap) {
    if (old_flag == false || old_name == std::string(sitemap)) {
      fin.close();
      fout.close();
      FileUtil::DeleteFile(temp_file.c_str());
      return true;
    }
  }

  // Check the copy result.
  bool result = false;
  do {
    if (robots_exist) {
      if (fin.eof()) {
        fin.close();
      } else {
        Logger::Log(EVENT_ERROR, "Failed to read from old robots.txt.");
        break;
      }
    }

    if (include_sitemap == true) {
      std::string sitemap_url;
      if (!data_manager_->GetHostName(&sitemap_url)) {
        Logger::Log(EVENT_ERROR, "Failed to get host name to update robots.txt");
        break;
      }
      sitemap_url.append("/").append(sitemap);
      fout << "Sitemap: " << sitemap_url << " " << kAddFlag << std::endl;
    }

    result = true;
  } while (false);

  if (fout.fail()) {
    Logger::Log(EVENT_ERROR, "Failed to write temp robots.txt.");
    FileUtil::DeleteFile(temp_file.c_str());
    result = false;
  }
  fout.close();

  if (result && (include_sitemap || old_flag)) {
    if (robots_exist) {
      // Make a backup for robots.txt.
      std::string backup(robotstxt_path);
      backup.append("_sitemap_backup");
      if (!FileUtil::CopyFile(robotstxt_path.c_str(), backup.c_str())) {
        Logger::Log(EVENT_ERROR, "Failed to backup robots.txt.");
        FileUtil::DeleteFile(temp_file.c_str());
        return false;
      }
    }

    if (!FileUtil::MoveFile(temp_file.c_str(), robotstxt_path.c_str())) {
      Logger::Log(EVENT_ERROR, "Failed to create new robots.txt.");
      FileUtil::DeleteFile(temp_file.c_str());
      return false;
    }
  }

  // Remove file if possible.
  FileUtil::DeleteFile(temp_file.c_str());
  return true;
}
