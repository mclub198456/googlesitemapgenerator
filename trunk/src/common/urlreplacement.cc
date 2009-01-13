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


#include "common/urlreplacement.h"

#include "third_party/tinyxml/tinyxml.h"

UrlReplacement::UrlReplacement() : BaseSetting("UrlReplacement") {
  ResetToDefault();
}

UrlReplacement::UrlReplacement(const std::string& find,
                               const std::string& replace)
 : BaseSetting("UrlReplacement") {
  ResetToDefault();

  find_ = find;
  replace_ = replace;
}

void UrlReplacement::ResetToDefault() {
  find_.clear();
  replace_.clear();
}

bool UrlReplacement::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;
  LoadAttribute("find", find_);
  LoadAttribute("replace", replace_);
  return true;
}

TiXmlElement* UrlReplacement::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute("find", find_);
  SaveAttribute("replace", replace_);
  return xml_node_;
}

TiXmlElement* UrlReplacement::SaveSetting(const BaseSetting* global) {
  xml_node_ = NULL;
  const UrlReplacement* another = (const UrlReplacement*) global;
  SaveAttribute("find", find_, another->find_);
  SaveAttribute("replace", replace_, another->replace_);
  return xml_node_;
}

bool UrlReplacement::Validate() const {
  if (find_.length() == 0)
    return false;

  return true;
}

bool UrlReplacement::Equals(const BaseSetting* another) const {
  const UrlReplacement* right = (const UrlReplacement*) another;
  return right->find_ == find_ && right->replace_ == replace_;
}

const char kUrlReplacementsName[] = "UrlReplacements";
const char kUrlReplacementName[] = "UrlReplacement";
