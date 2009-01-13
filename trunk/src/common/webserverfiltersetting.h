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


#ifndef COMMON_WEBSERVERFILTERSETTING_H__
#define COMMON_WEBSERVERFILTERSETTING_H__

// Settings for webserver filter.
// There is only one simple setting value defined, which is "enabled" flag.
// This flag indicates whether request/response information should be sent
// to service side.

#include "common/basesetting.h"

class WebserverFilterSetting : public BaseSetting {
 public:
  WebserverFilterSetting();
  virtual ~WebserverFilterSetting() {}

  // Overridden methods. Please see base class for explanation.
  virtual bool LoadSetting(TiXmlElement* element);

  virtual TiXmlElement* SaveSetting();

  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  virtual bool Validate() const { return true; }

  virtual void ResetToDefault();
  
  // Getter/setter for setting values.
  bool enabled() const { return enabled_; }
  void set_enabled(const bool enabled) {
    enabled_ = enabled;
    SaveAttribute("enabled", enabled_);
  }

  bool Equals(const BaseSetting* another) const;

 protected:
  // This flag indicates whether request/response information should be sent.
  bool enabled_;
};


#endif  // COMMON_WEBSERVERFILTERSETTING_H__

