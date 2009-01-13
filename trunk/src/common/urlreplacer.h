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


// UrlReplacer can replace specific pattern in a URL with specific values.
// The pattern could contains asterisk, which can be matched with any number of
// characters. Brackets are used to indicate which values should be replaced.
// For example, supposing the pattern is "/a*b[c][d]", and the replacing values
// are "C" and "D" respectively, a url like "/a123bcd" will be replaced as
// "/a123bCD".
// This class is thread-safe, and must be thread-safe.

#ifndef COMMON_URLREPLACER_H__
#define COMMON_URLREPLACER_H__

#include <string>
#include <vector>
#include <utility>

#include "common/patternfinder.h"

class UrlReplacer {
 public:
  UrlReplacer();
  ~UrlReplacer();

  // "pattern" can contains asterisk, which represents any number of characters.
  // Bracket is used in the pattern to mark which segments should be replaced
  // by "values". So the number of bracket pair should be same with the size of
  // "values" array.
  bool Initialize(const std::string& pattern,
                  std::vector<std::string>& values);

  // A convenient initialization method.
  // "values" is an array of replacing values, which are separated by comma.
  // See overloaded Initialize method above.
  bool Initialize(const std::string& pattern, const std::string& values);

  // Do replacement on given "url".
  // "size" is the maximum size of the url after replacing.
  bool Replace(char* url, int size);

 private:
  // Segments of given pattern, which is built in Initialize method.
  std::vector<std::pair<int, std::string> > segments_;

  // PatternFinder used to find pattern in url.
  PatternFinder* finder_;
};

#endif  // COMMON_URLREPLACER_H__

