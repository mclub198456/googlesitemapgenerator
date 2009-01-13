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


#include "common/webserverfiltersetting.h"

// -- implementation for WebServerPluginSetting
WebserverFilterSetting::WebserverFilterSetting()
: BaseSetting("WebserverFilterSetting") {
  ResetToDefault();
}

void WebserverFilterSetting::ResetToDefault() {
  enabled_ = true;
}

bool WebserverFilterSetting::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;
  LoadAttribute("enabled", enabled_);
  return true;
}

TiXmlElement* WebserverFilterSetting::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute("enabled", enabled_);
  return xml_node_;
}

TiXmlElement* WebserverFilterSetting::SaveSetting(const BaseSetting* g) {
  xml_node_ = NULL;
  const WebserverFilterSetting* another = (const WebserverFilterSetting*) g;
  SaveAttribute("enabled", enabled_, another->enabled_);
  return xml_node_;
}

bool WebserverFilterSetting::Equals(const BaseSetting* a) const {
  const WebserverFilterSetting* another = (const WebserverFilterSetting*) a;
  return enabled_ == another->enabled_;
}

