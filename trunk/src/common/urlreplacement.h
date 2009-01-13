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


// UrlReplacement represents configuration for url replacements.
// The corresponding xml elements look like:
//    <UrlReplacement find="/[a]" value="A" />
// This class provides a Load method to load setting from xml element.
// It also provides a function to load a list of UrlReplacement from a
// <UrlReplacements> xml element. This element should contain a list of child
// <UrlReplacement> elements.

#ifndef COMMON_URLREPLACEMENT_H__
#define COMMON_URLREPLACEMENT_H__

#include <string>
#include <vector>

#include "common/basesetting.h"
#include "common/listsetting.h"

class TiXmlElement;

class UrlReplacement : public BaseSetting {
 public:
  UrlReplacement();
  UrlReplacement(const std::string& find, const std::string& replace);

  virtual void ResetToDefault();

  virtual bool LoadSetting(TiXmlElement* element);

  virtual TiXmlElement* SaveSetting();

  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  virtual bool Validate() const;

  // Compare if two UrlReplacements are equal.
  bool Equals(const BaseSetting* another) const;

  // Getter/setter for setting values.
  const std::string& find() const { return find_; }
  void set_find(const std::string& find) { find_ = find; }

  const std::string& replace() const { return replace_; }
  void set_replace(const std::string& replace) { replace_ = replace; }

 private:
  std::string find_;
  std::string replace_;
};

// Declear UrlReplacements.
extern const char kUrlReplacementsName[];
extern const char kUrlReplacementName[];
typedef ListSetting<kUrlReplacementsName, kUrlReplacementName, UrlReplacement> UrlReplacements;

#endif  // COMMON_URLREPLACEMENT_H__


