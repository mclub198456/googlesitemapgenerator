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


// SiteInfo contains the runtime information for a single site. It uses site-id
// to identifiy itself. Generally, it contains the runtime information of
// url providers, runtime information of all kinds of enabled sitemap services,
// used disk space, used memory size, and etc. For the concrete information,
// please investigate the data members.
// This class is not thread-safe.

#ifndef SITEMAPSERVICE_SITEINFO_H__
#define SITEMAPSERVICE_SITEINFO_H__

#include <string>

#include "common/basictypes.h"
#include "sitemapservice/baseruntimeinfo.h"
#include "sitemapservice/urlproviderinfo.h"
#include "sitemapservice/sitemapserviceinfo.h"
#include "sitemapservice/blogsearchpingserviceinfo.h"
#include "sitemapservice/webserverfilterinfo.h"

class SiteInfo : public BaseRuntimeInfo {
 public:
  // Constructor. Reset() is called in it.
  SiteInfo();

  // Empty destructor.
  virtual ~SiteInfo() {}

  // "site_id" is used to uniquely identify this SiteInfo.
  const std::string& site_id() const { return site_id_; }
  void set_site_id(const std::string& site_id) { site_id_ = site_id; }

  // "url_in_database" reprensents the number of URLs contains in database.
  int64 url_in_database() const { return url_in_database_; }
  void set_url_in_database(int64 url_in_database) {
    url_in_database_ = url_in_database;
  }

  // "url_in_tempfile" represents the number of URLs in all temporary files.
  // A temporary file contains recent URL visiting records which haven't merged
  // into database yet.
  int64 url_in_tempfile() const { return url_in_tempfile_; }
  void set_url_in_tempfile(int64 url_in_tempfile) {
    url_in_tempfile_ = url_in_tempfile;
  }

  // "url_in_memory" represents the number of URLs in memory.
  int64 url_in_memory() const { return url_in_memory_; }
  void set_url_in_memory(int64 url_in_memory) {
    url_in_memory_ = url_in_memory;
  }

  // "host_name" represents the site host name used in sitemap.
  const std::string& host_name() const { return host_name_; }
  void set_host_name(const std::string& host_name) { host_name_ = host_name; }

  // Runtime information for log parser.
  UrlProviderInfo* logparser_info() { return &logparser_info_; }
  void set_logparser_info(const UrlProviderInfo& logparser_info) {
    logparser_info_ = logparser_info;
  }

  // Runtime information for file scanner.
  UrlProviderInfo* filescanner_info() { return &filescanner_info_; }
  void set_filescanner_info(const UrlProviderInfo& filescanner_info) {
    filescanner_info_ = filescanner_info;
  }

  // Runtime information for webserver filter.
  WebServerFilterInfo* webserverfilter_info() { return &webserverfilter_info_; }
  void set_webserverfilter_info(const WebServerFilterInfo& webserverfilter_info) {
    webserverfilter_info_ = webserverfilter_info;
  }

  // Runtime information for web sitemap servce.
  SitemapServiceInfo* web_sitemapservice_info() {
    return &web_sitemapservice_info_;
  }
  void set_web_sitemapservice_info(const SitemapServiceInfo& info) {
    web_sitemapservice_info_ = info;
  }
  
  // Runtime information for news sitemap servce.
  SitemapServiceInfo* news_sitemapservice_info() {
    return &news_sitemapservice_info_;
  }
  void set_news_sitemapservice_info(const SitemapServiceInfo& info) {
    news_sitemapservice_info_ = info;
  }
  
  // Runtime information for video sitemap servce.
  SitemapServiceInfo* video_sitemapservice_info() {
    return &video_sitemapservice_info_;
  }
  void set_video_sitemapservice_info(const SitemapServiceInfo& info) {
    video_sitemapservice_info_ = info;
  }
  
  // Runtime information for mobile sitemap servce.
  SitemapServiceInfo* mobile_sitemapservice_info() {
    return &mobile_sitemapservice_info_;
  }
  void set_mobile_sitemapservice_info(const SitemapServiceInfo& info) {
    mobile_sitemapservice_info_ = info;
  }

  // Runtime information for code-search sitemap servce.
  SitemapServiceInfo* codesearch_sitemapservice_info() {
    return &codesearch_sitemapservice_info_;
  }
  void set_codesearch_sitemapservice_info(const SitemapServiceInfo& info) {
    codesearch_sitemapservice_info_ = info;
  }

  // Runtime information for blog-search ping servce.
  BlogSearchPingServiceInfo* blogsearch_pingservice_info() {
    return &blogsearch_pingservice_info_;
  }
  void set_blogsearch_pingservice_info(const BlogSearchPingServiceInfo& info) {
    blogsearch_pingservice_info_ = info;
  }

  // Represents memory used by this site to cache URL visiting records.
  // Note, this value is an approximate value and doesn't include the memory
  // used by sitemap service and url providers.
  int64 memory_used() const { return memory_used_; }
  void set_memory_used(int64 memory_used) { memory_used_ = memory_used; }

  // Represents disk used by this site to store visiting records on disk.
  // This value including both the permanent data base, as well as temporary
  // data files.
  int64 disk_used() const { return disk_used_; }
  void set_disk_used(int64 disk_used) { disk_used_ = disk_used; }

  // Save this site info into given XML element.
  // The element name is set as "SiteInfo". Except URL provider runtime-info
  // and sitemap service runtime-info, all the other info is saved as simple
  // attributes of this xml element.
  virtual bool Save(TiXmlElement* element);

  // Reset this SiteInfo to default (initial) values.
  // NOTE, site_id_ is not reset.
  virtual void Reset();

 private:
  std::string site_id_;

  int64 url_in_database_;
  int64 url_in_tempfile_;
  int64 url_in_memory_;

  std::string host_name_;

  int64 memory_used_;
  int64 disk_used_;

  // Sitemap service runtime information.
  SitemapServiceInfo web_sitemapservice_info_;
  SitemapServiceInfo news_sitemapservice_info_;
  SitemapServiceInfo video_sitemapservice_info_;
  SitemapServiceInfo mobile_sitemapservice_info_;
  SitemapServiceInfo codesearch_sitemapservice_info_;
  BlogSearchPingServiceInfo blogsearch_pingservice_info_;

  // Url provider runtime information.
  UrlProviderInfo logparser_info_;
  UrlProviderInfo filescanner_info_;
  WebServerFilterInfo webserverfilter_info_;
};

#endif  // SITEMAPSERVICE_SITEINFO_H__
