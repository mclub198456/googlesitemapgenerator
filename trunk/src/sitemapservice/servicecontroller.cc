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

#include "sitemapservice/servicecontroller.h"

#include <algorithm>

#include "common/util.h"

#include "sitemapservice/webserver.h"
#include "sitemapservice/pagecontroller.h"
#include "sitemapservice/filescanner.h"
#include "sitemapservice/logparser.h"
#include "sitemapservice/runtimeinfomanager.h"

#include "sitemapservice/websitemapservice.h"
#include "sitemapservice/newssitemapservice.h"
#include "sitemapservice/videositemapservice.h"
#include "sitemapservice/codesearchsitemapservice.h"
#include "sitemapservice/mobilesitemapservice.h"
#include "sitemapservice/blogsearchpingservice.h"
#include "sitemapservice/backupservice.h"

#ifdef WIN32
#include <crtdbg.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <comutil.h>
#else 
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#endif

///////////////////////////////////////////////////////////
// Stuff for SiteSetting thread.
struct ParamSet {
  int param1;
  void* function;
};

static void* SiteSettingThread(void* param) {
  ParamSet* realparam = (ParamSet*) param;
  Util::Log(EVENT_IMPORTANT, "Web server start: port %d", realparam->param1);
  if (!webserver::getInstance()->Start(
          realparam->param1, (webserver::request_func)realparam->function,
          true /* Single thread */ )) {
    Util::Log(EVENT_ERROR, "Web server start failed");
  } 
  Util::Log(EVENT_IMPORTANT, "Web server thread exit");
  return 0;
}

///////////////////////////////////////////////////////////
// Implementation of ServiceController class.

ServiceController::ServiceController() {
  receiver_thread_ = NULL;
  service_pool_ = NULL;
  update_listener_ = NULL;
}

ServiceController::~ServiceController() {
  Util::Log(EVENT_NORMAL, "Start to destory service controller....");

  if (receiver_thread_ != NULL) {
    delete receiver_thread_;
  }

  if (service_pool_ != NULL) {
    delete service_pool_;
  }

  while (service_runners_.size() > 0) {
    delete service_runners_.front();
    service_runners_.pop_front();
  }

  if (update_listener_ != NULL) {
    delete update_listener_;
  }

  // Clear controllers.
  std::map<std::string, SiteDataManager*>::iterator itr = data_managers_.begin();
  while (itr != data_managers_.end()) {
    delete itr->second;
    ++itr;
  }

  data_managers_.clear();
  Util::Log(EVENT_NORMAL, "Service controller destoryed successfully");
}

