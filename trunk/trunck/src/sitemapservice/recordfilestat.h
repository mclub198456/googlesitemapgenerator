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


// RecordFileStat represents the statistics data of a visiting record file.
// It contains the total number of URLs contained in the record file. Besides
// that, it also provides method to calculate priority of a URL in the file.
// This class also provides the histograms of last-access time value. The
// histogram provides the number of URLs whose last-access time is in last hour,
// in last two hour ... and  up to last kHours. After kHours, the granularity
// becomes more coarse, only last day, last two days .. last kDays values are
// provided.
// This class is NOT thread-safe.

#ifndef SITEMAPSERVICE_RECORDFILESTAT_H__
#define SITEMAPSERVICE_RECORDFILESTAT_H__

#include <time.h>
#include "sitemapservice/visitingrecord.h"


class RecordFileStat {
public:
  // This class counts the number of URLs whose last-access is in last one
  // hour, last two hours ... and last "kHours".
  static const int kHours = 100;

  // This class counts the number of URLs whose last-access is in last day,
  // last two days ... and last "kDays". Please note that, here last day means
  // 24 hours before kHours, i.e. [NOW - (kHours + 24), Now - kHours].
  static const int kDays = 1000;

  // Empty constructor.
  RecordFileStat();

  // Reset the statistics data to intial values.
  // This method MUST be called whenever a new file is processed.
  void Reset();

  // Add a record to the statisic data.
  void AddRecord(const VisitingRecord& record);

  // Get cutdown time for the newest "maxsize" visting records.
  // This means at most "maxsize" number of URLs whose last-access time is
  // later than returned cut-down time.
  // Note, the returned value is not very accurate.
  time_t GetCutDownTime(int maxsize);

  // Get total number of visiting records (URLs) in this statistics data.
  int GetTotalCount() const {return total_count_;}

  // Get the priority of given "record" according to internal statistics data.
  // This method is only meaningful after all visting records are added.
  // The returned priority is in the range of [0.1, 1.0].
  double GetPriority(const VisitingRecord& record);

private:
  // "hours_[i]" represents the number of URLs whose last-access time is in
  // last "i" hour.
  int hours_[kHours];

  // "days_[i]" represents the nubmer of URLs whose last-access time is in last
  // "i" day. Here the benchmark time of last "i" day is "kHours" ago.
  int days_[kDays];

  // Represents the number of URLs whose last-access time neither belongs to
  // last kHours nor belongs to last kDays.
  // As a summary, all urls belongs to one of following values according to
  // their last-access time: (before kDays), (between kDays and kDays - 1),
  // ... (in last hour).
  int very_old_;

  // Represents the total number of URLs in statistics data.
  int total_count_;

  // Represents the largest value of last-access time of all URLs.
  time_t newest_;

  // Represents total accessing times of all URLs in this statistics data.
  int64 total_access_;

  // Represents the max accessing time of all URls.
  int64 max_access_;

  // Represents base 2 logarithm of max_access_ value.
  double max_access_log_;
};

#endif // SITEMAPSERVICE_RECORDFILESTAT_H__

