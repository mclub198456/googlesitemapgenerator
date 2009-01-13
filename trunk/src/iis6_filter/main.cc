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


#include "common/logger.h"
#include "common/util.h"
#include "common/iisconfig.h"
#include "iis6_filter/sitemapfilter.h"

using std::string;

static SitemapFilter* sitemap_filter = NULL;

BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID) {
  // Set log type.
  Logger::SetLogType(EVENT_SYSLOG | EVENT_APPLOG);

  // Set log path.
  std::string module_dir;
  if (!Util::GetModuleDir(handle, &module_dir)) {
    Logger::Log(EVENT_ERROR, "Failed to get sitemap filter dll path.");
    return FALSE;
  }
  module_dir.append("\\google-sitemap-generator.log");
  Logger::SetLogFileName(module_dir);

  if (reason == DLL_PROCESS_ATTACH) {
    Logger::Log(EVENT_CRITICAL, "Attaching sitemapfilter.dll.");

    sitemap_filter = new SitemapFilter();
    if (sitemap_filter == NULL) {
      Logger::Log(EVENT_ERROR, "Failed to create sitemap filter.");
      return FALSE;
    }

    if (!sitemap_filter->Initialize(handle)) {
      delete sitemap_filter;
      sitemap_filter = NULL;
      Logger::Log(EVENT_ERROR, "Failed to initialize sitemap filter.");
      return FALSE;
    }
  } else if (reason == DLL_PROCESS_DETACH) {
    Logger::Log(EVENT_CRITICAL, "Detaching sitemapfilter.dll.");

    if (sitemap_filter != NULL) {
      delete sitemap_filter;
      sitemap_filter = NULL;
    }
  }

  return TRUE;
}

// Every ISAPI filter is contained in a separate DLL that must export
// two entry-point functions, GetFilterVersionand HttpFilterProc, and
// optionally export the TerminateFilter function.

// The GetFilterVersion function is the first entry-point function
// called by IIS on your ISAPI filter, and must be present for the filter
// to work properly. IIS passes a pointer to a HTTP_FILTER_VERSION Structure,
// which can be used to supply important filter configuration
// information to IIS.
// The most important information passed to IIS is the bitmask that contains
// flags that specify which notification events your filter can process,
// and a flag that indicates the overall processing priority for your filter.
BOOL WINAPI GetFilterVersion(HTTP_FILTER_VERSION* filter_version) { 
  try {
    Logger::Log(EVENT_CRITICAL, "Getting filter version information.");

    if (!sitemap_filter->GetFilterVersion(filter_version)) {
      Logger::Log(EVENT_ERROR, "GetFilterVersion failed.");      
      return FALSE;
    }

    return TRUE;
  }
  catch (...) {
    Logger::Log(EVENT_ERROR, "Exception occured in GetFilterVersion().");
    return FALSE;
  }
} 

// Each HTTP transaction between IIS and a client browser triggers
// several distinct events. Every time an event occurs for which
// an ISAPI filter is registered, IIS calls the filter's HttpFilterProc
// entry-point function. 
DWORD WINAPI HttpFilterProc(HTTP_FILTER_CONTEXT *filter_context, 
                            DWORD notification_type, 
                            VOID *data) {
  return sitemap_filter->Process(filter_context, notification_type, data);
}

BOOL WINAPI TerminateFilter(DWORD flags) {
  Logger::Log(EVENT_CRITICAL, "Terminating sitemap filter");
	return TRUE;
}