bool ServiceController::Initialize() {

  // Load setting from IIS (or other web server)
  if (!settings_.LoadWebserverConfig()) {
    return false;
  }

  // Load setting from xml file
  std::string settings_file = SiteSettings::GetDefaultFilePath();
  if ( !settings_.LoadFromFile(settings_file.c_str()) ) {
    return false;
  }

  // Initialize runtime information manager
  if (RuntimeInfoManager::Initialize(settings_)) {
    Util::Log(EVENT_CRITICAL, "Runtime info manager is intialized.");
  } else {
    Util::Log(EVENT_ERROR, "Failed to initialize runtime info manager.");
    return false;
  }

  // Create service pool to contain all the services.
  service_pool_ = new ServicePool();

  // Register all the site services under the controller, 
  // each site service corresponding to one site, and one sub-controller
  std::vector<SiteSetting>::const_iterator itr = settings_.site_settings().begin();
  for (;itr != settings_.site_settings().end(); ++itr) {
    // ignore un-enabled site
    if (itr->enabled() == false) continue;

    SiteDataManagerImpl* controller = new SiteDataManagerImpl();
    data_managers_[itr->site_id()] = controller;

    // Initialize the sub-controller with the site setting
    if (!controller->Initialize(*itr)) {
      Util::Log(EVENT_ERROR, "Failed to initialize for site. [%s].",
                itr->site_id().c_str());
      return false;
    }

    // Initialize backup service, which is a must-have.
    BackupService* bkservice = new BackupService();
    service_pool_->ReturnService(bkservice);
    if (!bkservice->Initialize(controller, *itr, settings_.backup_duration())) {
      Util::Log(EVENT_ERROR, "Failed to initialize backup service. [%s].",
        itr->site_id().c_str());
      return false;
    }

    // Initialize sitemap services.
    if (itr->web_sitemap_setting().enabled()) {
      BaseSitemapService* service = new WebSitemapService();
      service_pool_->ReturnService(service);
      if (!service->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to initialize web sitemap service. [%s]",
                  itr->site_id().c_str());
        return false;
      }
    }

    if (itr->news_sitemap_setting().enabled()) {
      BaseSitemapService* service = new NewsSitemapService();
      service_pool_->ReturnService(service);
      if (!service->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to initialize news sitemap service. [%s]",
                  itr->site_id().c_str());
        return false;
      }
    }

    if (itr->video_sitemap_setting().enabled()) {
      BaseSitemapService* service = new VideoSitemapService();
      service_pool_->ReturnService(service);
      if (!service->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to initialize video sitemap service. [%s]",
                  itr->site_id().c_str());
        return false;
      }
    }

    if (itr->mobile_sitemap_setting().enabled()) {
      BaseSitemapService* service = new MobileSitemapService();
      service_pool_->ReturnService(service);
      if (!service->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to initialize mobile sitemap service. [%s]",
                  itr->site_id().c_str());
        return false;
      }
    }

    if (itr->codesearch_sitemap_setting().enabled()) {
      BaseSitemapService* service = new CodeSearchSitemapService();
      service_pool_->ReturnService(service);
      if (!service->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to initialize code search sitemap service. [%s]",
                  itr->site_id().c_str());
        return false;
      }
    }

    if (itr->blogsearch_ping_setting().enabled()) {
      BlogSearchPingService* service = new BlogSearchPingService();
      service_pool_->ReturnService(service);
      if (!service->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to intialize blogsearch ping service. [%s]",
          itr->site_id().c_str());
        return false;
      }
    }

    // Create file scanner.
    if (itr->file_scanner_setting().enabled()) {
      FileScanner* scanner = new FileScanner();
      if (!scanner->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to initialize file scannder for [%s].",
                  itr->site_id().c_str());
        delete scanner;
      } else {
        service_pool_->ReturnService(scanner);
      }
    }

    // Create log parser.
    if (itr->log_parser_setting().enabled()) {
      LogParser* parser = new LogParser();
      if (!parser->Initialize(controller, *itr)) {
        Util::Log(EVENT_ERROR, "Failed to initialize log parser for [%s].",
                  itr->site_id().c_str());
        delete parser;
      } else {
        service_pool_->ReturnService(parser);
      }
    }
  }

  // Create setting update listener.
  update_listener_ = new SettingUpdateListener();
  if (!update_listener_->Initialize(data_managers_)) {
    Util::Log(EVENT_ERROR, "Failed to intialize setting update listener.");
    return false;
  }
  PageController::getInstance()->setUpdateListener(update_listener_);
  
  // Change firewall setting according to remote_admin flag.
  if (!ChangeFirewallSetting(settings_.remote_admin())) {
    Util::Log(EVENT_ERROR, "Failed to change firewall setting (ignore).");
  }

  // Start setting UI thread.
  ParamSet* param = new ParamSet();
  param->param1 = settings_.setting_port();
  param->function = (void*)PageController::PageControl;

  if (!setting_thread_.Start(&SiteSettingThread, param)) {
     Util::Log(EVENT_ERROR, "Fail to create setting thread.");
    return false;
  } else {
    Util::Log(EVENT_IMPORTANT, "Create setting thread success.");
  }

  // Receive the url access record from IIS filter through pipe.
  // Receiver can only be started after all initialization is done.
  receiver_thread_ = new UrlReceiveThread();
  if (!receiver_thread_->Initialize(settings_, data_managers_) ) {
    Util::Log(EVENT_ERROR, "Receiver thread initialization failed!");
    return false;
  }
  if (!receiver_thread_->Start()) {
    Util::Log(EVENT_ERROR, "Start receiver thread failed!");
    return false;
  }

  // Create service runners' pool.
  for (int i = 0; i < kMaxServiceRunner; ++i) {
    service_runners_.push_back(new ServiceRunner());
    if (!service_runners_.back()->Start(service_pool_)) {
      Util::Log(EVENT_ERROR, "Failed to start service thread [i].", i);
      return false;
    }
  }

  Util::Log(EVENT_CRITICAL, "Service controller started successfully.");
  return true;
}

// Actively schedule services periodically.
int ServiceController::RunService() {

  int next = service_pool_->ScheduleService();

  Util::Log(EVENT_IMPORTANT, "Next service scheduling time in [%d]S.", next);

  return next * 1000;
}

void ServiceController::StopService() {

  Util::Log(EVENT_CRITICAL, "Start to stop service...");
  std::map<std::string, SiteDataManager*>::iterator itr = data_managers_.begin();
  for (; itr != data_managers_.end(); ++itr) {
    // Try to flush all the records in memory,
    // but we don't want to wait.
    itr->second->SaveMemoryData(true, false);
  }

  // Stop all the running services.
  std::list<ServiceRunner*>::iterator runner_itr = service_runners_.begin();
  for (; runner_itr != service_runners_.end(); ++runner_itr) {
    (*runner_itr)->Stop();
  }

  setting_thread_.Stop();

  if (receiver_thread_ != NULL) {
    receiver_thread_->Stop();
  }

  Util::Log(EVENT_CRITICAL, "Serivce is stopped successfully.");
}

