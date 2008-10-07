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


// BaseSetting is the base class for all setting classes. A setting class
// contains configurations for certain part of application. Usually, a setting
// object can be mapped to a segmement of XML configuration file.
// During runtime, an xml-node can be held by setting object. The setting
// object will load value of its members from given xml-node, or save value
// to given xml-node. There are two ways to combine a setting object with xml-
// node. One is Load(TiXmlNode* root) method, which extracts a child element
// from given "root" node as its xml-node. The other is "set_xml_node" method,
// which set the xml node directly.
// If there is no xml-node associate to setting object, the object could be
// used as an entity class with simple setter/getter for its members.
// This class declares the contract all setting classes must follow, and
// defines common member variables. It also provides some methods to help sub-
// class to load/save value from/to XML element.
// This class is not thread-safe.

#ifndef COMMON_BASESETTING_H__
#define COMMON_BASESETTING_H__

#include <string>
#include <vector>

#include "common/url.h"

#include "third_party/tinyxml/tinyxml.h"

class BaseSetting {
 public:
  // Constructor.
  // "setting_name" is the corresponding XML node name.
  BaseSetting(const char * setting_name);

  virtual ~BaseSetting();

  // Reset all fields to default values.
  virtual void ResetToDefault() = 0;

  // Load settings from an XML node.
  // It tires to find a child element of "parent" whose name is the same as
  // "setting_name" given to contructor. And then it loads setting values from
  // that child element.
  // If no child element with "setting_name" can be found, an empty xml node
  // is created. In this case, default values would be used.
  virtual bool LoadFromParent(TiXmlNode *parent);

  // Load setting values from the associated xml-node.
  // This method should be called after xml-node is set via "Load(TiXmlNode*)"
  // or "set_xml_node(TiXmlNode*)" method.
  // If some value doesn't occur in the xml-node, default value will be used.
  virtual bool LoadSetting(TiXmlElement* element) = 0;

  // Save settings to associated xml-node.
  // This method should be called after xml-node is set via "Load(TiXmlNode*)"
  // or "set_xml_node(TiXmlNode*)" method.
  virtual TiXmlElement* SaveSetting() = 0;

  virtual TiXmlElement* SaveSetting(const BaseSetting* global) = 0;

  // Validate if current settings are valid settings.
  virtual bool Validate() const = 0;

  virtual bool Equals(const BaseSetting* another) const = 0;

  // Accessor for "setting_name" value.
  // "setting_name" is the name of corresponding xml-node.
  const std::string & setting_name() const { return setting_name_; }
  void set_setting_name(const std::string &setting_name) {
    setting_name_ = setting_name;
  }

  // Accessor for the associated xml-node value.
  // See "Load" method for more information.
  const TiXmlElement *xml_node() const { return xml_node_; }
  void set_xml_node(TiXmlElement * xml_node) { xml_node_ = xml_node; }

 protected:
  // time_t value in string format, which should be recoganized by "strptime"
  // and "strftime".
  static const char         *kTimeFormat;

  bool UrlsEquals(const std::vector<Url>& a, const std::vector<Url>& b) const;

  // Load simple attribute values.
  // If the specific attribute doesn't exist, value wouldn't be modified.
  // Note, time should be in the format of "%Y-%m-%d %H:%M:%S" for strptime.
  // And for boolean value, only "true" (case-insensitive) is accepted as true.
  // A Url value is simply a string.
  void LoadAttribute(const char *attribute_name, std::string &value);
  void LoadAttribute(const char *attribute_name, int &value);
  void LoadAttribute(const char *attribute_name, bool &value);
  void LoadTimeAttribute(const char *attribute_name, time_t &value);
  void LoadAttribute(const char *attribute_name, Url &value);

  // Save simple attributes to assocciated xml-node.
  // If no xml-node is assocciated, nothing would be done. And if no specific
  // attribute is contained, a new one would be created.
  void SaveAttribute(const char *attribute_name, const std::string &value);
  void SaveAttribute(const char *attribute_name, const int value);
  void SaveAttribute(const char *attribute_name, const bool value);
  void SaveTimeAttribute(const char *attribute_name, const time_t &value);
  void SaveAttribute(const char *attribute_name, const Url &value);
  void SaveChild(BaseSetting* child);

  void SaveAttribute(const char *attribute_name,
                     const std::string &value, const std::string &global);
  void SaveAttribute(const char *attribute_name,
                     const int value, const int global);
  void SaveAttribute(const char *attribute_name,
                     const bool value, const bool global);
  void SaveTimeAttribute(const char *attribute_name,
                         const time_t &value, const time_t &global);
  void SaveAttribute(const char *attribute_name,
                     const Url &value, const Url& global);
  void SaveChild(BaseSetting* child, const BaseSetting* global);

  // Load a Url vector from a XML element tree.
  // "list-name" should be name of a child element of assocciated xml-node.
  // The the "list-name" child element should contains a list of <Url> elements.
  // e.g.
  // <this-xml-node>
  //   <actual-list-name>
  //      <Url value="/a.htm" />
  //      <Url value="/b.htm" />
  //   </actual-list-name>
  // </this-xml-node>
  void LoadUrls(const char *list_name, std::vector<Url> &urls);

  // Save a Url vector.
  void SaveUrls(const char *list_name, const std::vector<Url> &urls);

  // Setting tag name in XML file.
  std::string               setting_name_;

  TiXmlElement              *xml_node_;  
};

#endif  // COMMON_BASESETTING_H__
