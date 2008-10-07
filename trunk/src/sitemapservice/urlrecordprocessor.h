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


#ifndef SITEMAPSERVICE_URLRECORDPROCESSOR_H__
#define SITEMAPSERVICE_URLRECORDPROCESSOR_H__

#include "common/urlrecord.h"

class UrlRecordProcessor {
public:
  virtual ~UrlRecordProcessor() {}

  // NOTE, the record may be changed during processing.
  virtual int ProcessRecord(UrlRecord& record) = 0;

protected:
  UrlRecordProcessor() {}
};

#endif // SITEMAPSERVICE_URLRECORDPROCESSOR_H__

