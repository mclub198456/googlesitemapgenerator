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


#include "iis7_module/sitemapmodulefactory.h"

#include <string>
#include "common/util.h"
#include "common/version.h"
#include "common/basefilter.h"
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
    Util::Log(EVENT_ERROR, "SitemapModuleFactory is initialized twice.");
    return false;
  }

  Util::Log(EVENT_CRITICAL, "=== Google Sitemap Generator IIS Module [%s] ===",
            SITEMAP_VERSION1);

  // Get setting file path.
  std::string module_dir;
  if (!Util::GetModuleDir(dll_handle, &module_dir)) {
    Util::Log(EVENT_ERROR, "Failed to get sitemap module dll dir.");
    return false;
  }
  std::string settings_file = SiteSettings::GetDefaultFilePath(module_dir);

  // Load settings.
  SiteSettings settings;
  if (!settings.LoadFromFileForFilter(settings_file.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to load setting from file.");
    return false;
  }

  // Initialize flags.
  Util::SetLogLevel(settings.logging_level());

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
    Util::Log(EVENT_ERROR, "Failed to create sitemap module.");
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
