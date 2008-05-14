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


#ifndef APACHE_MODULE_SITEMAPMODULE_H__
#define APACHE_MODULE_SITEMAPMODULE_H__

#include "common/basefilter.h"

#include "httpd.h"
#include "http_config.h"
#include "ap_mmn.h"  // for MODULE_MAGIC_COOKIE

#include <pthread.h>
#include <map>
#include <string>

// apache 2.2 0x41503232UL, apache 2.0 0x41503230UL
// apache 1.3 with EAPI 0x45415049UL, without EAPI 0x41503133UL
#if MODULE_MAGIC_COOKIE == 0x41503230UL
#define APACHE_VERSION 20
#elif MODULE_MAGIC_COOKIE == 0x41503232UL
#define APACHE_VERSION 22
#elif MODULE_MAGIC_COOKIE == 0x45415049UL || MODULE_MAGIC_COOKIE == 0x41503133UL
#define APACHE_VERSION 13
#else
#error "Only apache 1.3, 2.0, 2.2 is supported."
#endif

// SitemapModule will do the real work as an apache module. The exported API
// will simply delegate to corresponding function in this class.
class SitemapModule : public BaseFilter
{
 public:
   SitemapModule();
   ~SitemapModule();

  bool Initialize();
  int Process(request_rec* req);

private:
  inline int CopyString(char* dest, int offset, int maxsize, const char* src) {
    if (src == NULL) return -1;
    size_t len = strlen(src);
    if (len + offset >= maxsize) return -1;
    memcpy(dest + offset, src, len);
    return offset + len;
  }

  std::string MatchSite(server_rec* server);

  bool inited_;

  // match a vhost to site id.
  // If the site is not enabled, site_id is empty.
  std::map<void*, std::string> siteids_;
  pthread_mutex_t siteid_mutex_;
};

#endif // APACHE_MODULE_SITEMAPMODULE_H__
