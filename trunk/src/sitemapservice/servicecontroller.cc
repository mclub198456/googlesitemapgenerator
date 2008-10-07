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

#include "common/logger.h"
#include "common/util.h"

#include "sitemapservice/pagecontroller.h"
#include "sitemapservice/runtimeinfomanager.h"
#include "sitemapservice/backupservice.h"
#include "sitemapservice/httpsettingmanager.h"

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
// Implementation of ServiceController class.

ServiceController::ServiceController() {
  receiver_thread_ = NULL;
  adminconsole_thread_ = NULL;
  service_queue_ = NULL;
  update_listener_ = NULL;
}

ServiceController::~ServiceController() {
  Logger::Log(EVENT_NORMAL, "Start to destory service controller....");

  if (receiver_thread_ != NULL) {
    delete receiver_thread_;
  }

  if (adminconsole_thread_ != NULL) {
    delete adminconsole_thread_;
  }

  if (service_queue_ != NULL) {
    delete service_queue_;
  }

  while (service_runners_.size() > 0) {
    delete service_runners_.front();
    service_runners_.pop_front();
  }

  if (update_listener_ != NULL) {
    delete update_listener_;
  }

  // Clear site managers if exist.
  std::map<std::string, SiteManager*>::iterator itr = site_managers_.begin();
  while (itr != site_managers_.end()) {
    delete itr->second;
    ++itr;
  }
  site_managers_.clear();

  Logger::Log(EVENT_NORMAL, "Service controller destoryed successfully");
}

bool ServiceController::Initialize() {
  // Reset runtime info manager.
  RuntimeInfoManager::Reset();

  // Create service queue to contain all the services.
  service_queue_ = new ServiceQueue();

  // Create setting update listener.
  update_listener_ = new SettingUpdateListener();
  if (!update_listener_->Initialize()) {
    Logger::Log(EVENT_ERROR, "Failed to intialize setting update listener.");
    return false;
  }

  // Initialize page controller.
  PageController* pagecontroller = PageController::instance();
  if (!pagecontroller->Initialize()) {
    Logger::Log(EVENT_ERROR, "Failed to initialize page controller.");
    return false;
  }
  pagecontroller->setting_manager()->SetUpdateListener(update_listener_);

  // Receive the url access record from IIS filter through pipe.
  // Receiver can only be started after all initialization is done.
  receiver_thread_ = new UrlReceiveThread();
  if (!receiver_thread_->Initialize() ) {
    Logger::Log(EVENT_ERROR, "Receiver thread initialization failed!");
    return false;
  }
  if (!receiver_thread_->Start()) {
    Logger::Log(EVENT_ERROR, "Start receiver thread failed!");
    return false;
  }

  adminconsole_thread_ = new AdminConsoleThread();
  if (!adminconsole_thread_->Initialize()) {
    Logger::Log(EVENT_ERROR, "Failed to initialize admin console thread.");
    return false;
  }
  if (!adminconsole_thread_->Start()) {
    Logger::Log(EVENT_ERROR, "Failed to start admin console thread.");
  }

  // Create service runners' pool.
  for (int i = 0; i < kMaxServiceRunner; ++i) {
    service_runners_.push_back(new ServiceRunner());
    if (!service_runners_.back()->Start(service_queue_)) {
      Logger::Log(EVENT_ERROR, "Failed to start service thread [i].", i);
      return false;
    }
  }

  if (!ReloadSetting()) {
    Logger::Log(EVENT_ERROR, "Failed to load setting.");
    return false;
  }

  Logger::Log(EVENT_CRITICAL, "Service controller started successfully.");
  return true;
}

bool ServiceController::SetReloadingStatus(bool status, bool result) {
  RuntimeInfoManager::Lock(true);
  RuntimeInfoManager::application_info()->set_is_reloading(status);
  if (status == false && result == false) {
    RuntimeInfoManager::application_info()->set_last_reloading(-1);
  }
  RuntimeInfoManager::Unlock();

  return result;
}

