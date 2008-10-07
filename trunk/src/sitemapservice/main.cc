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

#include "common/interproclock.h"
#include "common/port.h"
#include "common/accesscontroller.h"

#ifdef WIN32

#include "common/logger.h"
#include "common/iisconfig.h"
#include "common/cmdlineflags.h"
#include "sitemapservice/mainservice.h"
#include "sitemapservice/servicecontroller.h"
#include "sitemapservice/passwordmanager.h"
#include "sitemapservice/sitesettingmanager.h"

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
int __cdecl main(int argc, const char *argv[]) {
  // Initialize logging level, log dir, and etc
  if (!Util::InitFlags()) {
    Logger::Log(EVENT_ERROR, "Failed to init global flags. Exit.");
    return -1;
  }

  // Parse options from second value.
  CmdLineFlags* flags = CmdLineFlags::GetInstance();
  if (!flags->Parse(argc, argv)) {
    fprintf(stderr, "Failed to parse command line args.\n");
    return -1;
  }

  if (argc > 1) {
    int return_result = 0;
    const char * parameter = argv[1];
    if (parameter[0] == '-' || parameter[0] == '/')
      parameter++;

    printf("%s...\n", parameter);
    Logger::Log(EVENT_CRITICAL, "Command option: [%s]", parameter);

    // Call different functions based on command line type
    if (stricmp(parameter, "install_service") == 0) {
      return_result = MainService::InstallService();
    } else if (stricmp(parameter, "uninstall_service") == 0) {
      return_result = MainService::UninstallService();
    } else if (stricmp(parameter, "start_service") == 0) {
      return_result = MainService::ControlService("start", "GoogleSitemapGenerator");
    } else if (stricmp(parameter, "stop_service") == 0) {
      return_result = MainService::ControlService("stop", "GoogleSitemapGenerator");
    } else if (stricmp(parameter, "reload_setting") == 0) {
      return_result = MainService::ReloadSetting();
    } else if (stricmp(parameter, "restart_webserver") == 0) {
      return_result = MainService::ControlService("stop", "w3svc");
      return_result = MainService::ControlService("start", "w3svc");
    } else if (stricmp(parameter, "install_filter") == 0) {
      return_result = IisConfig::InstallFilter() ? 0 : 1;
    } else if (stricmp(parameter, "uninstall_filter") == 0) {
      return_result = IisConfig::UninstallFilter() ? 0 : 1;
    } else if (stricmp(parameter, "update_setting") == 0) {
      return_result = MainService::UpdateSetting();
    } else if (stricmp(parameter, "debug") == 0) {
      MainService::RunSitemapService();
    } else if (stricmp(parameter, "start_config") == 0) {
      return_result = MainService::StartConfig();
    } else if (stricmp(parameter, "set_permission") == 0) {
      return_result = MainService::SetPermission();
    } else if (stricmp(parameter, "set_password") == 0) {
      return_result = PasswordManager::CreatePassword() ? 0 : 1;
    } else if (stricmp(parameter, "change_password") == 0) {
      return_result = PasswordManager::ChangePassword() ? 0 : 1;
    } else if (stricmp(parameter, "reset_firewall") == 0) {
      if (!ServiceController::ChangeFirewallSetting(false)) {
        fprintf(stderr, "WARNING: Failed to reset firewall setting.");
      }
      return_result = 0;
    } else if (stricmp(parameter, "get_site_setting") == 0) {
      if (!flags->check_site_id() || !flags->check_file()) {
        fprintf(stderr, "Error: [site_id] and [file] is required.");
        return -1;
      } else {
        return_result = SiteSettingManager::GetSiteSettingToFile(
          flags->site_id(), flags->file()) ? 0 : 1;
      }
    } else if (stricmp(parameter, "set_site_setting") == 0) {
      if (!flags->check_site_id() || !flags->check_file()) {
        fprintf(stderr, "Error: [site_id] and [file] is required.");
        return -1;
      } else {
        return_result = SiteSettingManager::SetSiteSettingFromFile(
          flags->site_id(), flags->file()) ? 0 : 1;
      }
    } else if (stricmp(parameter, "install_admin_console") == 0) {
      return_result = IisConfig::InstallAdminConsole() ? 0 : 1;
    } else if (stricmp(parameter, "uninstall_admin_console") == 0) {
      return_result = IisConfig::UninstallAdminConsole() ? 0 : 1;
    } else {
      printf("Unsupported parameter %s\n", parameter);
      return -1;
    }

    if (return_result == 0)
      printf("%s successful!\n", parameter);
    else
      printf("%s failed. Error code=%d!\n", parameter, return_result);

    return return_result;
  } else {
    // Otherwise, the service is probably started by the Service Control Manager.
    return MainService::RunService();
  }
}
#endif // WIN32

