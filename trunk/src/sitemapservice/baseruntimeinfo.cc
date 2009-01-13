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


#include "sitemapservice/baseruntimeinfo.h"
#include "common/timesupport.h"

const char* BaseRuntimeInfo::kTimeFormat = "%Y-%m-%d %H:%M:%S";

// Format boolean value as "true" or "false"
void BaseRuntimeInfo::SaveBoolAttribute(TiXmlElement* element,
                                    const char* name, bool value) {
  element->SetAttribute(name, value ? "true" : "false");
}

// Format negative time value as -1;
// Format all the other value as kTimeFormat.
void BaseRuntimeInfo::SaveTimeAttribute(TiXmlElement* element,
                                    const char* name, time_t value) {
  if (value == -1) {  
    element->SetAttribute(name, "-1");
  } else {
    char value_str[256];
    strftime(value_str, sizeof(value_str), kTimeFormat, localtime(&value));
    element->SetAttribute(name, value_str);
  }
}

