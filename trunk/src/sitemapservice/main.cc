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

#include "common/version.h"
#include "common/interproclock.h"
#include "common/port.h"
#include "common/accesscontroller.h"
#include "common/settingmanager.h"
#include "sitemapservice/websitemapservice.h"

void PrintHelp(int exit_code);

int ProcessRemoteAdmin(int argc, const char* argv[]);

#ifdef WIN32

#include "common/logger.h"
#include "common/iisconfig.h"
#include "common/cmdlineflags.h"
#include "sitemapservice/mainservice.h"
#include "sitemapservice/servicecontroller.h"
#include "sitemapservice/passwordmanager.h"
#include "sitemapservice/sitesettingmanager.h"

using std::string;

// Print available command options for user.
void PrintHelp(int exit_code) {
  printf("\
Usage: SitemapService.exe [ reset_password ]\n\
                          [ remote_admin {enable | disable} ]\n\
                          [ help | -h ] [ version | v ]\n\
Options:\n\
  remote_admin        : change \"remote_admin\" flag in setting\n\
  reset_password      : enter interactive mode to reset admin password\n\
  version | -v        : show current version\n\
  help | -h           : list available command line options (this page)\n\
");

  exit(exit_code);
}

// Entry point for the process.
// It will call different functions based on command line type.
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
    PrintHelp(-1);
  }

  if (argc > 1) {
    int return_result = 0;
    const char * parameter = argv[1];

    if (stricmp(parameter, "-h") == 0
      || stricmp(parameter, "help") == 0) {
      PrintHelp(0);
    } else if (stricmp(parameter, "-v") == 0
               || stricmp(parameter, "version") == 0) {
      printf("Google Sitemap Generator (Beta) Version [%s]\n",
             SITEMAP_VERSION1);
      return 0;
    } else if (stricmp(parameter, "remote_admin") == 0) {
      return ProcessRemoteAdmin(argc, argv);
    }

    Logger::Log(EVENT_CRITICAL, "Command option: [%s]", parameter);

    // Call different functions based on command line type
    if (stricmp(parameter, "install_service") == 0) {
      printf("Installing Google Sitemap Generator as a Windows service...\n");
      return_result = MainService::InstallService();
    } else if (stricmp(parameter, "uninstall_service") == 0) {
      printf("Uninstalling Google Sitemap Generator service...\n");
      return_result = MainService::UninstallService();
    } else if (stricmp(parameter, "start_service") == 0) {
      printf("Starting the Google Sitemap Generator service...\n");
      return_result = MainService::ControlService("start", "GoogleSitemapGenerator");
    } else if (stricmp(parameter, "stop_service") == 0) {
      printf("Stoping the Google Sitemap Generator service...\n");
      return_result = MainService::ControlService("stop", "GoogleSitemapGenerator");
    } else if (stricmp(parameter, "reload_setting") == 0) {
      return_result = MainService::ReloadSetting();
    } else if (stricmp(parameter, "restart_webserver") == 0) {
      printf("Restarting IIS...\n");
      return_result = MainService::ControlService("stop", "w3svc");
      return_result = MainService::ControlService("start", "w3svc");
    } else if (stricmp(parameter, "install_filter") == 0) {
      printf("Installing Sitemap plugin for IIS...\n");
      return_result = IisConfig::InstallFilter() ? 0 : 1;
    } else if (stricmp(parameter, "uninstall_filter") == 0) {
      return_result = IisConfig::UninstallFilter() ? 0 : 1;
    } else if (stricmp(parameter, "update_setting") == 0) {
      printf("Updating application settings according to IIS configuration...\n");
      return_result = MainService::UpdateSetting();
    } else if (stricmp(parameter, "debug") == 0) {
      MainService::RunSitemapService();
    } else if (stricmp(parameter, "start_config") == 0) {
      printf("Opening the Admin Console...\n");
      return_result = MainService::StartConfig();
    } else if (stricmp(parameter, "set_permission") == 0) {
      printf("Changing access permission for program files...\n");
      return_result = MainService::SetPermission();
    } else if (stricmp(parameter, "reset_password") == 0) {
      printf("Set the password for the Admin Console...\n");
      return_result = PasswordManager::CreatePassword() ? 0 : 1;
    } else if (stricmp(parameter, "change_password") == 0) {
      return_result = PasswordManager::ChangePassword() ? 0 : 1;
    } else if (stricmp(parameter, "clean_robots") == 0) {
      printf("Removing Sitemap lines from robots.txt...");
      return_result = WebSitemapService::CleanRobotsTxt() ? 0 : 1;
    } else if (stricmp(parameter, "reset_firewall") == 0) {
      printf("Removing firewall setting for Google Sitemap Generator...");
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
      printf("Creating Admin Console site in IIS...\n");
      return_result = IisConfig::InstallAdminConsole() ? 0 : 1;
    } else if (stricmp(parameter, "uninstall_admin_console") == 0) {
      printf("Removing Admin Console site from IIS...\n");
      return_result = IisConfig::UninstallAdminConsole() ? 0 : 1;
    } else {
      printf("Unsupported parameter %s\n", parameter);
      PrintHelp(-1);
    }

    if (return_result == 0) {
      printf("%s successful!\n", parameter);
    } else {
      printf("%s failed. Error code=%d!\n", parameter, return_result);
    }

    return return_result;
  } else {
    // The service started by the Service Controller Manager.
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

// Print available command options for user.
void PrintHelp(int exit_code) {
  printf("\
Usage: sitemap-daemon [ service {start | stop | restart} ]\n\
                      [ reset_password ]\n\
                      [ remote_admin {enable | disable} ]\n\
                      [ help | -h ] [ version | v ]\n\
Options:\n\
  service             : control sitemap-daemon service\n\
  remote_admin        : change \"remote_admin\" flag in setting\n\
  reset_password      : enter interactive mode to reset admin password\n\
  version | -v        : show current version\n\
  help | -h           : list available command line options (this page)\n\
");

  exit(exit_code);
}

// Entry point for the process.
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
    PrintHelp(-1);
  }
  if (flags->check_apache_conf()) {
    ApacheConfig::SetConfFilePath(flags->apache_conf().c_str());
  }
  if (flags->check_apache_group()) {
    AccessController::set_apache_group(flags->apache_group());
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

    Logger::Log(EVENT_CRITICAL, "Command option: [%s]", parameter);

    // Call different functions based on command line type
    if (strcasecmp(parameter, "service") == 0) {
      if (argc != 3) {
        fprintf(stderr, "Invalid number of args for service command.\n");
        PrintHelp(1);
      }
      printf("%s sitemap daemon...\n", argv[2]);
      if (strcasecmp(argv[2], "start") == 0) {
        return_result = Daemon::Start();
      } else if (strcasecmp(argv[2], "stop") == 0) {
        return_result = Daemon::Stop();
      } else if (strcasecmp(argv[2], "restart") == 0) {
        return_result = Daemon::Restart();
      } else {
        fprintf(stderr, "Unrecognized option for service command.\n");
        PrintHelp(1);
      }

      if (return_result == 0) {
        printf("%s successful.\n", argv[2]);
      } else {
        fprintf(stderr, "Failed to %s.\n", argv[2]);
      }

    } else if (strcasecmp(parameter, "remote_admin") == 0) {
      return ProcessRemoteAdmin(argc, argv);
    } else if (strcasecmp(parameter, "reload_setting") == 0) {
      return_result = Daemon::ReloadSetting();
    } else if (strcasecmp(parameter, "debug") == 0) {
      return_result = Daemon::RunService();
    } else if (strcasecmp(parameter, "update_setting") == 0) {
      SettingManager* setting_manager = SettingManager::default_instance();
      return_result = setting_manager->UpdateSettingFile() ? 0 : 1;
    } else if (strcasecmp(parameter, "reset_password") == 0) {
      return_result = PasswordManager::CreatePassword() ? 0 : 1;
    } else if (strcasecmp(parameter, "change_password") == 0) {
      return_result = PasswordManager::ChangePassword() ? 0 : 1;
    } else if (strcasecmp(parameter, "clean_robots") == 0) {
      return_result = WebSitemapService::CleanRobotsTxt() ? 0 : 1;
    } else if (strcasecmp(parameter, "get_site_setting") == 0) {
      if (!flags->check_site_id() || !flags->check_file()) {
        fprintf(stderr, "Error: [site_id] and [file] is required.\n");
        return -1;
      } else {
        return_result = SiteSettingManager::GetSiteSettingToFile(
          flags->site_id(), flags->file()) ? 0 : 1;
      }
    } else if (strcasecmp(parameter, "set_site_setting") == 0) {
      if (!flags->check_site_id() || !flags->check_file()) {
        fprintf(stderr, "Error: [site_id] and [file] is required.\n");
        return -1;
      } else {
        return_result = SiteSettingManager::SetSiteSettingFromFile(
          flags->site_id(), flags->file()) ? 0 : 1;
      }
    } else if (strcasecmp(parameter, "help") == 0
      || strcasecmp(parameter, "-h") == 0) {
      PrintHelp(0);
    } else if (strcasecmp(parameter, "version") == 0
      || strcasecmp(parameter, "-v") == 0) {
      printf("Google Sitemap Generator (Beta) Version [%s]\n",
             SITEMAP_VERSION1);
    } else {
      fprintf(stderr, "Unsupported parameter %s\n", parameter);
      PrintHelp(-1);
    }

    return return_result;
  } else {
    PrintHelp(0);
    return 0;
  }
}