bool ServiceController::ReloadSetting() {
  Logger::Log(EVENT_CRITICAL, "Start to reload setting...");
  SetReloadingStatus(true, true);

  // Load setting.
  SettingManager* setting_manager = SettingManager::default_instance();
  if (!setting_manager->ReloadWebserverConfig()) {
    return SetReloadingStatus(false, false);
  }

  SiteSettings settings;
  if (!setting_manager->LoadSetting(&settings, true)) {
    return SetReloadingStatus(false, false);
  }

  // Convert encoding of setting values.
  if (!settings.ToSystemEncoding()) {
    Logger::Log(EVENT_ERROR, "Failed to convert setting values to system code.");
    return SetReloadingStatus(false, false);
  }

  // Change last reloading time.
  RuntimeInfoManager::Lock(true);
  RuntimeInfoManager::application_info()->set_last_reloading(time(NULL));
  RuntimeInfoManager::Unlock();

  // Reload global setting.
  Logger::SetLogLevel(settings.logging_level());
  BackupService::SetBackupDuration(settings.backup_duration());

  // Create a temporary setting map for new settings.
  std::map<std::string, SiteSetting> new_settings_map;
  const std::vector<SiteSetting>& new_site_settings = settings.site_settings();
  for (int i = 0; i < (int) new_site_settings.size(); ++i) {
    new_settings_map[new_site_settings[i].site_id()] = new_site_settings[i];
  }

  // Iterate old settings to remove obsoleted site.
  std::vector<SiteSetting> old_site_settings = settings_.site_settings();
  for (int i = 0; i < (int) old_site_settings.size(); ++i) {
    // Site[i] is removed or out-of-date.
    std::string site_id = old_site_settings[i].site_id();
    if (new_settings_map.find(site_id) == new_settings_map.end()
      || old_site_settings[i].Equals(&new_settings_map[site_id]) == false) {
      // Remove it from update listener.
      update_listener_->RemoveSite(site_id);

      // Remove it from receiver thread.
      receiver_thread_->RemoveSite(site_id);

      // Remove the site manager.
      SiteManager* manager = NULL;
      sites_lock_.Enter(true);
      if (site_managers_.find(site_id) != site_managers_.end()) {
        manager = site_managers_[site_id];
        site_managers_.erase(site_id);
      }
      sites_lock_.Leave();
      
      // Unload shouldn't be in critical section.
      if (manager != NULL) {
        manager->Unload();
        delete manager;
      }
    }
  }

  // Iterate new settings to add new site.
  for (int i = 0; i < (int) new_site_settings.size(); ++i) {
    SiteSetting site_setting = new_site_settings[i];

    // Site already exists.
    sites_lock_.Enter(true);
    if (site_managers_.find(site_setting.site_id()) != site_managers_.end()) {
      sites_lock_.Leave();
      continue;
    } else {
      sites_lock_.Leave();
    }

    // Skip disabled site.
    if (site_setting.enabled() == false) {
      continue;
    }

    SiteManager* site_manager = new SiteManager();
    if (!site_manager->Initialize(site_setting.site_id(), service_queue_)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize site [%s]. Skip.",
        site_setting.site_id().c_str());
      delete site_manager;
    } else if (!site_manager->Load(site_setting)) {
      Logger::Log(EVENT_ERROR, "Failed to load setting for site [%s]. Skip.",
        site_setting.site_id().c_str());
      delete site_manager;
    } else {
      // Add the site.
      sites_lock_.Enter(true);
      site_managers_[site_setting.site_id()] = site_manager;
      sites_lock_.Leave();

      // Register the site to listeners.
      update_listener_->AddSite(site_manager);
      if (site_setting.webserver_filter_setting().enabled()) {
        receiver_thread_->AddSite(site_manager);
      }
    }
  }

  // Change firewall setting according to remote_admin flag.
  if (!ChangeFirewallSetting(settings_.remote_admin())) {
    Logger::Log(EVENT_ERROR, "Failed to change firewall setting (ignore).");
  }

  // Force to re-schedule the services ASAP.
  // Here sites_lock_ is used to guide next_schedule,
  // because this value may be changed in ScheduleService method.
  sites_lock_.Enter(true);
  next_schedule_ = time(NULL);
  sites_lock_.Leave();

  // Save the new settings.
  settings_ = settings;
  Logger::Log(EVENT_CRITICAL, "Reloading setting successfully.");
  return SetReloadingStatus(false, true);
}

// Actively schedule services periodically.
void ServiceController::ScheduleService() {
  // Not the actual schedule time.
  if (next_schedule_ > time(NULL)) {
    return;
  }

  // Get lock.
  sites_lock_.Enter(true);
  AutoLeave leave_sites_lock(&sites_lock_);

  int next = 7 * 24 * 3600;
  std::map<std::string, SiteManager*>::iterator itr = site_managers_.begin();
  for (; itr != site_managers_.end(); ++itr) {
    int t = itr->second->ScheduleService();
    if (t < next) next = t;
  }
  next_schedule_ = time(NULL) + next;

  Logger::Log(EVENT_IMPORTANT, "Next service scheduling time in [%d]S.", next);
}

