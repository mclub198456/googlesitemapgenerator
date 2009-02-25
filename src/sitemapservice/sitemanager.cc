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

#include "sitemapservice/sitemanager.h"

#include <fstream>

#include "common/port.h"
#include "common/logger.h"
#include "common/fileutil.h"

#include "sitemapservice/websitemapservice.h"
#include "sitemapservice/newsdatamanager.h"
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

  // Handle in memory data and delete data manager.
  if (!data_manager_->GetNewsDataManager()->UpdateData()) {
    Logger::Log(EVENT_ERROR, "Failed to unload data manager.");
  }
  delete data_manager_;
  data_manager_ = NULL;

  /*
  // Remove the site from runtime info manager.
  RuntimeInfoManager::Lock(true);
  RuntimeInfoManager::application_info()->RemoveSiteInfo(setting_.site_id());
  RuntimeInfoManager::Unlock();
  */
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

  // DO NOT UNCOMMENT THIS CODE BLOCK!!!
  // News Sitemap Service is not guranteed to working properly.
  /*
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
  */

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
    return data_manager_->ProcessRecord(record);
  } else {
    return 0;
  }
}


