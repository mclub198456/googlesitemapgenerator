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


// Pattern finder is used to check whether a list of patterns occurs in a given
// text. The first pattern in the list must be matched at the beginning of text
// exactly. And the last pattern in the list must be matched to the ending of
// text exactly.
// For example, the list of patterns is {"a", "b"}. Then "ab" and "acb" are 
// matched text. "cab", "ba", and "abc" are not matched. Empty string is a
// sepcial pattern. "cadb" can be matched for {"", "a", ""}.

#ifndef COMMON_PATTERNFINDER_H__
#define COMMON_PATTERNFINDER_H__

#include <string>
#include <vector>

#include "common/kmp.h"

// Position of pattern in text.
// The pattern should be in [begin, end) range of text.
struct PatternPosition {
  int begin;
  int end;
};

class PatternFinder {
 public:
  // Constructor.
  // This is the same with vector<string>(pattern, 1).
  PatternFinder(const std::string& pattern);

  // Constructor.
  // The pattern should occur in text sequentially.
  PatternFinder(const std::vector<std::string>& patterns);

  ~PatternFinder();

  // Find patterns in given text.
  // The patterns should occurs in the text sequentially. The first pattern
  // should be matched at the beginning of text exactly. And the last pattern
  // should be matched at the ending of text exactly.
  // "len" is the length of "text" string.
  // "positions" returns the pattern positions in text string.
  bool Find(const char* text, int len, std::vector<PatternPosition>* positions);

 private:
  std::vector<KMPPattern> patterns_;
};

#endif  // COMMON_PATTERNFINDER_H__
