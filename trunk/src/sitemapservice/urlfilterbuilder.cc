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


#include "sitemapservice/asteriskfilter.h"
#include "sitemapservice/urlfilterbuilder.h"

UrlFilter* UrlFilterBuilder::Build(const Url& pattern) {
  AsteriskFilter* filter = new AsteriskFilter(pattern.path_url());
  return filter;
}

UrlFilter* UrlFilterBuilder::Build(const std::vector<Url>& patterns) {
  OrFilter* orfilter = NULL;
  for (int i = 0; i < static_cast<int>(patterns.size()); ++i) {
    UrlFilter* filter = Build(patterns[i]);
    if (filter != NULL) {
      if (orfilter == NULL) {
        orfilter = new OrFilter();
      }
      orfilter->AddFilter(filter);
    }
  }

  if (orfilter != NULL) {
    return orfilter;
  } else {
    return new DummyFilter(false);
  }
}
