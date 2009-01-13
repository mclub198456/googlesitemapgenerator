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


// This class provides function to load configuration from IIS 6, and functions
// to install/uninstall ISAPI Filter. It also provides some functions to
// determine whether IIS 6 is supported on current system, and function to
// check on which mode IIS runs.
// Implementations of this class is based on ADSI scripts.

#ifndef COMMON_IIS6CONFIG_H__
#define COMMON_IIS6CONFIG_H__

#include "windows.h"
#include "common/iisconfig.h"

struct IADs;

class Iis6Config: public IisConfig {
 public:
  Iis6Config() {}
  virtual ~Iis6Config() {}

  // Load configuration from IIS.
  virtual bool Load();

  // Install/uninstall filters in IIS.
  static bool InstallFilter(const wchar_t* dll_file);
  static bool UninstallFilter();

  // Install/uninstall a site for Admin Console.
  // Permissions for CGI exe are set/unset accordingly.
  static bool InstallAdminConsole(const char* site_root,
                                  const char* cgi_path);
  static bool UninstallAdminConsole(const char* site_root,
                                    const char* cgi_path);

  // Determine whether version 6 mode supported.
  static bool IsSupported();

  // Get running mode of iis 6.
  static AppMode GetAppMode();

 private:
  // Helper functions to load site configuration.
  HRESULT LoadSiteId(IADs *web_server);
  HRESULT LoadName(IADs *web_server);
  HRESULT LoadServerBindings(IADs *web_server);
  HRESULT LoadPhysicalPath(IADs *web_server);
  HRESULT LoadLogPath(IADs *web_server);
};

#endif  // COMMON_IIS6CONFIG_H__
