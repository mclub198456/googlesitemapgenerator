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


// Setting class for a site.
// In general, setting values for a site can be fit into one of the four
// categories. The first category is site meta data not specific to this
// program, including site id, site root directory, and etc. The second
// category is site runtime configurations, like max url number, which could
// be saved. The third category is sitemap settings and blog search ping
// service setting. The last part is url provider setting, including log parser
// setting, file scanner setting and webserver filter setting.
// Not all settings can be loaded from associated xml node. Some of values are
// loaded from webserver configuration directly. Please see the variables
// definition for details.

#ifndef COMMON_SITESETTING_H__
#define COMMON_SITESETTING_H__

#include <string>
#include <vector>

#include "common/url.h"
#include "common/urlreplacement.h"
#include "common/basesetting.h"
#include "common/websitemapsetting.h"
#include "common/newssitemapsetting.h"
#include "common/videositemapsetting.h"
#include "common/mobilesitemapsetting.h"
#include "common/codesearchsitemapsetting.h"
#include "common/blogsearchpingsetting.h"
#include "common/webserverfiltersetting.h"
#include "common/logparsersetting.h"
#include "common/filescannersetting.h"
#include "common/queryfield.h"

class TiXmlElement;

// This class is used to get/set site related settings
class SiteSetting : public BaseSetting {
 public:
  SiteSetting();
  explicit SiteSetting(const char *setting_name);
  virtual ~SiteSetting() {}

  // Reset the value to default values.
  virtual void ResetToDefault();

  // Load setting from xml_node_.
  virtual bool LoadSetting(TiXmlElement* element);

  // Convert values from UTF8 to system encoding.
  // Only values related to system file name are converted.
  // e.g. site physical path, log path, sitemap file names, and etc.
  bool ToSystemEncoding();

  bool LoadSettingForFilter();

  // Save settings to xml_node_.
  virtual TiXmlElement* SaveSetting();
  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  // Validate if current settings are valid settings.
  virtual bool Validate() const;

  // Getter/setter for setting values.
  // get/set site name
  const std::string& name() const { return name_; }
  void set_name(const std::string &name) {
    name_ = name;
  }

  // get/set enabled
  const bool enabled() const { return enabled_; }
  void set_enabled(bool enabled) {
    enabled_ = enabled;
  }

  // get/set max url number for this site
  const std::string site_id() const { return site_id_; }
  void set_site_id(std::string site_id) {
    site_id_ = site_id;
  }

  // get/set host url
  const Url& host_url() const { return host_url_; }
  void set_host_url(const Url& host_url) {
    host_url_ = host_url;
  }

  // get/set physical path of this site
  // physical_path is not saved to xml document.
  const std::string &physical_path() const { return physical_path_; }
  void set_physical_path(const std::string &physical_path) {
    physical_path_ = physical_path;
  }

  const std::string& log_path() const { return log_path_; }
  void set_log_path(const std::string& log_path) {
    log_path_ = log_path;
  }

  // get/set max url number for this site
  const int max_url_in_disk() const { return max_url_in_disk_; }
  void set_max_url_in_disk(int max_url_in_disk) {
    max_url_in_disk_ = max_url_in_disk;
  }

  // get/set max url in memory
  const int max_url_in_memory() const { return max_url_in_memory_; }
  void set_max_url_in_memory(int max_url_in_memory) {
    max_url_in_memory_ = max_url_in_memory;
  }

  // get/set UrlReplacement
  const UrlReplacements& url_replacements() const {
    return url_replacements_;
  }
  void set_url_replacements(const UrlReplacements& replaces) {
    url_replacements_ = replaces;
  }

  const IncludedQueryFields& included_queryfields() const {
    return included_queryfields_;
  }
  void set_included_queryfields(const IncludedQueryFields& fields) {
    included_queryfields_ = fields;
  }

  int max_url_life() const { return max_url_life_; }
  void set_max_url_life(int max_url_life) {
    max_url_life_ = max_url_life;
  }

  bool add_generator_info() const { return add_generator_info_; }
  void set_add_generator_info(bool add_generator_info) {
    add_generator_info_ = add_generator_info;
  }

