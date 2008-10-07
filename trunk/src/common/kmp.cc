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


#include "common/kmp.h"

#include <string>

KMPPattern::KMPPattern(const std::string& pattern) {
  pattern_ = pattern;
  table_.resize(pattern.length());

  // build KMP partial function table.
  int size = static_cast<int>(pattern_.size());
  if (size > 0) table_[0] = -1;

  for (int i = 1; i < size; ++i) {
    table_[i] = table_[i - 1] + 1;
    while (table_[i] > 0 && pattern[i - 1] != pattern[table_[i] - 1]) {
      table_[i] = table_[table_[i] - 1] + 1;
    }
  }
}

int KMPPattern::Match(const char *text, int len) {
  if (pattern_.length() == 0) return 0;

  // Run KMP match function.
  int j = 0;
  for (int i = 0; i < len; ++i) {
    if (text[i] == pattern_[j]) {
      ++j;
      if (j == pattern_.length()) {
        return i - static_cast<int>(pattern_.length()) + 1;
      }
    } else if (j != 0) {
      j = table_[j];
      --i;
    }
  }
  return -1;
}

int KMPPattern::Match(const char *text) {
  return Match(text, static_cast<int>(strlen(text)));
}
