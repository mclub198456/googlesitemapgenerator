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


// SitemapFilter will do the real work for IIS6 ISAPI filter. The exported API
// will simply delegate to corresponding function in this class.
// This class is and thread-safe, and MUST be thread-safe.

#ifndef IIS6_FILTER_SITEMAPFILTER_H__
#define IIS6_FILTER_SITEMAPFILTER_H__

#include <windows.h>
#include <httpfilt.h>


#include "common/basefilter.h"


class SitemapFilter : public BaseFilter
{
 public:
   // Constructor. A default UrlPipe will be used.
  SitemapFilter() {};
   
   // Constructor with customized UrlPipe.
  SitemapFilter(UrlPipe* pipe) : BaseFilter(pipe) {};

  virtual ~SitemapFilter() {};

  // Initialize with filter DLL module handle.
  // Via this handle, the filter could locate the setting files.
  bool Initialize(HMODULE module_handle);

  // Actual implementation of ISAPI GetFilterVersion method.
  // Besides the version information, the filter also tells IIS which kinds of
  // notifications it want to receive.
  bool GetFilterVersion(HTTP_FILTER_VERSION *version);

  // Actual implementation fo ISAPI HttpFilterProc method.
  // This method extracts http request/response information through ISAPI,
  // creates a UrlRecord and sents it to BaseFilter::Send.
  // This method only reads informations, but does no writting.
	DWORD Process(HTTP_FILTER_CONTEXT *context, DWORD notification_type, VOID *data);
};

#endif  // IIS6_FILTER_SITEMAPFILTER_H__