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


#include <windows.h>
#include <stdio.h>

#include "common/util.h"
#include "sitemapservice/mainservice.h"
#include "sitemapservice/servicecontroller.h"

using std::wstring;
using std::string;

// Initialize static member variables
SERVICE_STATUS          MainService::service_status_;
SERVICE_STATUS_HANDLE   MainService::service_status_handle_ = NULL;
HANDLE                  MainService::service_stop_event_handle_ = NULL;
const wchar_t *MainService::kServiceName = L"GoogleSitemapGenerator";
const wchar_t *MainService::kServiceDisplayName = L"Google Sitemap Generator";

int MainService::InstallService() {
  SC_HANDLE scm_handle;
  SC_HANDLE service_handler;
  wchar_t   module_file_name[MAX_PATH];

  if(!GetModuleFileName(NULL, module_file_name, MAX_PATH)) {
    DWORD error_code = GetLastError();
    Util::Log(EVENT_ERROR, "GetModuleFileName falied. Error Code: %d.", error_code );
    return error_code;
  }

  // Get a handle to the SCM database.
  scm_handle = OpenSCManager(NULL,                    // local computer
                             NULL,                    // service database
                             SC_MANAGER_ALL_ACCESS);  // full access rights
  if (NULL == scm_handle) {
    DWORD error_code = GetLastError();;
    Util::Log(EVENT_ERROR, "OpenSCManager failed. Error Code: %d.", error_code);
    return error_code;
  }

  // Create the service.
  service_handler = CreateService(
      scm_handle,                 // SCM database
      kServiceName,               // name of service
      kServiceDisplayName,        // service name to display
      SERVICE_ALL_ACCESS,         // desired access
      SERVICE_WIN32_OWN_PROCESS,  // service type
      SERVICE_AUTO_START,         // start type
      SERVICE_ERROR_NORMAL,       // error control type
      module_file_name,           // path to service's binary
      NULL,                       // no load ordering group
      NULL,                       // no tag identifier
      L"W3SVC\0\0",               // Depends on W3SVC.
      // TODO: if we could get IIS Version in some other way,
      // we can do better here. (b=1049762)
      NULL,                       // LocalSystem account
      NULL);                      // no password
  if (service_handler == NULL) {
    DWORD error_code = GetLastError();
    CloseServiceHandle(scm_handle);

    if (error_code != ERROR_SERVICE_EXISTS) {
      Util::Log(EVENT_ERROR, "CreateService failed. Error Code: %d", error_code);
      return error_code;
    }

    // Uninstall service first.
    error_code = UninstallService();
    if (error_code != 0)
      return error_code;

    // Reinstall service again if uninstallation is successful.
    return InstallService();
  } else {
    CloseServiceHandle(service_handler);
    CloseServiceHandle(scm_handle);
    Util::Log(EVENT_CRITICAL, "Service installed successfully");
    return 0;
  }
}

int MainService::ControlService(const char *command, const char *service) {
  PROCESS_INFORMATION pi;
  STARTUPINFOA        si;
  string              command_line = "net.exe";
  DWORD               exit_code = 0;

  command_line.append(" ").append(command);
  command_line.append(" ").append(service);
  memset(&pi, 0, sizeof(pi));
  memset(&si, 0, sizeof(si));
  si.cb= sizeof(si);

  if (!CreateProcessA(NULL, (LPSTR)command_line.c_str(), NULL, NULL, FALSE, 
    CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi)) {
    DWORD error_code = GetLastError();;
    Util::Log(EVENT_ERROR, "Use net.exe to %s service %s failed. Error Code: %d", 
      command, service, error_code);
    return error_code;
  }

  if (WaitForSingleObject(pi.hProcess, 300000)==WAIT_TIMEOUT) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    Util::Log(EVENT_ERROR, "net.exe didn't return in 60 seconds.");
    return 1;
  }

  GetExitCodeProcess(pi.hProcess, &exit_code);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  Util::Log(EVENT_IMPORTANT, "net.exe %s service %s successfully. Exit code: %d", 
    command, service, exit_code);

  return 0; // exit_code is ignored
}

int MainService::UninstallService() {
  SC_HANDLE scm_handle;
  SC_HANDLE service_handler;
  wchar_t   module_file_name[MAX_PATH];

  if(!GetModuleFileName(NULL, module_file_name, MAX_PATH)) {
    DWORD error_code = GetLastError();;
    Util::Log(EVENT_ERROR, "GetModuleFileName falied. Error Code: %d.", error_code );
    return error_code;
  }

  // Get a handle to the ServiceControlManager database.
  scm_handle = OpenSCManager(NULL,                    // local computer
                             NULL,                    // service database
                             SC_MANAGER_ALL_ACCESS);  // full access rights
  if (NULL == scm_handle) {
    DWORD error_code = GetLastError();
    Util::Log(EVENT_ERROR, "OpenSCManager failed. Error Code %d.", error_code);
    return error_code;
  }

  // Get a handle to the service.
  service_handler = OpenService(scm_handle,            // SCM database
                                kServiceName,          // name of service
                                SERVICE_STOP|DELETE);  // access right
  if (service_handler == NULL) {
    DWORD error_code = GetLastError();
    Util::Log(EVENT_ERROR, "OpenService failed. Error Code: %d.", error_code);
    CloseServiceHandle(scm_handle);
    return error_code;
  }

  // Stop the service first if it's still running. Return value is ignored
  // because we will continue even if the service can not be stopped.
  ::ControlService(service_handler, SERVICE_CONTROL_STOP, &service_status_);

  // Delete the service.
  if (!DeleteService(service_handler)) {
    DWORD error_code = GetLastError();
    Util::Log(EVENT_ERROR, "DeleteService failed. Error Code:%d.", error_code);
    CloseServiceHandle(service_handler);
    CloseServiceHandle(scm_handle);
    return error_code;
  }

  CloseServiceHandle(service_handler);
  CloseServiceHandle(scm_handle);
  Util::Log(EVENT_CRITICAL, "Service deleted successfully.");

  // Wait for service database to be updated. 
  Sleep(1000);
  return 0;
}

