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


// Defines entry point for sitemap module dll.

#include "iis7_module/sitemapmodulefactory.h"
#include "common/util.h"
#include "common/logger.h"

// DLL handle
HINSTANCE dll_handle = NULL;

// module's exported registration function.
HRESULT __stdcall RegisterModule(DWORD dwServerVersion,
                                 IHttpModuleRegistrationInfo * pModuleInfo,
                                 IHttpServer * pGlobalInfo) {
  UNREFERENCED_PARAMETER(dwServerVersion);
  UNREFERENCED_PARAMETER(pGlobalInfo);

    // Set the request notifications and exit.
  SitemapModuleFactory* module_factory = new SitemapModuleFactory();
  if (!module_factory->Initialize()) {
    delete module_factory;
    Logger::Log(EVENT_ERROR, "Failed to intialize SitemapModuleFactory. Skip.");
    return S_OK;
  } else {
    return pModuleInfo->SetRequestNotifications(module_factory,
                                                RQ_LOG_REQUEST, 0);
  }
}

BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID) {
  dll_handle = handle;

  // Set log type.
  Logger::SetLogType(EVENT_SYSLOG | EVENT_APPLOG);

  // Set log path.
  std::string module_path;
  if (!Util::GetModuleDir(handle, &module_path)) {
    Logger::Log(EVENT_ERROR, "Failed to get sitemap module dll dir.");
    return FALSE;
  }
  module_path.append("\\google-sitemap-generator.log");
  Logger::SetLogFileName(module_path);

  // for tracing purpose
  if (reason == DLL_PROCESS_ATTACH) {
    Logger::Log(EVENT_CRITICAL, "Attaching SitemapModule.dll.");
  } else if (reason == DLL_PROCESS_DETACH) {
    Logger::Log(EVENT_CRITICAL, "Detaching SitemapModule.dll.");
  }

  return TRUE;
}
