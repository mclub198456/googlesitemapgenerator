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


#include "common/filescannersetting.h"

FileScannerSetting::FileScannerSetting()
: BaseSetting("FileScannerSetting") {
  ResetToDefault();
}

bool FileScannerSetting::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;
  LoadAttribute("enabled", enabled_);
  LoadAttribute("update_duration_in_seconds", update_duration_);

  return true;
}

TiXmlElement* FileScannerSetting::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute("enabled", enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_);
  return xml_node_;
}

TiXmlElement* FileScannerSetting::SaveSetting(const BaseSetting* global) {
  xml_node_ = NULL;
  const FileScannerSetting* another = (const FileScannerSetting*) global;
  SaveAttribute("enabled", enabled_, another->enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_,
    another->update_duration_);
  return xml_node_;
}

bool FileScannerSetting::Validate() const {
  if (update_duration_ <= 0) return false;

  return true;
}

void FileScannerSetting::ResetToDefault() {
  enabled_ = false;
  update_duration_ = 24 * 3600;
}

bool FileScannerSetting::Equals(const BaseSetting* a) const {
  const FileScannerSetting* another = (const FileScannerSetting*) a;
  return enabled_ == another->enabled_ &&
    update_duration_ == another->update_duration_;
}