void ServiceController::StopService() {
  Logger::Log(EVENT_CRITICAL, "Start to stop service...");

  // Get lock.
  sites_lock_.Enter(true);
  AutoLeave leave_sites_lock(&sites_lock_);

  // Remove all services from waiting queue.
  service_queue_->RemoveAllWaitingServices();

  // Unload all sites.
  std::map<std::string, SiteManager*>::iterator itr = site_managers_.begin();
  for (; itr != site_managers_.end(); ++itr) {
    // Remove site from reciever thread and update listener.
    receiver_thread_->RemoveSite(itr->first);
    update_listener_->RemoveSite(itr->first);

    // Remove site
    itr->second->Unload();
    delete itr->second;
  }
  site_managers_.clear();

  // Stop all the runners.
  std::list<ServiceRunner*>::iterator runner_itr = service_runners_.begin();
  for (; runner_itr != service_runners_.end(); ++runner_itr) {
    (*runner_itr)->Stop();
  }

  // Stop setting thread.
  setting_thread_.Stop();

  // Stop receiver thread.
  receiver_thread_->Stop();

  // Stop admin console thread.
  adminconsole_thread_->Stop();

  Logger::Log(EVENT_CRITICAL, "Serivce is stopped successfully.");
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
    Logger::Log(EVENT_ERROR, "Failed to intialize COM.");
    return false;
  }

  do {
    // Create an instance of the firewall settings manager.
    hr = CoCreateInstance(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER,
              __uuidof(INetFwMgr), (void**)&firewall_manager);
    if (FAILED(hr)) {
      Logger::Log(EVENT_ERROR, "CoCreateInstance failed: 0x%08lx", hr);
      break;
    }

    // Retrieve the local firewall policy.
    hr = firewall_manager->get_LocalPolicy(&firewall_policy);
    if (FAILED(hr)) {
      Logger::Log(EVENT_ERROR, "get_LocalPolicy failed: 0x%08lx", hr);
      break;
    }

    // Retrieve the firewall profile currently in effect.
    hr = firewall_policy->get_CurrentProfile(&firewall_profile);
    if (FAILED(hr)) {
      Logger::Log(EVENT_ERROR, "get_CurrentProfile failed: 0x%08lx", hr);
      break;
    }

    // Retrieve the authorized application collection.
    hr = firewall_profile->get_AuthorizedApplications(&firewall_apps);
    if (FAILED(hr)) {
      Logger::Log(EVENT_ERROR, "get_AuthorizedApplications failed: 0x%08lx", hr);
      break;
    }

    // Attempt to retrieve the authorized application.
    hr = firewall_apps->Item(process_image, &firewall_app);
    if (FAILED(hr) && remote_admin) {
      Logger::Log(EVENT_CRITICAL, "Firewall setting is changed for sitemap.");
      // Create an instance of an authorized application.
      hr = CoCreateInstance(__uuidof(NetFwAuthorizedApplication), NULL,
                CLSCTX_INPROC_SERVER, __uuidof(INetFwAuthorizedApplication),
                (void**)&firewall_app);
      if (FAILED(hr)) {
        Logger::Log(EVENT_ERROR, "CoCreateInstance failed: 0x%08lx", hr);
        break;
      }

      // Set the process image file name.
      hr = firewall_app->put_ProcessImageFileName(process_image);
      if (FAILED(hr)) {
        Logger::Log(EVENT_ERROR, "put_ProcessImageFileName failed: 0x%08lx", hr);
        break;
      }

      // Set the application friendly name.
      hr = firewall_app->put_Name(name);
      if (FAILED(hr)) {
        Logger::Log(EVENT_ERROR, "put_Name failed: 0x%08lx", hr);
        break;
      }

      // Add the application to the collection.
      hr = firewall_apps->Add(firewall_app);
      if (FAILED(hr)) {
        Logger::Log(EVENT_ERROR, "Add failed: 0x%08lx", hr);
        break;
      }
    } else if (hr == S_OK && remote_admin == false) {
      hr = firewall_apps->Remove(process_image);
      if (FAILED(hr)) {
        Logger::Log(EVENT_ERROR, "Remove failed: 0x%08lx", hr);
        break;
      }
    } else if (FAILED(hr) && remote_admin == false) {
      hr = S_OK;
    }

    // Find out if the authorized application is enabled.
    if (remote_admin == true) {
      hr = firewall_app->get_Enabled(&var_enabled);
      if (FAILED(hr)) {
        Logger::Log(EVENT_ERROR, "get_Enabled failed: 0x%08lx", hr);
        break;
      }

      if (var_enabled == VARIANT_FALSE) {
        var_enabled = VARIANT_TRUE;
        hr = firewall_app->put_Enabled(var_enabled);
        if (FAILED(hr)) {
          Logger::Log(EVENT_ERROR, "put_Enabled failed: 0x%08lx", hr);
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

#elif defined(__linux__) || defined(__unix__)
bool ServiceController::ChangeFirewallSetting(bool remote_admin) {
  // TODO: Open linux firewall.
  return true;
}
#endif
