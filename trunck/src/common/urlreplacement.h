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

class TiXmlElement;

class UrlReplacement {
 public:
  UrlReplacement() {}

  UrlReplacement(const char *find, const char *replace) {
    find_ = find;
    replace_ = replace;
  }

  // Load UrlReplacement fom XML setting.
  // This element should be a <UrlReplacement> element.
  void Load(const TiXmlElement *root);

  bool Validate() const;

  // Load UrlReplacement vector from XML setting.
  // The given "root" should contains a <UrlReplacements> element, which have
  // a list of <UrlReplacement> child elements.
  static void LoadUrls(const TiXmlElement *root,
                       std::vector<UrlReplacement> &url_replacements);

  // Save UrlReplacment vector to an XML element.
  // This is a reverse method of "LoadUrls"
  static void SaveUrls(TiXmlElement *root,
                       const std::vector<UrlReplacement> &value_list);

  // Used in SaveUrls to compare if two UrlReplacements are equal.
  bool operator == (const UrlReplacement &right) const;

  // Getter/setter for setting values.
  const std::string& find() const { return find_; }
  void set_find(const std::string& find) { find_ = find; }

  const std::string& replace() const { return replace_; }
  void set_replace(const std::string& replace) { replace_ = replace; }

 private:
  std::string find_;
  std::string replace_;
};

#endif  // COMMON_URLREPLACEMENT_H__