  // get/set common sitemap setting
  const WebSitemapSetting& web_sitemap_setting() const {
    return web_sitemap_setting_;
  }
  void set_web_sitemap_setting(
    const WebSitemapSetting& web_sitemap_setting) {
    web_sitemap_setting_ = web_sitemap_setting;
  }
  WebSitemapSetting* mutable_web_sitemap_setting() {
    return &web_sitemap_setting_;
  }

  // get/set news sitemap setting
  const NewsSitemapSetting& news_sitemap_setting() const {
    return news_sitemap_setting_;
  }
  void set_news_sitemap_setting(
    const NewsSitemapSetting& news_sitemap_setting) {
    news_sitemap_setting_ = news_sitemap_setting;
  }

  // get/set video sitemap setting
  const VideoSitemapSetting& video_sitemap_setting() const {
    return video_sitemap_setting_;
  }
  void set_video_sitemap_setting(
      const VideoSitemapSetting& video_sitemap_setting) {
    video_sitemap_setting_ = video_sitemap_setting;
  }

  // get/set mobile sitemap setting
  const MobileSitemapSetting& mobile_sitemap_setting() const {
    return mobile_sitemap_setting_;
  }
  void set_mobile_sitemap_setting(
      const MobileSitemapSetting mobile_sitemap_setting) {
    mobile_sitemap_setting_ = mobile_sitemap_setting;
  }
 
  // get/set code search sitemap setting
  const CodeSearchSitemapSetting& codesearch_sitemap_setting() const {
    return codesearch_sitemap_setting_;
  }
  void set_codesearch_sitemap_setting(
      const CodeSearchSitemapSetting& codesearch_sitemap_setting) {
    codesearch_sitemap_setting_ = codesearch_sitemap_setting;
  }

  // get/set blog search ping setting
  const BlogSearchPingSetting& blogsearch_ping_setting() const {
    return blogsearch_ping_setting_;
  }
  void set_blogsearch_ping_setting(
      const BlogSearchPingSetting& blogsearch_ping_setting) {
    blogsearch_ping_setting_ = blogsearch_ping_setting;
  }


  const WebserverFilterSetting& webserver_filter_setting() const {
    return webserver_filter_setting_;
  }
  void set_webserver_filter_setting(
    const WebserverFilterSetting& webserver_filter_setting) {
    webserver_filter_setting_ = webserver_filter_setting;
  }

  const FileScannerSetting& file_scanner_setting() const {
    return file_scanner_setting_;
  }
  void set_file_scanner_setting(
    const FileScannerSetting& file_scanner_setting) {
    file_scanner_setting_ = file_scanner_setting;
  }

  const LogParserSetting& log_parser_setting() const {
    return log_parser_setting_;
  }
  void set_log_parser_setting(
    const LogParserSetting& log_parser_setting) {
    log_parser_setting_ = log_parser_setting;
  }

  bool Equals(const BaseSetting* another) const;

 private:
  // Whether this site is endabled or not.
  bool                        enabled_;

  // The key used to match data read from web server and setting file.
  // It can be used to identify site uniquely.
  std::string                 site_id_;

  // Max url number stored in database file.
  int                         max_url_in_disk_;

  // Max url number cached in memory.
  int                         max_url_in_memory_;

  // Max url life.
  // Url which is not accessed in the latest "max_url_life_" days is discarded.
  int                         max_url_life_;

  // Defines url replacement rules.
  UrlReplacements url_replacements_;

  // Included query fields.
  IncludedQueryFields included_queryfields_;

  // Sitemap settings.
  WebSitemapSetting           web_sitemap_setting_;
  NewsSitemapSetting          news_sitemap_setting_;
  VideoSitemapSetting         video_sitemap_setting_;
  MobileSitemapSetting        mobile_sitemap_setting_;
  CodeSearchSitemapSetting    codesearch_sitemap_setting_;

  BlogSearchPingSetting       blogsearch_ping_setting_;

  // Website main host name.
  Url                         host_url_;

  // Website local physical path (read from web server)
  std::string                 physical_path_;

  // Website log path, which could be a single file or a directory.
  std::string                 log_path_;

  // Website name (read from web server)
  std::string                 name_;

  // Whether generator info should be added to sitemap file file.
  bool                        add_generator_info_;

  // Url provider settings.
  WebserverFilterSetting      webserver_filter_setting_;
  FileScannerSetting          file_scanner_setting_;
  LogParserSetting            log_parser_setting_;
};

#endif  // COMMON_SITESETTING_H__
