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


// BlogSearchPingSetting contains configurations for BlogSearchPingService.
// A typical BlogSearchPingSetting xml element looks like:
//  <BlogSearchPingSetting enabled="false" update_duration_in_seconds="1800"
//    blog_name="MyBlog" blog_url="http://exmpale.com/post1.htm"
//    blog_changes_url="httpd://example.com/blog.rss" >
//    <IncludedUrls />
//    <ExcludedUrls />
//  </BlogSearchPingSetting>
// If "blog_name", "blog_url", and "blog_changes_url" are provided, they will
// be used to call the ping API. Otherwise, urls specified by "IncludedUrls"
// and "ExcludedUrls" will be used to call ping API. In the latter case, no 
// "name" or "changes_url" is provided to ping API.
// For more about Blog Search Ping service, see:
// http://www.google.com/help/blogsearch/pinging_API.html

#ifndef COMMON_BLOGSEARCHPINGSETTING_H__
#define COMMON_BLOGSEARCHPINGSETTING_H__

#include <string>
#include <vector>

#include "common/url.h"
#include "common/urlsetting.h"
#include "common/basesetting.h"

class BlogSearchPingSetting : public BaseSetting {
 public:
  // Ping service server URL.
  static const char* kPingServiceUrl;

  BlogSearchPingSetting();
  virtual ~BlogSearchPingSetting();

  // Methods inherited from BaseSetting class.
  virtual void ResetToDefault();
  virtual bool LoadSetting(TiXmlElement* element);
  virtual TiXmlElement* SaveSetting();
  virtual TiXmlElement* SaveSetting(const BaseSetting* global);
  virtual bool Validate() const;

  // Getter/setter for setting values.
  const bool enabled() const {return enabled_; }
  void set_enabled(const bool enabled) {
    enabled_ = enabled;
  }

  const int update_duration() const {return update_duration_; }
  void set_update_duration(const int update_duration) {
    update_duration_ = update_duration;
  }

  const std::string &blog_name() const {return blog_name_; }
  void set_blog_name(const std::string &blog_name) {
    blog_name_ = blog_name;
  }

  const std::string &blog_url() const {return blog_url_; }
  void set_blog_url(const std::string &blog_url) {
    blog_url_ = blog_url;
  }

  const std::string &blog_changes_url() const {return blog_changes_url_; }
  void set_blog_changes_url(const std::string &blog_changes_url) {
    blog_changes_url_ = blog_changes_url;
  }

  const IncludedUrls &included_urls() const {
    return included_urls_;
  }
  void set_included_urls(const IncludedUrls &included_urls) {
    included_urls_ = included_urls;
  }

  const ExcludedUrls &excluded_urls() const {
    return excluded_urls_;
  }
  void set_excluded_urls(const ExcludedUrls &excluded_urls) {
    excluded_urls_ = excluded_urls;
  }

  bool Equals(const BaseSetting* another) const;

 private:
  // Setting enabled or not.
  bool                      enabled_;

  // Service running period in seconds.
  // Every "update_duration_" seconds, the service will check all URLs to see
  // whether there is a new blog post.
  int                       update_duration_;

  // These three fields are used for sites only hosting one blog.
  // Note, they don't appear in setting UI.
  // Advanced users could edit these values in xml file manually.
  std::string               blog_name_;
  std::string               blog_url_;
  std::string               blog_changes_url_;

  // Following two fields are used for sites hosting many blogs.
  // If blog_url_ is not empty, these two fields are ignored.
  IncludedUrls         included_urls_;
  ExcludedUrls          excluded_urls_;
};

#endif  // COMMON_BLOGSEARCHPINGSETTING_H__
