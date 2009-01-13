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


#ifndef COMMON_URLRECORD_H__
#define COMMON_URLRECORD_H__

#include <time.h>
#include "common/basictypes.h"


// This record is used to store individual URL information
struct UrlRecord {
  // the raw, encoded url, for example: "/dir1/query?key=value"
  char url[kMaxUrlLength];

  // the host value, maybe include port if not 80
  char host[kMaxHostLength];

  // the site id
  char siteid[kMaxSiteIdLength];

  // hash code of content pointed by the url
  int64 contentHashCode;

  // Represents the LastModified field in http header.
  // -1 represents nothing is defined.
  time_t last_modified;

  // This field represents the last write time of corresponding file.
  // -1 represents un-available.
  time_t last_filewrite;

  // represents the access time
  time_t last_access;

  

  // represents the http status code
  int statuscode;
};


#endif  // COMMON_URLRECORD_H__