#if defined(__linux__) || defined(__unix__)

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "common/logger.h"
#include "common/sitesettings.h"
#include "common/apacheconfig.h"
#include "common/cmdlineflags.h"
#include "sitemapservice/daemon.h"
#include "sitemapservice/passwordmanager.h"
#include "sitemapservice/sitesettingmanager.h"

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
  // Requires the root
  if (getuid() != 0) {
    fprintf(stderr, "Permission denied. Please run it as a super user.\n");
    return EACCES;
  }

  // Parse options from second value.
  CmdLineFlags* flags = CmdLineFlags::GetInstance();
  if (!flags->Parse(argc, argv)) {
    fprintf(stderr, "Failed to parse command line args.\n");
    return -1;
  }
  if (flags->check_apache_conf()) {
    ApacheConfig::SetConfFilePath(flags->apache_conf().c_str());
  }

  if (!Util::InitFlags()) {
    fprintf(stderr, "Failed to initialize global flags.\n");
    return -1;
  }

  if (!AccessController::RunWithApacheGroup()) {
    fprintf(stderr, "Failed to switch process group to apache group.\n");
    return -1;
  }
  if (argc > 1) {
    int return_result = 0;
    const char * parameter = argv[1];
    if (parameter[0] == '-' || parameter[0] == '/')
      parameter++;

    Logger::Log(EVENT_CRITICAL, "Command option: [%s]", parameter);

    // Call different functions based on command line type
    if (strcasecmp(parameter, "start") == 0) {
      printf("Start sitemap daemon...\n");
      return_result = Daemon::Start();
      if (return_result == 0) {
        printf("Start successful.\n");
      } else {
        fprintf(stderr, "Failed to start.\n");
      }
    } else if (strcasecmp(parameter, "stop") == 0) {
      printf("Stop sitemap daemon...\n");
      return_result = Daemon::Stop();
      if (return_result == 0) {
        printf("Stop successful.\n");
      } else {
        fprintf(stderr, "Failed to stop.\n");
      }

    } else if (strcasecmp(parameter, "restart") == 0) {
      printf("Restart sitemap daemon...\n");
      return_result = Daemon::Restart();
      if (return_result == 0) {
        printf("Restart successful.\n");
      } else {
        fprintf(stderr, "Failed to restart.\n");
      }

    } else if (strcasecmp(parameter, "reload_setting") == 0) {
      return_result = Daemon::ReloadSetting();
    } else if (strcasecmp(parameter, "debug") == 0) {
      return_result = Daemon::RunService();
    } else if (strcasecmp(parameter, "update_setting") == 0) {
      SettingManager* setting_manager = SettingManager::default_instance();
      return_result = setting_manager->UpdateSettingFile() ? 0 : 1;
    } else if (strcasecmp(parameter, "set_password") == 0) {
      return_result = PasswordManager::CreatePassword() ? 0 : 1;
    } else if (strcasecmp(parameter, "change_password") == 0) {
      return_result = PasswordManager::ChangePassword() ? 0 : 1;
    } else if (strcasecmp(parameter, "get_site_setting") == 0) {
      if (!flags->check_site_id() || !flags->check_file()) {
        fprintf(stderr, "Error: [site_id] and [file] is required.");
        return -1;
      } else {
        return_result = SiteSettingManager::GetSiteSettingToFile(
          flags->site_id(), flags->file()) ? 0 : 1;
      }
    } else if (strcasecmp(parameter, "set_site_setting") == 0) {
      if (!flags->check_site_id() || !flags->check_file()) {
        fprintf(stderr, "Error: [site_id] and [file] is required.");
        return -1;
      } else {
        return_result = SiteSettingManager::SetSiteSettingFromFile(
          flags->site_id(), flags->file()) ? 0 : 1;
      }
    } else {
      fprintf(stderr, "Unsupported parameter %s\n", parameter);
      return -1;
    }

    return return_result;
  } else {
    // Otherwise, start the service as a console app.
    return Daemon::RunService();
  }
}

#endif // __linux__
