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


// KMPPattern includes a string pattern, and uses well-known KMP algorithm
// to search pattern from given text.
// This class is thread-safe.

#ifndef COMMON_KMP_H__
#define COMMON_KMP_H__

#include <string>
#include <vector>

class KMPPattern {
 public:
  // Constructor.
  KMPPattern(const std::string& pattern);

  // Search pattern from text.
  // Returns the matching position, or -1 if no pattern can be found.
  int Match(const char* text, int len);
  int Match(const char* text);

  // The length of pattern.
  int Length() const {
    return static_cast<int>(pattern_.length());
  }

 private:
  // Pattern string.
  std::string pattern_;

  // "partial match" table for KMP algorithm.
  // It is built in constructor.
  std::vector<int> table_;
};

#endif  // COMMON_KMP_H__
