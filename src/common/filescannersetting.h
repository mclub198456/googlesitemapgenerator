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


// Settings for file scanner.
// There are two simple setting values defines, "enabled" and "update_duration".
// Please note that, the root scanning directory is the website's root
// directory.

#ifndef COMMON_FILESCANNERSETTING_H__
#define COMMON_FILESCANNERSETTING_H__

#include "common/basesetting.h"


class FileScannerSetting : public BaseSetting {
 public:
  FileScannerSetting();
  virtual ~FileScannerSetting() {}

  // Overridden methods. Please see base class for explanation.
  virtual bool LoadSetting(TiXmlElement* element);
  virtual TiXmlElement* SaveSetting();
  virtual TiXmlElement* SaveSetting(const BaseSetting* global);
  virtual bool Validate() const;  
  virtual void ResetToDefault();

  // Getter/setter for setting values.
  bool enabled() const { return enabled_; }
  void set_enabled(const bool enabled) {
    enabled_ = enabled;
    SaveAttribute("enabled", enabled_);
  }
  
  int update_duration() const { return update_duration_; }
  void set_update_duration(const int update_duration) {
    update_duration_ = update_duration;
    SaveAttribute("update_duration", update_duration_);
  }

  bool Equals(const BaseSetting* another) const;

private:
  // Whether the file scanner should be enabled.
  bool enabled_;

  // Scanner running period in seconds.
  int update_duration_;
};

#endif  // COMMON_FILESCANNERSETTING_H__
