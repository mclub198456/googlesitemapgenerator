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


#include "common/patternfinder.h"

#include <vector>
#include <string>

PatternFinder::PatternFinder(const std::string& pattern) {
  patterns_.push_back(KMPPattern(pattern));
}

PatternFinder::PatternFinder(const std::vector<std::string>& patterns)
  : patterns_() {
  for (int i = 0, isize = static_cast<int>(patterns.size());
    i < isize; ++i) {
      patterns_.push_back(KMPPattern(patterns[i]));
  }
}

PatternFinder::~PatternFinder() {
  // do nothing
}

bool PatternFinder::Find(const char* text, int len,
                         std::vector<PatternPosition>* positions) {
  positions->clear();
  if (patterns_.size() == 0) return true;

  // The first one should be matched at index 0.
  if (patterns_.front().Match(text, len) != 0) return false;
  int left = patterns_.front().Length();

  // Add the first position.
  PatternPosition position;
  position.begin = 0, position.end = patterns_.front().Length();
  positions->push_back(position);

  // The last one should be matched extactly at the tail.
  int right = len - patterns_.back().Length();
  if (right < 0 || patterns_.back().Match(text + right, len - right) < 0) {
    return false;
  }

  // Match the intermediate patterns.
  for (int i = 1, isize = static_cast<int>(patterns_.size()) - 1;
    i < isize; ++i) {
    int k = patterns_[i].Match(text + left, right - left);
    if (k < 0) return false;

    position.begin = k + left;
    left += k + patterns_[i].Length();
    position.end = left;
    positions->push_back(position);
  }

  position.begin = right, position.end = len;
  positions->push_back(position);

  return true;
}
