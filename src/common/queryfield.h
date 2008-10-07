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

// QueryField represents query field in http url query string.
// The corresponding xml elements look like:
//    <QueryField name="field1" />

#ifndef COMMON_QUERYURL_H__
#define COMMON_QUERYURL_H__

#include <string>

#include "common/basesetting.h"
#include "common/listsetting.h"

class QueryField : public BaseSetting {
 public:
  QueryField();
  ~QueryField() {}

  virtual void ResetToDefault();

  virtual bool LoadSetting(TiXmlElement* element);
  virtual TiXmlElement* SaveSetting();
  virtual TiXmlElement* SaveSetting(const BaseSetting* global);

  virtual bool Validate() const;

  bool Equals(const BaseSetting* another) const;

  // Getter/setter for setting values.
  const std::string& name() const { return name_; }
  void set_name(const std::string& name) { name_ = name; }

 private:
  std::string name_;
};

// Define IncludedQueryFields setting.
extern const char kQueryFieldName[];
extern const char kIncludedQueryFieldsName[];
typedef ListSetting<kIncludedQueryFieldsName, kQueryFieldName, QueryField> \
IncludedQueryFields;

#endif  // COMMON_QUERYURL_H__

