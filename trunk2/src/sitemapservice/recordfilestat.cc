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


#include <string.h>
#include <math.h>
#include "sitemapservice/recordfilestat.h"

RecordFileStat::RecordFileStat() {
  Reset();
}

void RecordFileStat::Reset() {
  time(&newest_);
  memset(hours_, 0, sizeof(hours_));
  memset(days_, 0, sizeof(days_));
  very_old_ = 0;

  total_count_ = 0;
  total_access_ = 0;

  max_access_ = 1;
  max_access_log_ = -1.0;
}

// if set not full, use 1000(kDays) days plus 100(kHours) hours
time_t RecordFileStat::GetCutDownTime(int maxsize) {
  int offset = total_count_ - maxsize;

  // 3600 (minutes) * 24 hours
  // newest_ uses minute as unit, 
  // very_old_ means the count that before 1000(kDays) days plus 100(kHours) hours
  if (very_old_ >= offset) {
    return newest_ - kDays * 3600 * 24LL - kHours * 3600;
  }

  // days_[i] means the count in the day that is i days plus 100(kHours) hours before.
  int sum = very_old_;
  for (int i = kDays - 1; i >= 0; --i) {
    sum += days_[i];
    if (sum >= offset) {
      return newest_ - i * 3600 * 24LL - kHours * 3600;
    }
  }
 
  // hours_[i] means the count in the hour that is i hours before.
  for (int i = kHours - 1; i >= 0; --i) {
    sum += hours_[i];
    if (sum >= offset) {
      return newest_ - i * 3600;
    }
  }

  return newest_;
}

void RecordFileStat::AddRecord(const VisitingRecord &record) {
  ++total_count_;
  total_access_ += record.count_access;

  if (max_access_ < record.count_access) {
    max_access_ = record.count_access;
    max_access_log_ = log(static_cast<double>(max_access_));
  }

  int offset = static_cast<int>(newest_ - record.last_access);
  if (offset < 0) { // fixed it. it should add to hours_[0]
    ++hours_[0];
    return;
  }

  int hour_index = offset / 3600;
  if (hour_index < kHours) {
    ++hours_[hour_index];
    return;
  }

  offset -= kHours * 3600;
  int day_index = offset / (3600 * 24);
  if (day_index < kDays) {
    ++days_[day_index];
  } else {
    ++very_old_;
  }
}

double RecordFileStat::GetPriority(const VisitingRecord& record) {
  /*
  if (total_access_ == 0) return 0.5;

  double priority = record.count_access * 0.5 * total_count_ / total_access_;
  return priority > 1.0 ? 1.0 : priority;
  */
  if (record.count_access <= 0) return 0;

  double thislog = log(static_cast<double>(record.count_access));
  return thislog / max_access_log_ * 0.9 + 0.1;
}

