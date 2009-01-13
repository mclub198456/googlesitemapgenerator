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


// BaseRuntimeInfo is the base class for all runtime information classes.
// It defines two pure virtual methods, "Save" and "Reset", to force all
// runtime information classes to provide these two functionalities. Besides
// that, this class also provides some common routines to help sub-classes
// to save and format attributes.
// This class is thread-safe.

#ifndef SITEMAPSERVICE_BASERUNTIMEINFO_H__
#define SITEMAPSERVICE_BASERUNTIMEINFO_H__

#include <sstream>

#include "third_party/tinyxml/tinyxml.h"
#include "common/basictypes.h"

class BaseRuntimeInfo {
 public:
  // Empty constructor.
  BaseRuntimeInfo() {}

  // Empty destructor.
  virtual ~BaseRuntimeInfo() {}

  // Save values to given xml element.
  virtual bool Save(TiXmlElement* element) = 0;

  // Reset all the fields to default values.
  virtual void Reset() = 0;

 protected:
  // Helper methods for Save runtime info to an XML element.
  // NOTE, there is no validation on args.
  void SaveBoolAttribute(TiXmlElement* element, const char* name, bool value);
  void SaveTimeAttribute(TiXmlElement* element, const char* name, time_t value);

  template<typename T> void SaveAttribute(TiXmlElement* element,
                                          const char* name, T value) {
    std::ostringstream out;
    out << value;
    element->SetAttribute(name, out.str().c_str());
  }

 private:
  // Format of time value.
  // The format string should be understood by "strftime" method.
  static const char* kTimeFormat;
};

#endif  // SITEMAPSERVICE_BASERUNTIMEINFO_H__
