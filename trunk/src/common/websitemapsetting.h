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


// Setting for Web Sitemap.
// Besides common settings inherited from SitemapSetting class, this class
// also includes a flag named as "included_in_robots_txt". If this flag is
// true, a line with "Sitemap: <sitemap_url>" will be added to corresponding
// site's robots.txt file. See http://sitemaps.org/protocol.php#informing

#ifndef COMMON_WEBSITEMAPSETTING_H__
#define COMMON_WEBSITEMAPSETTING_H__

#include <vector>

#include "common/sitemapsetting.h"
#include "common/url.h"

class WebSitemapSetting: public SitemapSetting {
 public:
  WebSitemapSetting();
  virtual ~WebSitemapSetting();

  // Overridden methods. See base class for explanation.
  virtual void ResetToDefault();
  virtual bool LoadSetting(TiXmlElement* element);
  virtual TiXmlElement* SaveSetting();
  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  // Getter/setter for settings values.
  bool included_in_robots_txt() const {
    return included_in_robots_txt_;
  }
  void set_included_in_robots_txt(bool included_in_robots_txt) {
    included_in_robots_txt_ = included_in_robots_txt;
  }

  bool Equals(const BaseSetting* another) const;

 private:
  // A flag indicating whether the sitemap should be submitted via robots.txt.
  // Default value is false.
  bool included_in_robots_txt_;
};

#endif  // COMMON_WEBSITEMAPSETTING_H__
