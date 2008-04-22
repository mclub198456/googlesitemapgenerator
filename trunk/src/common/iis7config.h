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


// This class provides function to load configuration from IIS 7, and functions
// to install/uninstall IIS7 Module. It also provides some functions to
// determine whether IIS 7 is supported on current system, and function to
// check on which mode IIS runs.
// Implementation of this class is based on IIS7 native API.

#ifndef COMMON_IIS7CONFIG_H__
#define COMMON_IIS7CONFIG_H__

#include <ahadmin.h>
#include <string>
#include "common/iisconfig.h"

// This class uese IIS7.0 native API to operate on
// %windir%\system32\inetsrv\config\applicationHost.config
class Iis7Config: public IisConfig {
 public:
  Iis7Config() {}
  virtual ~Iis7Config() {}

  // Install/uninstall filters in IIS.
  // Actually, add/remove entry to/from <system.webServer/globalModules>
  // adn <system.webServer/modules> sections.
  static bool InstallFilter(const wchar_t* dll_file);
  static bool UninstallFilter();

  // Determine whether version 7 mode supported.
  static bool IsSupported();

  // Load configuration from IIS
  // Actually, load from <system.applicationHost/sites> section.
  virtual bool Load();

  // Get IIS 7 running mode.
  static AppMode GetAppMode();

 private:
  // The sitemap module name, "SitemapModule".
  static BSTR kModuleName;

  // Add an entry to <system.webServer/globalModules> section, like:
  // <add name="SitemapModule" image="path\sitemap_module.dll" />
  static bool AddToGlobalModules(IAppHostWritableAdminManager* manager,
                                 BSTR image);

  // Add an entry to <system.webServer/modules> section, like:
  // <add name="SitemapModule" />
  static bool AddToModules(IAppHostWritableAdminManager* manager);

  // Set property value.
  static bool SetProperty(IAppHostElement* element, BSTR name, BSTR value);

  // Get property value.
  static bool GetProperty(IAppHostElement* element, BSTR name, VARIANT* value);

  // Get the index from given collection, whose key=value.
  // *index == -1 indicates nothing found.
  static bool GetFromCollection(IAppHostElementCollection* collection,
                                BSTR property_key, BSTR property_value,
                                short* index);

  // Get the element from given colletion.
  // The element should contains a propery, whose key/value is given by arg.
  // "*element = NULL" indicates nothing can be found.
  static bool GetFromCollection(IAppHostElementCollection* collection,
                                BSTR property_key, BSTR property_value,
                                IAppHostElement** element);

  // Remove SitemapModule entry from given section, which could be,
  // <system.webServer/globalModules> section
  // OR <system.webServer/modules> section
  static bool RemoveSitemapModule(IAppHostWritableAdminManager* manager,
                                  BSTR section);

  // Load site configuration from an element.
  bool LoadSite(IAppHostElement* site_element);

};

#endif  // COMMON_IIS7CONFIG_H__
