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


// Setting for News Sitemap.
// See: http://www.google.com/support/webmasters/bin/topic.py?topic=10078
// Besides common settings inherited from SitemapSetting class, this class
// also includes a integer value "expire_duration". Only the URLs whose
// publication date is in the latest "expire_duration" seconds will be included
// in news sitemap file.

#ifndef COMMON_NEWSSITEMAPSETTING_H__
#define COMMON_NEWSSITEMAPSETTING_H__

#include <string>
#include <vector>

#include "common/sitemapsetting.h"
#include "common/url.h"

class NewsSitemapSetting: public SitemapSetting {
 public:
  NewsSitemapSetting();
  virtual ~NewsSitemapSetting();

  // Overridden methods. See base class for explanation.
  virtual void ResetToDefault();
  virtual bool Validate() const;

  // Load/Save setting from xml_node_.
  virtual bool LoadSetting(TiXmlElement* element);
  virtual TiXmlElement* SaveSetting();
  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  // Getter/setter for settings values.
  const int expire_duration() const { return expire_duration_; }
  void set_expire_duration(const int expire_duration) {
    expire_duration_ = expire_duration;
    SaveAttribute("expire_duration_in_seconds", expire_duration_);
  }

  bool Equals(const BaseSetting* another) const;

 protected:
  // News expire duration in seconds.
  int                       expire_duration_;
};

#endif  // COMMON_NEWSSITEMAPSETTING_H__

