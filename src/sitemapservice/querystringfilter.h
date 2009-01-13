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

// This filter is used to filter query fields in url query string part.
// Query fileds are splitted by '&' and '='. And only configured parts are
// kept while all the parts are removed.

#ifndef SITEMAPSERVICE_QUERYSTRINGFILTER_H__
#define SITEMAPSERVICE_QUERYSTRINGFILTER_H__

#include <string>
#include <set>

#include "common/queryfield.h"

class QueryStringFilter {
public:
  QueryStringFilter();
  ~QueryStringFilter();

  // Initialize this filter with query fields name, which should be included.
  bool Initialize(const IncludedQueryFields& fields);
  
  // Filter given url.
  // All un-expected query fields will be removed from the url.
  void Filter(char* url);

private:
  std::set<std::string> fields_;
};

#endif // SITEMAPSERVICE_QUERYSTRINGFILTER_H__
