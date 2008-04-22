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


#ifdef WIN32

#include "common/util.h"
#include "common/iisconfig.h"
#include "sitemapservice/mainservice.h"

using std::string;

// Entry point for the process.
// It will call different functions based on command line type.
//
// Command Line             Purpose
// "install_service"    Install this process as a Windows Service
// "uninstall_service"  Uninstall this process from Windows Service
// "start_service"      Ask service manager to start sitemap service
// "stop_service"       Ask service manager to stop sitemap service
// "restart_webserver"  Ask service manager to restart webserver
// "install_filter"     Install web server filter
// "uninstall_filter"   Uninstall web server filter
// "update_setting"     Update setting file and save non-duplicated attributes
// "debug"          Launch the work function directly for debugging purpose
//  none                Started by Windows ServiceControlManager directly.
//
// Return 0 if operation is succesful, otherwise return an error code.
int __cdecl wmain(int argc, wchar_t *argv[]) {
  // Initialize logging level, log dir, and etc
  if (!Util::InitFlags()) {
    Util::Log(EVENT_ERROR, "Failed to init global flags. Exit.");
    return -1;
  }

  if (argc > 1) {
    int return_result = 0;
    wchar_t * parameter = argv[1];
    if (parameter[0] == L'-' || parameter[0] == L'/')
      parameter++;

    wprintf( L"%s...\n", parameter);

    // Call different functions based on command line type
    if (wcsicmp(parameter, L"install_service") == 0) {
      return_result = MainService::InstallService();
    } else if (wcsicmp(parameter, L"uninstall_service") == 0) {
      return_result = MainService::UninstallService();
    } else if (wcsicmp(parameter, L"start_service") == 0) {
      return_result = MainService::ControlService("start", "GoogleSitemapGenerator");
    } else if (wcsicmp(parameter, L"stop_service") == 0) {
      return_result = MainService::ControlService("stop", "GoogleSitemapGenerator");
    } else if (wcsicmp(parameter, L"restart_webserver") == 0) {
      return_result = MainService::ControlService("stop", "w3svc");
      return_result = MainService::ControlService("start", "w3svc");
    } else if (wcsicmp(parameter, L"install_filter") == 0) {
      return_result = IisConfig::InstallFilter() ? 0 : 1;
    } else if (wcsicmp(parameter, L"uninstall_filter") == 0) {
      return_result = IisConfig::UninstallFilter() ? 0 : 1;
    } else if (wcsicmp(parameter, L"update_setting") == 0) {
      return_result = MainService::UpdateSetting();
    } else if (wcsicmp(parameter, L"debug") == 0) {
      MainService::RunSitemapService();
    } else if (wcsicmp(parameter, L"start_config") == 0 ) {
      return_result = MainService::StartConfig();
    }else {
      wprintf( L"Unsupported parameter %s\n", parameter);
      return -1;
    }

    if (return_result == 0)
      wprintf( L"%s successful!\n", parameter);
    else
      wprintf( L"%s failed. Error code=%d!\n", parameter, return_result);

    return return_result;
  } else {
    // Otherwise, the service is probably started by the Service Control Manager.
    return MainService::RunService();
  }
}
#endif // WIN32

#ifdef __linux__

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "common/util.h"
#include "common/sitesettings.h"
#include "common/apacheconfig.h"
#include "sitemapservice/recordfilemanager.h"
#include "sitemapservice/daemon.h"

// Entry point for the process.
// It will call different functions based on command line type.
//
// Command Line       Purpose
// "start"            Start daemon
// "stop"             Stop daemon
// "restart"          Restart daemon
// "debug"            Run the servce as a stand alone application.
// "update_setting"   Update default setting file.
//  none              Same as "debug"
//
// Return 0 if operation is succesful, otherwise return an error code.
int main(int argc, const char *argv[]) {
  // requires the root
  if (getuid() != 0) {
    printf("Permission denied. Please run it as a super user.");
    return EACCES;
  }

  // Parse options from second value.
  for (int i = 2; i < argc; ++i) {
    if (strstr(argv[i], "apache_conf=") != NULL) {
      ApacheConfig::SetConfFilePath(argv[i] + strlen("apache_conf="));
      break;
    }
  }

  if (!Util::InitFlags()) {
    fprintf(stderr, "Failed to initialize global flags.");
    return -1;
  }

  // TODO: get configured file path...
  RecordfileManager::SetRecordfileHome("/var/spool/google-sitemap-generator");

  if (!Util::RunWithApacheGroup()) {
    fprintf(stderr, "Failed to switch process group to apache.");
    return -1;
  }
  if (argc > 1) {
    int return_result = 0;
    const char * parameter = argv[1];
    if (parameter[0] == '-' || parameter[0] == '/')
      parameter++;

    printf("%s...\n", parameter);

    // Call different functions based on command line type
    if (strcasecmp(parameter, "start") == 0) {
      return_result = Daemon::Start();
    } else if (strcasecmp(parameter, "stop") == 0) {
      return_result = Daemon::Stop();
    } else if (strcasecmp(parameter, "restart") == 0) {
      return_result = Daemon::Restart();
    } else if (strcasecmp(parameter, "debug") == 0) {
      return_result = Daemon::RunService();
    } else if (strcasecmp(parameter, "update_setting") == 0) {
      return_result = SiteSettings::UpdateSiteSettings() ? 0 : 466453;
    } else {
      fprintf(stderr, "Unsupported parameter %s\n", parameter);
      return -1;
    }

    if (return_result == 0) {
      printf("%s successful!\n", parameter);
    }else {
      fprintf(stderr, "%s failed. Error code = %d!\n",
              parameter, return_result);
    }

    return return_result;
  } else {
    // Otherwise, start the service as a console app.
    return Daemon::RunService();
  }
}

#endif // __linux__
