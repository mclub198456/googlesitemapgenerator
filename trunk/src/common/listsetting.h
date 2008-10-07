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

// ListSetting is a template class for list based setting element classes.
// The list setting should be looks like:
// <ListName>
//   <ItemName />
//   <!-- other child items -->
// </ListName>
// This template implements all the methods required by a setting element.

#ifndef COMMON_LISTSETTING_H__
#define COMMON_LISTSETTING_H__

#include <vector>
#include "common/basesetting.h"

// Template params:
// "Name": name of list setting element.
// "ItemName": name of child setting element.
// "ItemType": type of child setting element.
template<const char* Name, const char* ItemName, typename ItemType>
class ListSetting : public BaseSetting {
public:
  ListSetting() : BaseSetting(Name) {
    ResetToDefault();
  }

  // Clear all child elements.
  void ResetToDefault() {
    items_.clear();
  }

  // Load settings from parent element.
  virtual bool LoadFromParent(TiXmlNode *root) {
    if (root == NULL) return false;

    // Find setting element node first.
    xml_node_ = root->FirstChildElement(setting_name_.c_str());

    // NOTE, empty element is not created for ListSetting.
    // For ListSetting, an empty element represents an empty list,
    // and no element represents using default list.
    if (xml_node_ == NULL) {
      return true;
    }

    // Load setting values from xml_node_.
    return LoadSetting(xml_node_);
  }

  // Load setting from a given element, which should represent the list setting.
  bool LoadSetting(TiXmlElement* element) {
    xml_node_ = element;
    if (xml_node_ == NULL) return true;

    items_.clear();
    for (TiXmlElement* itr =
      xml_node_->FirstChildElement(ItemName);
      itr; itr = itr->NextSiblingElement()) {
      ItemType item;
      item.LoadSetting(itr);
      items_.push_back(item);
    }
    return true;
  }

  // Save list setting to an xml element.
  TiXmlElement* SaveSetting() {
    xml_node_ = new TiXmlElement(setting_name_.c_str());

    // Enumerate value in urls
    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
      if (items_[i].Validate()) {
        xml_node_->LinkEndChild(items_[i].SaveSetting());
      }
    }
    return xml_node_;
  }

  TiXmlElement* SaveSetting(const BaseSetting* global) {
    if (Equals(global)) {
      return NULL;
    } else {
      return SaveSetting();
    }
  }

  bool Validate() const {
    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
      if (!items_[i].Validate()) {
        return false;
      }
    }

    return true;
  }

  bool Equals(const BaseSetting* a) const {
    const ListSetting<Name, ItemName, ItemType>* another = 
      (const ListSetting<Name, ItemName, ItemType>*) a;
    // Check size.
    if (items_.size() != another->items_.size()) {
      return false;
    }

    // Compare items one by one.
    for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
      if (!items_[i].Equals(&(another->items_[i]))) {
        return false;
      }
    }

    return true;
  }

  // Add a child element.
  void AddItem(const ItemType& item) {
    items_.push_back(item);
  }

  // Getter/setter for child elements.
  const std::vector<ItemType>& items() const {
    return items_;
  }
  void set_items(const std::vector<ItemType>& items) {
    items_ = items;
  }

private:
  std::vector<ItemType> items_;
};

#endif // COMMON_LISTSETTING_H__
