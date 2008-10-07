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


#include "common/urlsetting.h"

#include "third_party/tinyxml/tinyxml.h"

UrlSetting::UrlSetting() : BaseSetting("Url") {
  ResetToDefault();
}

UrlSetting::UrlSetting(const Url& value)
 : BaseSetting("Url") {
  ResetToDefault();

  enabled_ = true;
  value_ = value;
}

UrlSetting::UrlSetting(const Url& value, bool enabled)
 : BaseSetting("Url") {
  ResetToDefault();

  enabled_ = enabled;
  value_ = value;
}

void UrlSetting::ResetToDefault() {
  value_ = Url();
  enabled_ = true;
}

bool UrlSetting::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;
  LoadAttribute("enabled", enabled_);
  LoadAttribute("value", value_);
  return true;
}

TiXmlElement* UrlSetting::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute("enabled", enabled_);
  SaveAttribute("value", value_);
  return xml_node_;
}

TiXmlElement* UrlSetting::SaveSetting(const BaseSetting* global) {
  xml_node_ = NULL;
  const UrlSetting* another = (const UrlSetting*) global;
  SaveAttribute("value", value_, another->value_);
  SaveAttribute("enabled", enabled_, another->enabled_);
  return xml_node_;
}

bool UrlSetting::Validate() const {
  return value_.Validate();
}

bool UrlSetting::Equals(const BaseSetting* right) const {
  const UrlSetting* another = (const UrlSetting*) right;
  return enabled_ == another->enabled_ && value_.url() == another->value_.url();
}

std::vector<Url> UrlSetting::GetEnabledUrls(const std::vector<UrlSetting>& settings) {
  std::vector<Url> urls;
  for (int i = 0; i < (int) settings.size(); ++i) {
    if (settings[i].enabled()) {
      urls.push_back(settings[i].value());
    }
  }
  return urls;
}

const char kIncludedUrlsName[] = "IncludedUrls";
const char kExcludedUrlsName[] = "ExcludedUrls";
const char kNotifyUrlsName[] = "NotifyUrls";
const char kUrlName[] = "Url";

