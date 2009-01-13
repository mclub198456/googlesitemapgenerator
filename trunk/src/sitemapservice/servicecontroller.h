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


// This is the controller of all services in the application. It intializes
// url receiver thread, all site data managers and services
// in "Initialize" method. "RunService" should be checked periodically to see
// the status of the controller. And "StopService" is used to stop this
// controller. When this controller is stopped, all services are stopped as
// well.

#ifndef SITEMAPSERVICE_SERVICECONTROLLER_H__
#define SITEMAPSERVICE_SERVICECONTROLLER_H__

#include "common/sitesettings.h"
#include "common/criticalsection.h"
#include "sitemapservice/sitemanager.h"
#include "sitemapservice/servicerunner.h"
#include "sitemapservice/urlreceivethread.h"
#include "sitemapservice/adminconsolethread.h"
#include "sitemapservice/settingupdatelistener.h"

#include <map>
#include <string>

class ServiceController {
 public:
  ServiceController();
  virtual ~ServiceController();

  // Initialize this controller.
  // It creates all the managers, services used in this application.
  virtual bool Initialize();

  // Schedule the service.
  // This method should be called after Initialize method.
  void ScheduleService();

  // Stop this controller.
  void StopService();

  // Load new configuration from file.
  bool ReloadSetting();

  // Change system firewall for this app.
  // This is called in "Initialize" method.
  static bool ChangeFirewallSetting(bool remote_admin);

 private:
  // Set reloading status.
  bool SetReloadingStatus(bool status, bool result);

  // Max number of service runner used in the application.
  static const int kMaxServiceRunner = 5;

  // Settings for the application.
  SiteSettings        settings_;

  // Managers for all sites.
  std::map<std::string, SiteManager*> site_managers_;

  // Used to lock site_managers_.
  CriticalSection sites_lock_;

  // Service queue to hold waiting services.
  ServiceQueue* service_queue_;

  // The service runners used to run service.
  std::list<ServiceRunner*> service_runners_;

  // Thread used for setting http server.
  Thread     setting_thread_;

  // Thread used to receive records from webserver plugin.
  UrlReceiveThread*    receiver_thread_;

  AdminConsoleThread* adminconsole_thread_;

  SettingUpdateListener* update_listener_;

  // Indicates the next service schedule time.
  ThreadSafeVar<time_t> next_schedule_;
};

#endif // SITEMAPSERVICE_SERVICECONTROLLER_H__

