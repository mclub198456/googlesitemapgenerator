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


// UrlFilterBuilder provides static methods to build url filter from a url
// pattern or a list or url patterns. A single URL pattern given here should
// be asterisk pattern only.

#ifndef SITEMAPSERVICE_URLFILTERBUILDER_H__
#define SITEMAPSERVICE_URLFILTERBUILDER_H__

#include <vector>
#include "common/url.h"
#include "sitemapservice/urlfilter.h"

class UrlFilterBuilder {
 public:
  // Build a url filter form a single asterisk pattern.
  // Note, caller should release the returned pointer.
  static UrlFilter* Build(const Url& pattern);

  // Build a complicated url filter from a list of asterisk patterns.
  // The operator used to combine single pattern is OR logic operator.
  // If given patterns are empty, an accept-all filter will be returned.
  // Note, caller should release the returned pointer.
  static UrlFilter* Build(const std::vector<Url>& patterns);

 private:
  UrlFilterBuilder();
};

#endif // SITEMAPSERVICE_URLFILTERBUILDER_H__

