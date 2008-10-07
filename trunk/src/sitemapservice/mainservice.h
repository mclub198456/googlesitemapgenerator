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


#ifndef SITEMAPSERVICE_MAINSERVICE_H__
#define SITEMAPSERVICE_MAINSERVICE_H__

// Wrap of Sitemap Service. Define the variables and functions
// for installing/uninstalling/starting/debuging SitemapService
class MainService {
 public:
  // Installs this process as a Windows Service by registering it in the
  // ServiceControlManager database.
  // Returns 0 if operation is successful, otherwise returns an system
  // defined error code.
  static int InstallService();

  // Call shell command "net.exe" to start or stop this service
  // Returns 0 if operation is successful, otherwise returns an system
  // defined error code.
  static int ControlService(const char *command, const char *service);

  // Unnstalls this process from a Windows Service by unregistering it in the
  // ServiceControlManager database.
  // Returns 0 if operation is successful, otherwise returns an system defined
  // error code.
  static int UninstallService();

  // Update the setting file based on IIS configuration. Create the setting file
  // if it does not exist.
  // If save_all is false, attributes that have same value as global setting or 
  // default values will not be saved.
  // If save_all is true, all attributes will be saved even if it's same as 
  // global setting or default values.
  static int UpdateSetting();

  // Setups service dispatch function, and then waits until ServiceMain 
  // finished. It should be called by tmain when no command line parameter is 
  // used. In this case, tmain is launched by ServiceControlManager directly.
  static int RunService();

  // Does service-specific work here. It will be called by ServiceControlManager
  // after initilization is done, or called directly by wmain if "debug" command
  // line parameter is used.
  static void RunSitemapService();

  // Open browser for configuration.
  static int StartConfig();

  // Set permission for program files.
  static int SetPermission();

  // Reload setting for the service.
  static int ReloadSetting();

 private:
  // Does not allow creating new instance of this class
  MainService();

  // Entry point for the service called by ServiceControlManager. After calling
  // generic service initalization code, it wil call DoWork() to do service
  // specific work. 
  // Parameters are ignored
  static void WINAPI ServiceMain(DWORD argument_count,
                                 wchar_t **argument_array);

  // Called by ServiceControlManager whenever a control code is sent to the
  // service using the ControlService function.
  // control_code: control code of service
  static void WINAPI ServiceControlHandler(DWORD control_code);

  // Sets the current service status and reports it to ServiceControlManager.
  // current_status is the current state of service (see SERVICE_STATUS).
  static void ReportServiceStatus(DWORD current_status);

 private:
  // Service reload thread's entry point.
  static DWORD WINAPI ServiceReloadThread(LPVOID param);

  // static string variables to store service name.
  static const wchar_t          *kServiceName;
  static const wchar_t          *kServiceDisplayName;

  // static variables to store service specific data.
  static SERVICE_STATUS         service_status_;
  static SERVICE_STATUS_HANDLE  service_status_handle_;
  static HANDLE                 service_stop_event_handle_;

  static HANDLE                 service_reload_event_;
};

#endif  // SITEMAPSERVICE_MAINSERVICE_H__
