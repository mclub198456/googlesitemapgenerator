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

// This file defines conversion functions for different message formats.

#ifndef COMMON_MESSAGECONVERTER_H__
#define COMMON_MESSAGECONVERTER_H__

#include <string>
#include <map>

class MessageConverter {
 public:
  typedef std::map<std::string, std::string> StringMap;

  // Converter between map and its string representation, like:
  // key1:value,key2:value2,...,keyN:valueN
  // Keys and values can contains ',', ':' or '\\', which must be escaped
  // by '\\'.
  static void MapToString(const StringMap& strmap, std::string* str);
  static bool StringToMap(const std::string& str, StringMap* strmap);

  // Encode/decode a string.
  // ',', ':', '\\' are escaped by '\\' in an encoded string.
  static void EncodeString(const std::string& str, std::string* result);
  static bool DecodeString(const std::string& str, std::string* result);

 private:
  MessageConverter() {}
};

#endif // COMMON_MESSAGECONVERTER_H__
