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


// IisConfig provides an abstraction on configuration for different IIS version.
// It uses state pattern to hide IIS version information from client.
// But please note that, it is not an typical or orthodox state pattern.
// The state is determined on the fly.

#ifndef COMMON_IISCONFIG_H__
#define COMMON_IISCONFIG_H__

#include <windows.h>
#include "common/webserverconfig.h"
#include "common/criticalsection.h"

class IisConfig: public WebserverConfig {
 public:
  // Defines the IIS running mode.
  enum AppMode {
    MODE_X64,  // IIS runs under x86_64 mode.
    MODE_X86,  // IIS runs under i386 mode.
    MODE_UNKNOWN,  // IIS run under an unknow mode.
    MODE_FAIL  // Failed to retrieve app mode.
  };

  IisConfig() {}
  virtual ~IisConfig() {}

  // Load configuration from IIS.
  virtual bool Load();

  // Install filters in IIS.
  // The actual filter installed depends on IIS version and IIS running mode.
  // For IIS 7.0, the official name for this filter is Module.
  static bool InstallFilter();

  // Remove the filter(module) from IIS.
  static bool UninstallFilter();

  // Create a site for Admin Console,
  // whose name is "Google Sitemap Generator Admin Console".
  // Permissions for CGI exe are set accordingly.
  static bool InstallAdminConsole();

  // Remove the Admin Console site.
  // This method removes the site named as
  // "Google Sitemap Generator Admin Console".
  static bool UninstallAdminConsole();

  // Returns 6 for IIS 6.0, 7 for IIS 7.0.
  // 0 for not found.
  static int GetIisVersion();

  // Get IIS version information from registry.
  // Returns -1 if failed.
  static int GetMajorVersinFromReg();

  // Get IIS running mode.
  static AppMode GetAppMode();

  // A wrapper on GetNativeSystemInfo system call.
  // GetNativeSystemInfo is not part of Win2K API.
  // Returns -1 if error occurs, 0 if not support, 1 if successful.
  static int GetNativeSystemInfo(SYSTEM_INFO* system_info);

  // Declare the server binding for Admin Console site.
  // "8181" port is binded to Admin Console site.
  static const wchar_t* kAdminConsoleBinding;

 private:
  // Cached iis_version value.
  static int iis_version_;

  // Thread-safe protector for iis_version_ value accessor. 
  static CriticalSection critical_section_;
};

#endif  // COMMON_IISCONFIG_H__
