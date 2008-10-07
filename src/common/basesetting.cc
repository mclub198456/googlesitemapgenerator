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


#include "common/basesetting.h"

#include <string>
#include <vector>

#include "common/timesupport.h"
#include "common/port.h"
#include "common/util.h"

#include "third_party/tinyxml/tinyxml.h"

const char * BaseSetting::kTimeFormat = "%Y-%m-%d %H:%M:%S";

BaseSetting::BaseSetting(const char * setting_name) {
  setting_name_ = setting_name;
};

BaseSetting::~BaseSetting() {
};

bool BaseSetting::LoadFromParent(TiXmlNode *root) {
  if (root == NULL) return false;

  // Find setting element node first.
  xml_node_ = root->FirstChildElement(setting_name_.c_str());

  // Ignore this element.
  if (xml_node_ == NULL) {
    return true;
  }

  // Load setting values from xml_node_.
  return LoadSetting(xml_node_);
}

void BaseSetting::LoadAttribute(const char *attribute_name,
                                std::string &value) {
  const char *attribute = xml_node_->Attribute(attribute_name);
  if (attribute != NULL)
    value = attribute;
}

void BaseSetting::LoadAttribute(const char *attribute_name, int &value) {
  const char *attribute = xml_node_->Attribute(attribute_name);
  if (attribute != NULL)
    value = atoi(attribute);
}

void BaseSetting::LoadAttribute(const char *attribute_name, bool &value) {
  const char *attribute = xml_node_->Attribute(attribute_name);
  if (attribute != NULL)
    value = stricmp(attribute, "true") == 0 ? true: false;
}

void BaseSetting::LoadTimeAttribute(const char *attribute_name, time_t &value) {
  // Contruct a tm struct.
  tm tm;
  memset(&tm, 0, sizeof(tm));

  // A value less than zero to have the C run-time library code compute whether
  // standard time or daylight saving time is in effect when mk_time is called.
  tm.tm_isdst = -1;

  // Parse and save time value.
  const char *attribute = xml_node_->Attribute(attribute_name);
  if (attribute != NULL && strptime(attribute, kTimeFormat, &tm))
    value = mktime(&tm);
}

void BaseSetting::LoadAttribute(const char *attribute_name, Url &value) {
  const char *attribute = xml_node_->Attribute(attribute_name);
  if (attribute != NULL)
    value.Parse(attribute);
}

void BaseSetting::LoadUrls(const char *list_name, std::vector<Url> &urls) {

  if (list_name == NULL || xml_node_ == NULL) return;

  // Finds the element containing a list of <Url> element.
  const TiXmlElement *list_element = xml_node_->FirstChildElement(list_name);
  if (list_element != NULL) {
    urls.clear();

    // Load all child <Url> elements.
    for (const TiXmlElement* itr = list_element->FirstChildElement("Url");
         itr; itr = itr->NextSiblingElement()) {

      Url url;
      const char *attribute = itr->Attribute("value");
      if (attribute != NULL) {
        url.Parse(attribute);

        // Only load valid and non-duplicated url.
        if (url.Validate()) {
          // Check if attribute is already in urls
          std::vector<Url>::const_iterator itr = urls.begin();
          bool found = false;

          while (itr != urls.end()) {
            if (*itr == url) {
              found = true;
              break;
            }
            itr++;
          }

          // Only add this new url if it doesn't exist in urls.
          if (!found)
            urls.push_back(url);
        }
      }
    }
  }
}

void BaseSetting::SaveAttribute(const char *attribute_name,
                                const std::string &value) {
  // Only saves attribute if it's not empty.
  if (!value.empty())
    xml_node_->SetAttribute(attribute_name, value.c_str());
}

void BaseSetting::SaveAttribute(const char *attribute_name, const int value) {
  xml_node_->SetAttribute(attribute_name, value);
}

void BaseSetting::SaveAttribute(const char *attribute_name, const bool value) {
  // Save boolean value as a string.
  xml_node_->SetAttribute(attribute_name, value ? "true" : "false");
}

void BaseSetting::SaveTimeAttribute(const char *attribute_name,
                                    const time_t &value) {
  // Format time and save it as a string.
  char attribute[256];
  strftime(attribute, sizeof(attribute), kTimeFormat, localtime(&value));
  xml_node_->SetAttribute(attribute_name, attribute);
}

void BaseSetting::SaveAttribute(const char *attribute_name, const Url &value) {
  // Only save valid Url
  if (value.Validate())
    xml_node_->SetAttribute(attribute_name, value.url().c_str());
}

void BaseSetting::SaveUrls(const char *list_name,
                           const std::vector<Url> &urls) {

  if (xml_node_ == NULL || list_name == NULL) return;

  TiXmlElement *list_element = xml_node_->FirstChildElement(list_name);
  if (list_element == NULL) {
    list_element = new TiXmlElement(list_name);
    if (list_element == NULL)
      return;

    xml_node_->LinkEndChild(list_element);
  } else  {
    list_element->Clear();
  }

  // Enumerate value in urls
  for (std::vector<Url>::const_iterator itr = urls.begin();
    itr != urls.end(); itr++) {
    // Only save valid url
    if (itr->Validate()) {
      TiXmlElement* child_element = new TiXmlElement("Url");
      if (child_element == NULL)
        break;

      // Save attribute, and link it to parent_element
      child_element->SetAttribute("value", itr->url().c_str());
      list_element->LinkEndChild(child_element);
    }
  }
}

bool BaseSetting::UrlsEquals(const std::vector<Url>& a,
                             const std::vector<Url>& b) const {
  if (a.size() != b.size()) {
    return false;
  }

  int size = static_cast<int>(a.size());
  for (int i = 0; i < size; ++i) {
    if (a[i].url() != b[i].url()) {
      return false;
    }
  }

  return true;
}

void BaseSetting::SaveChild(BaseSetting* child) {
  xml_node_->LinkEndChild(child->SaveSetting());
}

void BaseSetting::SaveAttribute(const char *attribute_name,
                                const std::string &value,
                                const std::string &global) {
  if (value == global) return;
  if (xml_node_ == NULL) xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute(attribute_name, value);
}

void BaseSetting::SaveAttribute(const char *attribute_name,
    const int value, const int global) {
  if (value == global) return;
  if (xml_node_ == NULL) xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute(attribute_name, value);
}

void BaseSetting::SaveAttribute(const char *attribute_name,
                                const bool value, const bool global) {
  if (value == global) return;
  if (xml_node_ == NULL) xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute(attribute_name, value);
}

void BaseSetting::SaveTimeAttribute(const char *attribute_name,
                                    const time_t &value, const time_t &global) {
  if (value == global) return;
  if (xml_node_ == NULL) xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveTimeAttribute(attribute_name, value);
}

void BaseSetting::SaveAttribute(const char *attribute_name,
                                const Url &value, const Url& global) {
  if (value.url() == global.url()) return;
  if (xml_node_ == NULL) xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute(attribute_name, value);
}

void BaseSetting::SaveChild(BaseSetting* child, const BaseSetting* global) {
  TiXmlElement* element = child->SaveSetting(global);
  if (element == NULL) return;

  if (xml_node_ == NULL) {
    xml_node_ = new TiXmlElement(setting_name_.c_str());
  }
  xml_node_->LinkEndChild(element);
}



