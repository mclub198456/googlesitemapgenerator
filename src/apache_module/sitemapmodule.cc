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

#include <utility>
#include <string>
#include <set>
#include <map>

#include "httpd.h"
#include "http_protocol.h"

#include "common/port.h"
#include "common/version.h"
#include "common/settingmanager.h"
#include "common/util.h"
#include "common/logger.h"

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
  // Init application dir.
  std::string dir = Util::ReadApplicationDir();
  if (dir.length() == 0) {
    Logger::Log(EVENT_ERROR, "Failed to read application dir.");
    return false;
  }
  Util::SetApplicationDir(dir);

  // Set log file path.
  std::string log_file(dir);
  log_file.append("/log/google-sitemap-generator.log");
  Logger::SetLogFileName(log_file);

  Logger::Log(EVENT_CRITICAL, "=== Google Sitemap Generator Apache Module [%s] ===",
            SITEMAP_VERSION1);

  // Initialize setting manager.
  SettingManager* setting_manager = SettingManager::default_instance();
  setting_manager->Initialize(SettingManager::GetDefaultFilePath());
  SiteSettings settings;
  if (!setting_manager->LoadApplicationSetting(&settings)) {
    Logger::Log(EVENT_ERROR, "Failed to load app setting from file.");
    return false;
  }
  ApacheConfig::SetConfFilePath(settings.apache_conf().c_str());

  // Init flags.
  Logger::SetLogLevel(settings.logging_level());

  pthread_mutex_init(&siteid_mutex_, NULL);

  inited_ = BaseFilter::Initialize(settings);

  return inited_;
}


int SitemapModule::Process(request_rec* req) {
  if (!inited_) return DECLINED;

  if (req->method_number != M_GET) {
    return DECLINED;
  }

  if (!BaseFilter::CheckStatusCode(req->status)) {
    Logger::Log(EVENT_NORMAL, "Url [%s] is ignored, status: [%d]",
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
  if (p != NULL && strcmp(p, "http") != 0) {
    return DECLINED;
  } 
  offset = CopyString(record.host, offset, kMaxHostLength, "http://");

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

  Logger::Log(EVENT_NORMAL,
              "url:%s; host:%s; siteid:%s; content:%lld; "
              "lastmod:%ld; lastwrite:%ld; status:%d;",
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
  Logger::Log(EVENT_NORMAL, "Match site (%s) (%s) [%d] [%d] [%d].",
            site_id.c_str(), (matched ? "YES" : "NO"), this, server, getpid());
  if (!matched) site_id.clear();

  // Cache the site id
  pthread_mutex_lock(&siteid_mutex_);
  if (siteids_.size() < 100000) {
    siteids_[server] = site_id;
  } else {
    siteids_.clear();
    Logger::Log(EVENT_CRITICAL, "SiteIds cache is cleared.");
  }
  pthread_mutex_unlock(&siteid_mutex_);

  return site_id;
}
