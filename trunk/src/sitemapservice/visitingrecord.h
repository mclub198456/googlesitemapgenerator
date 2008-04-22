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


// This struct reprents a record for a specific URL's visiting history.
// It is used by application to store the basic information of URL.

#ifndef SITEMAPSERVICE_VISITINGRECORD_H__
#define SITEMAPSERVICE_VISITINGRECORD_H__

#include <time.h>
#include "common/basictypes.h"

struct VisitingRecord {
  // Defines the maximum length of url string.
	static const int kMaxUrlLength = 512;

  // The raw, encoded url, for example: "/dir1/query?key=value"
	char url[kMaxUrlLength];

  // Time when the url visiting record first comes.
	time_t first_appear;

  // The latest time when the url was visited.
	time_t last_access;

  // The last time when content pointed by the url is changed.
	time_t last_change;

  // Number of visiting times.
	int count_access;

  // Number of changing times.
	int count_change;

  // Hash code of latest content pointed by the url.
  int64 last_content;
};

#endif // SITEMAPSERVICE_VISITINGRECORD_H__

