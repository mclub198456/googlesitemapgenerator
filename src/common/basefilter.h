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


// BaseFilter is the base class for all webserver plugins. It is named with
// "Filter" because IIS6 Filter is developed first. Developers don't change its
// name after IIS7 Module and Apache Module is introduced.
// It maintains a list of available sites, and provides site-id matching
// function. It also wraps the IPC calls to send URL record to service side.
// Besides these two functions, this class also provides functions like
// checking status code, copy site id, and etc.
// This class is thread-safe, and MUST be thread-safe.

#ifndef COMMON_BASEFILTER_H__
#define COMMON_BASEFILTER_H__

#include <set>
#include <string>
#include <vector>

#include "common/urlpipe.h"
#include "common/sitesettings.h"

class BaseFilter {
 public:
  // Constructor. A default UrlPipe is created internally.
  BaseFilter();

  // This constructor allows customized UrlPipe.
  // The "pipe" pointer will be released in the destructor.
  BaseFilter(UrlPipe* pipe);

  virtual ~BaseFilter();

  // This method build a site-id map from given setting, and initializes
  // internal UrlPipe as a sender. The site-id mapping maps a string site id
  // to an integer site index. This integer site index should be recognized by
  // service side.
  // It only loads the sites, which is enabled and whose WebServerFilter is
  // enabled is loaded.
  bool Initialize(const SiteSettings& settings);

  // Return an integer site index for given siteid string.
  // "false" is returned for un-matched site. In this case, the record for given
  // site shouldn't be sent.
  bool MatchSite(const char* siteid);

  // Copy site id into "dest". Only maxlen should be used in dest.
  bool CopySiteId(int siteindex, char* dest, int maxlen);

  // Send a UrlRecord to service side through UrlPipe.
  // Before sending, this method makes record->host to lower case.
  bool Send(UrlRecord* record);

  // Whether given file should be treated as a static web page.
  // The sub-class (concrete filter) should check the web page size on disk.
  // If the size on disk is same as Content-Length header code, the concrete
  // filter should treat the URL as static page. Otherwise, it should use this
  // method to check whether the URL is a static page.
  // An example of such page is an SHTML web page, whose disk size is not the
  // same with content-length.
  static bool TreatAsStatic(const char* file);

  // Check the http response status code.
  // If this method returns false, the concrete filter shouldn't process the
  // URL any more, but should return immediately.
  // In current implementation, 200, 301, 302, 307 and 404 are accepted.
  static bool CheckStatusCode(int status);

  // Parse "Last-Modified" Http header.
  static bool ParseTime(const char *str, time_t* time);

 private:
  // Build site id mapping, which maps string id to an integer index.
  bool BuildSiteIdMap();

  // UrlPipe used to communicate with service side.
  UrlPipe* pipe_;

  SiteSettings settings_;

  // If default_enabled_ is true, site_ids_ contains disabled sites.
  bool default_enabled_;
  std::set<std::string> site_ids_;
};

#endif // COMMON_BASEFILTER_H__
