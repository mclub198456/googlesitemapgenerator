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

#include "common/queryfield.h"

const char kQueryFieldName[] = "QueryField";
const char kIncludedQueryFieldsName[] = "IncludedQueryFields";

QueryField::QueryField() : BaseSetting("QueryField") {
  ResetToDefault();
}

void QueryField::ResetToDefault() {
  name_.clear();
}

bool QueryField::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;
  LoadAttribute("name", name_);
  return true;
}

TiXmlElement* QueryField::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute("name", name_);
  return xml_node_;
}

TiXmlElement* QueryField::SaveSetting(const BaseSetting* global) {
  xml_node_ = NULL;
  const QueryField* another = (const QueryField*) global;
  SaveAttribute("name", name_, another->name_);
  return xml_node_;
}

bool QueryField::Validate() const {
  return name_.length() != 0;
}

bool QueryField::Equals(const BaseSetting* a) const {
  const QueryField* another = (const QueryField*) a;
  return name_ == another->name_;
}
