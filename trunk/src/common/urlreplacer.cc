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


#include "common/urlreplacer.h"

#include <string>
#include <vector>

UrlReplacer::UrlReplacer() {
  finder_ = NULL;
}

UrlReplacer::~UrlReplacer() {
  if (finder_ != NULL) delete finder_;
}

bool UrlReplacer::Initialize(const std::string& pattern,
                             std::vector<std::string>& values) {
  // first pass: determine whether [] are matched.
  int count = 0;
  for (int i = 0; i < static_cast<int>(pattern.length()); ++i) {
    if (pattern[i] == '[') {
      if (count & 1) return false;
      ++count;
    }

    if (pattern[i] == ']') {
      if ((count & 1) == 0) return false;
      ++count;
    }
  }

  count >>= 1;
  if (count != static_cast<int>(values.size())) {
    return false;
  }

  // second pass: split with asterisk (*)
  // after splitting, the pattern will be like: part1*part2*part3...partN
  std::vector<std::string> parts;
  if (pattern.length() > 0 && pattern[0] == '*') {
    parts.push_back("");
  }
  for (int i = 0, len = static_cast<int>(pattern.length()); i < len;) {
    while (i < len && pattern[i] == '*') ++i;
    int j = i + 1;
    while (j < len && pattern[j] != '*') ++j;
    parts.push_back(pattern.substr(i, j - i));
    i = j;
  }
  if (pattern.length() > 0 && pattern[pattern.length() - 1] == '*') {
    parts.push_back("");
  }

  // third pass: try to replace values in splitted part
  // Result: ActualValue1 + AsteriskA1 + ActualValue2 + AstersikA2 + ...
  segments_.clear();
  bool in_bracket = false;
  int valueindex = 0;
  std::vector<std::string> patterns;
  for (int i = 0, isize = static_cast<int>(parts.size()); i < isize; ++i) {
    std::string buffer;
    std::string pattern;
    for (int j = 0, jsize = static_cast<int>(parts[i].length());
      j < jsize; ++j) {
      char c = parts[i][j];
      if (in_bracket) {
        if (c == ']') {
          buffer.append(values[valueindex++]);
          in_bracket = false;
        }
      } else {
        if (c == '[') {
          in_bracket = true;
        } else {
          buffer.push_back(c);
        }
      }

      if (c != '[' && c != ']') {
        pattern.push_back(c);
      }
    }

    if (buffer.length() != 0) {
      segments_.push_back(std::make_pair(-1, buffer));
    }
    if (in_bracket == false && i + 1 != isize) {
      segments_.push_back(std::make_pair(i, ""));
    }

    patterns.push_back(pattern);
  }
  finder_ = new PatternFinder(patterns);

  return true;
}

bool UrlReplacer::Initialize(const std::string& pattern,
                             const std::string& values) {
  std::vector<std::string> valuearray;
  for (size_t i = 0; i < values.length(); ++i) {
    while (i < values.length() && values[i] != '[') ++i;
    size_t j = ++i;
    while (j < values.length() && values[j] != ']') ++j;
    if (j < values.length()) {
      valuearray.push_back(values.substr(i, j - i));
    }
    i = j;
  }
  return Initialize(pattern, valuearray);
}



bool UrlReplacer::Replace(char* url, int size) {
  // Use PatternFinder to find patterns in url.
  std::vector<PatternPosition> positions;
  int len = static_cast<int>(strlen(url));
  if (len >= size) return false;

  if (!finder_->Find(url, len, &positions)) {
    return false;
  }

  // Reconstruct url according to pattern finder result.
  std::string clone(url);
  int count = 0;
  for (int i = 0, isize = static_cast<int>(segments_.size()); i < isize; ++i) {
    if (segments_[i].first == -1) {
      int length = static_cast<int>(segments_[i].second.length());
      if (count + length >= size)
        return false;
      strncpy(url + count, segments_[i].second.c_str(), size - length - count);
      count += length;
    } else {
      int left = positions[segments_[i].first].end;
      int right = positions[segments_[i].first + 1].begin;
      if (count + right - left >= size) return false;
      strncpy(url + count, clone.c_str() + left, right - left);
      count += right - left;
    }
  }
  url[count] = '\0';

  return true;
}