int MainService::UpdateSetting() {
  Util::Log(EVENT_CRITICAL, "Begin to create default setting file.");
  
  if (!SiteSettings::UpdateSiteSettings()) {
    Util::Log(EVENT_ERROR, "Update settting file failed.");
    return 1;
  } else {
    Util::Log(EVENT_CRITICAL, "Update setting file successfully.");
    return 0;
  }
}

int MainService::RunService(void) {
  SERVICE_TABLE_ENTRY DispatchTable[] = {
    { (wchar_t *)kServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
    { NULL, NULL }
  };

  // This call returns when the service has stopped.
  // The process should simply terminate when the call returns.
  if (!StartServiceCtrlDispatcher(DispatchTable)) {
    DWORD error_code = GetLastError();
    Util::Log(EVENT_ERROR, "StartServiceControlDispatcher failed. Error Code: %d", error_code);
    return error_code;
  }

  return 0;
}

void WINAPI MainService::ServiceMain(DWORD argument_count,
                                     wchar_t **argument_array) {
  Util::Log(EVENT_CRITICAL, "Starting sitemap service.");

  // Register the handler function for the service
  service_status_handle_ = RegisterServiceCtrlHandler(kServiceName,
                                                      ServiceControlHandler);
  if(service_status_handle_ == NULL) {
    Util::Log(EVENT_ERROR, "RegisterServiceControlHandler failed. Error Code: %d.", GetLastError());
    return;
  }

  // These SERVICE_STATUS members remain as set here
  service_status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  service_status_.dwServiceSpecificExitCode = 0;
  service_status_.dwWin32ExitCode = NO_ERROR;
  service_status_.dwWaitHint = 0;

  // Report running status when initialization is complete.
  ReportServiceStatus(SERVICE_RUNNING);

  // Perform service-specific initialization and work.
  RunSitemapService();

  // Report stopped status when work is completed.
  ReportServiceStatus(SERVICE_STOPPED);

  Util::Log(EVENT_CRITICAL, "Exiting sitemap service.");
}

void MainService::RunSitemapService() {
  // Create an event. The control handler function, ServiceControlHandler,
  // signals this event when it receives the stop control code.
  // If RunSitemapService() is called in debugging mode (using "debug" command
  // line), this event will not be signaled forever.
  service_stop_event_handle_ = CreateEvent(NULL,   // default security attr.
                                           TRUE,   // manual reset event
                                           FALSE,  // not signaled
                                           NULL);  // no name
  if (service_stop_event_handle_ == NULL) {
    Util::Log(EVENT_ERROR, "CreateEvent failed in RunSitemapService: %d", GetLastError());
    return;
  }

  ServiceController service_controller;
  if (!service_controller.Initialize()) {
    CloseHandle(service_stop_event_handle_);
    return;
  }

  // waiting_time is used to control how long we should call function
  // sitemap_controller.WriteSitemapFile().
  DWORD waiting_time = 0;

  while(true) {
    // Check whether to stop the service.
    if (WaitForSingleObject(service_stop_event_handle_,
                            waiting_time) != WAIT_TIMEOUT)
      break;

    // ervice_controller.RunService() will return how long we should
    // call it again
    waiting_time = service_controller.RunService();
  }

  service_controller.StopService();
  CloseHandle(service_stop_event_handle_);
}

void WINAPI MainService::ServiceControlHandler(DWORD control_code) {
  // Handle the requested control code.
  switch(control_code) {
    case SERVICE_CONTROL_STOP:
      ReportServiceStatus(SERVICE_STOP_PENDING);

      // Signal the service to stop.
      SetEvent(service_stop_event_handle_);
      return;
    default:
      break;
  }

  ReportServiceStatus(service_status_.dwCurrentState);
}

void MainService::ReportServiceStatus(DWORD current_status) {
  static DWORD dwCheckPoint = 1;

  // Fill in the SERVICE_STATUS structure.
  service_status_.dwCurrentState = current_status;

  if (current_status == SERVICE_START_PENDING)
    service_status_.dwControlsAccepted = 0;
  else
    service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP;

  if (current_status == SERVICE_RUNNING ||
      current_status == SERVICE_STOPPED)
    service_status_.dwCheckPoint = 0;
  else
    service_status_.dwCheckPoint = dwCheckPoint++;

  // Report the status of the service to the ServiceControlManager.
  SetServiceStatus(service_status_handle_, &service_status_);
}

int MainService::StartConfig() {
  // load setting from file
  SiteSettings settings;
  std::string path = SiteSettings::GetDefaultFilePath();
  if (!settings.LoadFromFile(path.c_str()) ) {
    Util::Log(EVENT_ERROR, "StartConfig: Failed to load config file.");
    return 0;
  }

  // build the url
  char portstr[1024];
  itoa(settings.setting_port(), portstr, 10);

  std::string url("http://localhost");
  url.append(":").append(portstr);

  // open a url
  HINSTANCE result = ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
  int intval = *reinterpret_cast<int*>(&result);
  if (intval > 32) {
    return 0;
  } else {
    Util::Log(EVENT_ERROR, "StartConfig: Failed to open browser. (%d)", intval);
    return 0;
  }
}

