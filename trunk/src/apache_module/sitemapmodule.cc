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


#include "apache_module/sitemapmodule.h"
#include "common/port.h"
#include "common/version.h"
#include "common/sitesettings.h"
#include "common/util.h"

#include "httpd.h"
#include "http_protocol.h"

#include <utility>
#include <string>
#include <set>
#include <map>

#if APACHE_VERSION >= 20
#include "apr_strings.h"
#include "apr_tables.h"
#else
#define apr_table_get(table, name) ap_table_get(table, name)
#endif

#if APACHE_VERSION <= 20
#define ap_http_scheme(r) ap_http_method(r)
#endif

SitemapModule::SitemapModule() {
  inited_ = false;
}

SitemapModule::~SitemapModule() {
  pthread_mutex_destroy(&siteid_mutex_);
}

bool SitemapModule::Initialize() {
  Util::SetLogFileName("/var/log/google-sitemap-generator.log");

  Util::Log(EVENT_CRITICAL, "=== Google Sitemap Generator Apache Module [%s] ===",
            SITEMAP_VERSION1);

  std::string setting_file = SiteSettings::GetDefaultFilePath();
  SiteSettings settings;
  if (!settings.LoadFromFile(setting_file.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to load setting from file.");
    return false;
  }
  ApacheConfig::SetConfFilePath(settings.apache_conf().c_str());

  if (!settings.LoadWebserverConfig()) {
    Util::Log(EVENT_ERROR, "Failed to load webserver config.");
    return false;
  }
  if (!settings.LoadFromFile(setting_file.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to load setting from file.");
    return false;
  }

  // Init flags.
  Util::SetLogLevel(settings.logging_level());

  pthread_mutex_init(&siteid_mutex_, NULL);

  inited_ = BaseFilter::Initialize(SiteSettings::GetDefaultFilePath().c_str());

  return inited_;
}


int SitemapModule::Process(request_rec* req) {
  if (!inited_) return DECLINED;

  if (req->method_number != M_GET) {
    return DECLINED;
  }

  if (!BaseFilter::CheckStatusCode(req->status)) {
    Util::Log(EVENT_NORMAL, "Url [%s] is ignored, status: [%d]",
              req->unparsed_uri, req->status);
    return DECLINED;
  }

  UrlRecord record;

  // retrieve status code
  record.statuscode = req->status;

  // Determine which site this url belongs to.
  std::string site_id = MatchSite(req->server);
  if (site_id.length() == 0 || site_id.length() >= kMaxSiteIdLength) {
    return DECLINED;
  }
  strcpy(record.siteid, site_id.c_str());

  // Check whether it is an authenticated url.
  if (apr_table_get(req->headers_in, "Authorization") != NULL) {
    DLog(EVENT_NORMAL, "Url [%s] is ignored for Authorization",
		       req->unparsed_uri);
    return DECLINED;
  }
  if (apr_table_get(req->headers_in, "Proxy-Authorization") != NULL) {
    DLog(EVENT_NORMAL, "Url [%s] is ignored for Proxy-Authorization",
		       req->unparsed_uri);
    return DECLINED;
  }

  // copy http scheme
  int offset = 0;
  const char* p = ap_http_scheme(req);
  if (p != NULL) {
    offset = CopyString(record.host, offset, kMaxHostLength, p);
  } else {
    offset = CopyString(record.host, offset, kMaxHostLength, "http");
  }
  if (offset == -1) return DECLINED;
  offset = CopyString(record.host, offset, kMaxHostLength, "://");
  if (offset == -1) return DECLINED;

  // copy http host
  p = apr_table_get(req->headers_in, "Host");
  if (p == NULL && req->connection != NULL) {
    p = req->connection->local_ip;
  }
  offset = CopyString(record.host, offset, kMaxHostLength, p);
  if (offset == -1) return DECLINED;
  record.host[offset] = '\0';

  // get content-length http header
  p = apr_table_get(req->headers_out, "Content-Length");
  record.contentHashCode = p == NULL ? -1 : atoi(p);

  // get last-modified http header
  p = apr_table_get(req->headers_out, "Last-Modified");
  if (p == NULL || !BaseFilter::ParseTime(p, &record.last_modified)) {
    record.last_modified = -1;
  }

  // get last write time from static file
  record.last_filewrite = -1;
#if APACHE_VERSION >= 20
  if (req->finfo.protection != 0 && req->finfo.mtime > 0) {
    // static file or files like SHTML
    if (record.contentHashCode == req->finfo.size ||
        BaseFilter::TreatAsStatic(req->finfo.fname)) {
      // WARN: 10000000 is workable under apache 2.0,
      // although APR doc says mtime is in second
      record.last_filewrite = req->finfo.mtime / 1000000;
    }
  }
#else
  if (req->finfo.st_mode != 0 && req->finfo.st_mtime > 0) {
    // static file or files like SHTML
    if (record.contentHashCode == req->finfo.st_size ||
        BaseFilter::TreatAsStatic(req->filename)) {
      record.last_filewrite = req->finfo.st_mtime / 1000000;
    }
  }
#endif

  // get http url, including path and parameter (uri + args)
  if (req->unparsed_uri != NULL) {
    offset = CopyString(record.url, 0, kMaxUrlLength, req->unparsed_uri);
  } else {
    offset = CopyString(record.url, 0, kMaxUrlLength, req->uri);
    if (offset == -1) return DECLINED;

    // if it has args
    if (req->args != NULL) {
      offset = CopyString(record.url, offset, kMaxUrlLength, "?");
      if (offset == -1) return DECLINED;
      offset = CopyString(record.url, offset, kMaxUrlLength, req->args);
    }
  }
  if (offset == -1) return DECLINED;
  record.url[offset] = '\0';

  Util::Log(EVENT_NORMAL,
            "Record generated:[url:%s|host:%s|siteid:%s|content:%lld|"
            "lastmod:%ld|lastwrite:%ld|status:%d]",
            record.url, record.host, record.siteid, record.contentHashCode,
            record.last_modified, record.last_filewrite, record.statuscode);

  BaseFilter::Send(&record);

  return DECLINED;
}

std::string SitemapModule::MatchSite(server_rec* server) {
  bool found = false;
  std::string site_id;

  pthread_mutex_lock(&siteid_mutex_);
  std::map<void*, std::string>::iterator itr = siteids_.find(server);
  if (itr != siteids_.end()) {
    site_id = itr->second;
    found = true;
  }
  pthread_mutex_unlock(&siteid_mutex_);

  if (found) {
    return site_id;
  }

  // add server name to id
  site_id.append(server->server_hostname);

  std::set<std::pair<std::string, int> > vhosts;
  server_addr_rec *sar = server->addrs;
  while (sar != NULL) {
    vhosts.insert(make_pair(std::string(sar->virthost), sar->host_port));
    sar = sar->next;
  }
  std::set<std::pair<std::string, int> >::iterator vhostitr = vhosts.begin();
  for (; vhostitr != vhosts.end(); ++vhostitr) {
    site_id.append(",").append(vhostitr->first);

    // append the port to siteid
    char buffer[128];
    itoa(vhostitr->second, buffer);
    site_id.append(":").append(buffer);
  }

  bool matched = BaseFilter::MatchSite(site_id.c_str());
  Util::Log(EVENT_IMPORTANT, "Match site (%s) (%d)\n",
            site_id.c_str(), (matched ? "YES" : "NO"));
  if (!matched) site_id.clear();

  // Cache the site id
  pthread_mutex_lock(&siteid_mutex_);
  if (siteids_.size() < 100000) {
    siteids_[server] = site_id;
  } else {
    siteids_.clear();
    Util::Log(EVENT_CRITICAL, "SiteIds cache is cleared.");
  }
  pthread_mutex_unlock(&siteid_mutex_);

  return site_id;
}
