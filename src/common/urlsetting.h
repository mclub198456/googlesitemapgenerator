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


// UrlSetting represents configuration for yrk,
// The corresponding xml elements look like:
//    <Url value="/A" />
// This class provides a Load method to load setting from xml element.

#ifndef COMMON_URLSETTING_H__
#define COMMON_URLSETTING_H__

#include <string>
#include <vector>

#include "common/basesetting.h"
#include "common/listsetting.h"
#include "common/url.h"

class TiXmlElement;

class UrlSetting : public BaseSetting {
 public:
  UrlSetting();
  UrlSetting(const Url& value);
  UrlSetting(const Url& value, bool enabled);

  virtual void ResetToDefault();

  virtual bool LoadSetting(TiXmlElement* element);

  virtual TiXmlElement* SaveSetting();

  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  virtual bool Validate() const;

  bool Equals(const BaseSetting* right) const;

  // Getter/setter for setting values.
  const Url& value() const { return value_; }
  void set_value(const Url& value) { value_ = value; }

  bool enabled() const { return enabled_; }
  void set_enabled(bool enabled) { enabled_ = enabled; }

  static std::vector<Url> GetEnabledUrls(const std::vector<UrlSetting>& url_settings);

 private:
  Url value_;

  bool enabled_;
};

// Declare name of <Url> element.
extern const char kUrlName[];

// Declare <IncludedUrls> element.
extern const char kIncludedUrlsName[];
typedef ListSetting<kIncludedUrlsName, kUrlName, UrlSetting> IncludedUrls;

// Declare <ExcludedUrls> element.
extern const char kExcludedUrlsName[];
typedef ListSetting<kExcludedUrlsName, kUrlName, UrlSetting> ExcludedUrls;

// Declare <NotifyUrls> element.
extern const char kNotifyUrlsName[];
typedef ListSetting<kNotifyUrlsName, kUrlName, UrlSetting> NotifyUrls;

#endif  // COMMON_URLSETTING_H__


