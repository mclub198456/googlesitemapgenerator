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


#ifndef SITEMAPSERVICE_SITEDATAPROVIDER_H__
#define SITEMAPSERVICE_SITEDATAPROVIDER_H__

#include "sitemapservice/visitingrecord.h"
#include "sitemapservice/recordfilestat.h"

class SiteDataProvider {
 public:
  virtual ~SiteDataProvider() {}

  virtual time_t GetLastUpdate() = 0;
  virtual bool ForceUpdate() = 0;

  virtual bool GetHostName(std::string* host) = 0;
  virtual RecordFileStat GetRecordFileStata() = 0;

  virtual bool OpenRecord() = 0;
  virtual bool ReadRecord(VisitingRecord* record) = 0;
  virtual void CloseRecord() = 0;
};


#endif // SITEMAPSERVICE_SITEDATAPROVIDER_H__

