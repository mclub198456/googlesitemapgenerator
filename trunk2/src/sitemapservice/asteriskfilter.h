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


// AsteriskFilter uses filtering pattern with asterisk for blob match.
// An asaterisk (*) could match any number of consecutive characters, including
// zero character. For example, "/*.html" can match "/.html" and
// "/view?page=a.html", but can't match "/a.html?key=value".
// This class is thread-safe.

#ifndef SITEMAPSERVIE_H__
#define SITEMAPSERVIE_H__

#include <string>

#include "common/patternfinder.h"
#include "sitemapservice/urlfilter.h"

// This class is implemented as an "Adapter" adapting PatternFinder to
// UrlFilter interface.
class AsteriskFilter : public UrlFilter {
 public:
  // Construct with given filtering pattern.
  // The pattern could contains zero or more asterisks in it. All the other
  // characters don't have special meaning.
  AsteriskFilter(const std::string& pattern);

  // Destructor.
  virtual ~AsteriskFilter();

  // Check whether given URL is acceptted by this filter.
  virtual bool Accept(const char* url, int len);

 private:
  // This instance does the actual pattern matching job.
  PatternFinder* finder_;
};

#endif // SITEMAPSERVIE_H__
