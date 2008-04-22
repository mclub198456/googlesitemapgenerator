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


#include "common/urlreplacement.h"

#include "third_party/tinyxml/tinyxml.h"

void UrlReplacement::Load(const TiXmlElement *root) {
  const char *attribute = root->Attribute("find");
  if (attribute != NULL)
    find_ = attribute;

  attribute = root->Attribute("replace");
  if (attribute != NULL)
    replace_ = attribute;
}

bool UrlReplacement::Validate() const {
  if (find_.length() == 0)
    return false;

  return true;
}

void UrlReplacement::LoadUrls(const TiXmlElement *root,
                              std::vector<UrlReplacement> &url_replacements) {
  const TiXmlElement* list_element = root->FirstChildElement("UrlReplacements");
  if (list_element != NULL) {
    // Clear existing content first.
    url_replacements.clear();

    for (const TiXmlElement* itr =
      list_element->FirstChildElement("UrlReplacement");
      itr; itr = itr->NextSiblingElement()) {
      UrlReplacement url_replacement;
      url_replacement.Load(itr);
      url_replacements.push_back(url_replacement);
    }
  }
}

void UrlReplacement::SaveUrls(TiXmlElement *root,
                              const std::vector<UrlReplacement> &value_list) {
  // Check parameters.
  if (root == NULL)
    return;

  TiXmlElement* list_element = root->FirstChildElement("UrlReplacements");
  if (list_element == NULL) {
    list_element = new TiXmlElement("UrlReplacements");
    if (list_element == NULL)
      return;

    root->LinkEndChild(list_element);
  } else {
    list_element->Clear();
  }

  // Enumerate value in urls
  for (std::vector<UrlReplacement>::const_iterator itr = value_list.begin();
    itr != value_list.end(); itr++) {
    if (itr->Validate()) {
      TiXmlElement* child_element = new TiXmlElement("UrlReplacement");
      if (child_element == NULL)
        break;

      // Save attribute, and link it to parent_element
      child_element->SetAttribute("find", itr->find().c_str());
      child_element->SetAttribute("replace", itr->replace().c_str());
      list_element->LinkEndChild(child_element);
    }
  }
}

bool UrlReplacement::operator == (const UrlReplacement & right) const {
  return right.find() == find_ && right.replace() == replace_;
}