#endif // __linux__

int ProcessRemoteAdmin(int argc, const char* argv[]) {
  int return_result = 0;

  if (argc != 3) {
    fprintf(stderr, "Invalid number of args for remote_admin command.\n");
    PrintHelp(1);
  }

  std::string remote_admin;
  if (stricmp(argv[2], "enable") == 0) {
    remote_admin = "true";
  } else if (stricmp(argv[2], "disable") == 0) {
    remote_admin = "false";
  } else {
    fprintf(stderr, "Unrecognized option for remote_admin command.\n");
    PrintHelp(1);
  }

  SettingManager* setting_manager = SettingManager::default_instance();
  return_result = setting_manager->SetApplicationAttribute(
    "remote_admin", remote_admin) ? 0 : 1;
  if (return_result == 0) {
    printf("Set remote_admin successful.\n");
  } else {
    fprintf(stderr, "Failed to set remote_admin.\n");
  }
  if (return_result == 0) {
    printf("Try to reload setting to make remote_admin take effect...\n");
#ifdef WIN32
    return_result = MainService::ReloadSetting();
#else
    return_result = Daemon::ReloadSetting();
#endif
    if (return_result != 0) {
      fprintf(stderr, "Faild to reload setting.\n");
    } else {
      printf("Setting is reloaded successfully.\n");
    }
  }

  return return_result;
}

