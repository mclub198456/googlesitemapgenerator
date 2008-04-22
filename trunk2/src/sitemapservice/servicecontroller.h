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


// This is the controller of all services in the application. It intializes
// url receiver thread, all site data managers and services
// in "Initialize" method. "RunService" should be checked periodically to see
// the status of the controller. And "StopService" is used to stop this
// controller. When this controller is stopped, all services are stopped as
// well.

#ifndef SITEMAPSERVICE_SERVICECONTROLLER_H__
#define SITEMAPSERVICE_SERVICECONTROLLER_H__

#include "common/urlpipe.h"
#include "common/sitesettings.h"
#include "common/criticalsection.h"
#include "sitemapservice/sitedatamanager.h"
#include "sitemapservice/servicerunner.h"
#include "sitemapservice/urlreceivethread.h"
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

  // Run the service.
  // Returns: the waiting time to run service next time (in seconds)
  int RunService();

  // Stop this controller.
  void StopService();

 private:
  // Max number of service runner used in the application.
  static const int kMaxServiceRunner = 5;

  // Change system firewall for this app.
  // This is called in "Initialize" method.
  bool ChangeFirewallSetting(bool remote_admin);

  // Settings for the application.
  SiteSettings        settings_;

  // Data managers for all sites.
  // It maps site-id to site's data manager.
  std::map<std::string, SiteDataManager*> data_managers_;

  // Service pool used to hold all services.
  ServicePool* service_pool_;

  // The service runners used to run service.
  std::list<ServiceRunner*> service_runners_;

  // Thread used for setting http server.
  Thread     setting_thread_;

  // Thread used to receive records from webserver plugin.
  UrlReceiveThread*    receiver_thread_;

  SettingUpdateListener* update_listener_;
};

#endif // SITEMAPSERVICE_SERVICECONTROLLER_H__

