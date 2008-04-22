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


#include "sitemapservice/asteriskfilter.h"


// Split the pattern string with "*" delimiter, and create a PatternFinder
// with the resulting pattern array.
// e.g. pattern = "*hello**world" -> pattern_array = {"", "hello", "world"}
AsteriskFilter::AsteriskFilter(const std::string& pattern) {
  std::vector<std::string> patterns;

  // If the first character is "*", an empty pattern should be put at first.
  // This is for convenience of matching, PatternFinder always use the first
  // pattern to match the beginning of string.
  // For example, "*hello**world" could be viewed as:
  // "[EMPTY_STRING][ANY_STRING][hello][ANY_STRING][world]
  if (pattern.length() > 0 && pattern[0] == '*') {
    patterns.push_back("");
  }

  // Split pattern according to "*". Duplicated "*" is ignored.
  for (int i = 0, len = static_cast<int>(pattern.length()); i < len; ) {
    while (i < len && pattern[i] == '*') ++i;
    int j = i + 1;
    while (j < len && pattern[j] != '*') ++j;
    patterns.push_back(pattern.substr(i, j - i));
    i = j;
  }

  // Check whether the last character is "*".
  if (pattern.length() > 0 && pattern[pattern.length() - 1] == '*') {
    patterns.push_back("");
  }

  finder_ = new PatternFinder(patterns);
}

AsteriskFilter::~AsteriskFilter() {
  if (finder_ != NULL) {
    delete finder_;
  }
}

bool AsteriskFilter::Accept(const char *url, int len) {
  // If the pattern could be found in given URL, the URL is accepted.
  std::vector<PatternPosition> positions;
  return finder_->Find(url, len, &positions);
}