#ifdef WIN32
bool ServiceController::ChangeFirewallSetting(bool remote_admin) {
  // Initialize firewall app.
  HRESULT hr = S_OK;
  INetFwMgr* firewall_manager = NULL;
  INetFwPolicy* firewall_policy = NULL;
  INetFwProfile* firewall_profile = NULL;

  BSTR process_image = NULL;
  BSTR name = NULL;
  VARIANT_BOOL var_enabled;
  INetFwAuthorizedApplication* firewall_app = NULL;
  INetFwAuthorizedApplications* firewall_apps = NULL;

  std::string process_name(Util::GetApplicationDir());
  process_name.append("\\SitemapService.exe");
  process_image = SysAllocString(bstr_t(process_name.c_str()));
  name = SysAllocString(L"Google Sitemap Generator");

  // Initialize COM.
  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(hr)) {
    Util::Log(EVENT_ERROR, "Failed to intialize COM.");
    return false;
  }

  do {
    // Create an instance of the firewall settings manager.
    hr = CoCreateInstance(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER,
              __uuidof(INetFwMgr), (void**)&firewall_manager);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "CoCreateInstance failed: 0x%08lx", hr);
      break;
    }

    // Retrieve the local firewall policy.
    hr = firewall_manager->get_LocalPolicy(&firewall_policy);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "get_LocalPolicy failed: 0x%08lx", hr);
      break;
    }

    // Retrieve the firewall profile currently in effect.
    hr = firewall_policy->get_CurrentProfile(&firewall_profile);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "get_CurrentProfile failed: 0x%08lx", hr);
      break;
    }

    // Retrieve the authorized application collection.
    hr = firewall_profile->get_AuthorizedApplications(&firewall_apps);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "get_AuthorizedApplications failed: 0x%08lx", hr);
      break;
    }

    // Attempt to retrieve the authorized application.
    hr = firewall_apps->Item(process_image, &firewall_app);
    if (FAILED(hr) && remote_admin) {
      Util::Log(EVENT_CRITICAL, "Firewall setting is changed for sitemap.");
      // Create an instance of an authorized application.
      hr = CoCreateInstance(__uuidof(NetFwAuthorizedApplication), NULL,
                CLSCTX_INPROC_SERVER, __uuidof(INetFwAuthorizedApplication),
                (void**)&firewall_app);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "CoCreateInstance failed: 0x%08lx", hr);
        break;
      }

      // Set the process image file name.
      hr = firewall_app->put_ProcessImageFileName(process_image);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "put_ProcessImageFileName failed: 0x%08lx", hr);
        break;
      }

      // Set the application friendly name.
      hr = firewall_app->put_Name(name);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "put_Name failed: 0x%08lx", hr);
        break;
      }

      // Add the application to the collection.
      hr = firewall_apps->Add(firewall_app);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "Add failed: 0x%08lx", hr);
        break;
      }
    } else if (hr == S_OK && remote_admin == false) {
      hr = firewall_apps->Remove(process_image);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "Remove failed: 0x%08lx", hr);
        break;
      }
    } else if (FAILED(hr) && remote_admin == false) {
      hr = S_OK;
    }

    // Find out if the authorized application is enabled.
    if (remote_admin == true) {
      hr = firewall_app->get_Enabled(&var_enabled);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "get_Enabled failed: 0x%08lx", hr);
        break;
      }

      if (var_enabled == VARIANT_FALSE) {
        var_enabled = VARIANT_TRUE;
        hr = firewall_app->put_Enabled(var_enabled);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "put_Enabled failed: 0x%08lx", hr);
          break;
        }
      }
    }
  } while (false);

  if (name != NULL) SysFreeString(name);
  if (process_image != NULL)
    SysFreeString(process_image);
  if (firewall_app != NULL) firewall_app->Release();
  if (firewall_apps != NULL) firewall_apps->Release();

  if (firewall_profile != NULL) firewall_profile->Release();
  if (firewall_policy != NULL) firewall_policy->Release();
  if (firewall_manager != NULL) firewall_manager->Release();

  CoUninitialize();
  return hr == S_OK;
}

#elif defined(__linux__)
bool ServiceController::ChangeFirewallSetting(bool remote_admin) {
  // TODO: Open linux firewall.
  return true;
}
#endif
