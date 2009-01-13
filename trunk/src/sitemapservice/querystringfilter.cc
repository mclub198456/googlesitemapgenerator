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

#include "sitemapservice/querystringfilter.h"

QueryStringFilter::QueryStringFilter() {
  // do nothing.
}

QueryStringFilter::~QueryStringFilter() {
  // do nothing.
}

bool QueryStringFilter::Initialize(const IncludedQueryFields &fields) {
  fields_.clear();
  const std::vector<QueryField>& field_vector = fields.items();
  for (int i = 0; i < static_cast<int>(field_vector.size()); ++i) {
    fields_.insert(field_vector[i].name());
  }
  return true;
}

void QueryStringFilter::Filter(char *url) {
#ifndef GSG_LOW_PRIVACY
  // Find the query string.
  while (*url != '\0' && *url != '?') ++url;
  if (*url == '\0') return;

  bool first_field = true;
  char* offset = url;
  std::string name;
  for (; (*url) != '\0'; ) {
    // Parse query field name.
    char* p = url + 1;
    char* p1 = p;
    while (*p1 != '\0' && *p1 != '=' && *p1 != '&') ++p1;

    // Parse query field value.
    char* p2 = p1;
    while (*p2 != '\0' && *p2 != '&') ++p2;

    if (p1 != p) {
      name.assign(p, p1);
      if (fields_.find(name) != fields_.end()) {
        // Append the separator
        *offset = first_field ? '?' : '&';
        ++offset;
        first_field = false;

        // Append the query field;
        strncpy(offset, p, p2 - p);
        offset += p2 - p;
      }
    }

    url = p2;
  }
  *offset = '\0';
#endif // GSG_LOW_PRIVACY
}
