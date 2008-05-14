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


// SitemapSetting defines common settings for all kinds of sitemap.
// See http://sitemaps.org
// See the member variables' doc for more details.
// This class is not thread-safe.

#ifndef COMMON_SITEMAPSETTING_H__
#define COMMON_SITEMAPSETTING_H__

#include <string>
#include <vector>

#include "common/url.h"
#include "common/basesetting.h"

class TiXmlElement;

class SitemapSetting: public BaseSetting {
  friend class SiteSettings;
 public:
  SitemapSetting(const char * setting_name);
  virtual ~SitemapSetting();

  // Generate a sitemap name with given prefix.
  // The result is like ${prefix}_XXXXXXXX.xml, and 'X' is an alpha-num.
  static std::string GenerateSitemapName(const std::string& prefix);

  // All fields are set to common values according to sitemap protocol.
  // If there is some special requirements, this method should be overrided.
  // See the method body for details.
  virtual void ResetToDefault();

  // Load setting values from xml_node_.
  virtual bool LoadSetting();

  // Save setting values to xml_node_.
  virtual void SaveSetting();

  // Validate current setting values.
  virtual bool Validate() const;

  // Getter/setter for setting values.
  const bool enabled() const {return enabled_; }
  void set_enabled(const bool enabled) {
    enabled_ = enabled;
    SaveAttribute("enabled", enabled_);
  }

  const int update_duration() const {return update_duration_; }
  void set_update_duration(const int update_duration) {
    update_duration_ = update_duration;
    SaveAttribute("update_duration_in_seconds", update_duration_);
  }

  const time_t update_start_time() const {return update_start_time_; }
  void set_update_start_time(const int update_start_time) {
    update_start_time_ = update_start_time;
    SaveTimeAttribute("update_start_time", update_start_time_);
  }

  const bool compress() const {return compress_; }
  void set_compress(const bool compress) {
    compress_ = compress;
    SaveAttribute("compress", compress_);
  }

  const std::string &file_name() const {return file_name_; }
  void set_file_name(const std::string &file_name) {
    file_name_ = file_name;
    SaveAttribute("file_name", file_name_);
  }

  const int max_file_url_number() const {return max_file_url_number_; }
  void set_max_file_url_number(int max_file_url_number) {
    max_file_url_number_ = max_file_url_number;
    SaveAttribute("max_file_url_number", max_file_url_number_);
  }

  const int max_file_size() const {return max_file_size_; }
  void set_max_file_size(int max_file_size) {
    max_file_size_ = max_file_size;
    SaveAttribute("max_file_size", max_file_size_);
  }

  const std::vector<Url> &included_urls() const {
    return included_urls_;
  }
  void set_included_urls(const std::vector<Url> &included_urls) {
    included_urls_ = included_urls;
    SaveUrls("IncludedUrls", included_urls_);
  }

  const std::vector<Url> &excluded_urls() const {
    return excluded_urls_;
  }
  void set_excluded_urls(const std::vector<Url> &excluded_urls) {
    excluded_urls_ = excluded_urls;
    SaveUrls("ExcludedUrls", excluded_urls_);
  }
  
  const std::vector<Url> &notify_urls() const { return notify_urls_; }
  void set_notify_urls(const std::vector<Url> &notify_urls) {
    notify_urls_ = notify_urls;
    SaveUrls("NotifyUrls", notify_urls);
  }


 protected:
  // Setting enabled or not.
  bool                      enabled_;

  // Update duration in seconds.
  // For every "update_duration_" seconds, sitemap should be re-generated.
  int                       update_duration_;

  // Update start time in time_t format.
  // sitemap should be generated for the first time at this time.
  time_t                    update_start_time_;

  // Compress the sitemap file or not.
  bool                      compress_;

  // Sitemap file name. It should be a file name without path element.
  std::string               file_name_;

  // The maximum number of urls in a sitemap file
  int                       max_file_url_number_;

  // Maximum file size for a single sitemap.
  // If "compress_" is true, this value represents the size limitation before
  // compressing.
  int                       max_file_size_;

  // Include url patterns.
  std::vector<Url>          included_urls_;

  // Exclude url patterns.
  std::vector<Url>          excluded_urls_;

  // Urls we'll notify after generating a sitemap file
  // eg. http://www.google.com/webmasters/sitemaps/ping?sitemap=
  std::vector<Url>          notify_urls_;
};

#endif  // COMMON_SITEMAPSETTING_H__
