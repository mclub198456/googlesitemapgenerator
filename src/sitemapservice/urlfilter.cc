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


#include "sitemapservice/urlfilter.h"

///////////////////////////////////////////////////////////
// AndFilter implementation below.

AndFilter::~AndFilter() {
  for (FILTER_IT it = filters.begin(); it!=filters.end(); it++) {
    delete (*it);
  }
}

void AndFilter::AddFilter(UrlFilter* filter) {
  filters.push_back(filter);
}

bool AndFilter::Accept(const char* url,int urlLen) {
  for (FILTER_CIT it = filters.begin(); it!=filters.end(); ++it) {
    if (!(*it)->Accept(url,urlLen)) return false;
  }
  return true;
}


///////////////////////////////////////////////////////////
// OrFilter implementation below.

OrFilter::~OrFilter() {
  for (FILTER_IT it = filters.begin(); it!=filters.end(); ++it) {
    delete (*it);
  }
}

void OrFilter::AddFilter(UrlFilter* filter) {
  filters.push_back(filter);
}

bool OrFilter::Accept(const char* url,int urlLen) {
  for (FILTER_CIT it = filters.begin(); it!=filters.end(); ++it) {
    if ((*it)->Accept(url,urlLen)) return true;
  }
  return false;
}
