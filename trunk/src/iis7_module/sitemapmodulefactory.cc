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


#include "iis7_module/sitemapmodulefactory.h"

#include <string>
#include "common/util.h"
#include "common/logger.h"
#include "common/version.h"
#include "common/basefilter.h"
#include "common/settingmanager.h"
#include "iis7_module/sitemapmodule.h"

extern HINSTANCE dll_handle;

SitemapModuleFactory::SitemapModuleFactory() {
  basefilter_ = NULL;
}
SitemapModuleFactory::~SitemapModuleFactory() {
  if (basefilter_ != NULL) {
    delete basefilter_;
    basefilter_ = NULL;
  }
}

bool SitemapModuleFactory::Initialize() {
  if (basefilter_ != NULL) {
    Logger::Log(EVENT_ERROR, "SitemapModuleFactory is initialized twice.");
    return false;
  }

  Logger::Log(EVENT_CRITICAL, "=== Google Sitemap Generator IIS Module [%s] ===",
            SITEMAP_VERSION1);

  // Get and set setting file path.
  std::string module_dir;
  if (!Util::GetModuleDir(dll_handle, &module_dir)) {
    Logger::Log(EVENT_ERROR, "Failed to get sitemap module dll dir.");
    return false;
  }
  module_dir.append("//sitesettings.xml");

  SettingManager* manager = SettingManager::default_instance();
  manager->Initialize(module_dir);
  
  SiteSettings settings;
  if (!manager->LoadApplicationSetting(&settings)) {
    Logger::Log(EVENT_ERROR, "Failed to load app setting from file (%s).",
              module_dir.c_str());
    return false;
  }

  // Initialize flags.
  Logger::SetLogLevel(settings.logging_level());

  basefilter_ = new BaseFilter();
  bool result = basefilter_->Initialize(settings);
  if (!result) {
    delete basefilter_;
    basefilter_ = NULL;
  }

  return result;
}

HRESULT SitemapModuleFactory::GetHttpModule(OUT CHttpModule ** ppModule,
                      IN IModuleAllocator * pAllocator) {

  UNREFERENCED_PARAMETER( pAllocator );

  // Create a new instance.
  SitemapModule* module = new SitemapModule(basefilter_);
  if (module == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to create sitemap module.");
    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
  } else {
    *ppModule = module;
    return S_OK;
  }
}

void SitemapModuleFactory::Terminate() {
  // wierd, but is copied from MSDN
  delete this;
}
