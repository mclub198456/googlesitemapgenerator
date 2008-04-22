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


// RecordMerger class is used to merge one or more visting record files.
// It merges records with same fingerprint. Other than that, during merging
// process, it also removes obsoleted URLs.
// A merging process is to merge two visiting records which belongs the same
// URL. For example, the visiting count should be added by merging.
// This class is thread safe as it contains no internal state.

#ifndef SITEMAPSERVICE_RECORDMERGER_H__
#define SITEMAPSERVICE_RECORDMERGER_H__

#include <set>
#include <string>
#include <vector>

#include "common/url.h"
#include "sitemapservice/visitingrecord.h"
#include "sitemapservice/recordfilestat.h"
#include "sitemapservice/recordfilemanager.h"


class RecordMerger {
public:
  // Empty constructor.
  RecordMerger();

  // Empty destructor.
  ~RecordMerger();

  // Merge two visiting records.
  // The merging result is stored in "record" instance.
  static void Merge(VisitingRecord& record, const VisitingRecord& another);

  // Merge multpile visting record files.
  // "destination" specifies the result record files. "fp_dest" represents the
  // file containing all the URL fingerprints included in "destination".
  // "sources" vector contains all the original record files.
  // "obsoleted" set contains fingerprints of all obsoleted visiting records,
  // which shouldn't occur in "destination" file.
  // "cutdonw" specifies the cut down time of last access value of visiting
  // records. Any visiting record whose last access time is older than that
  // should not occur in "destination" file.
  // "stat" stores the statistics of all the visting records.
  int Merge(const std::string& destination,
            const std::string& fp_dest,
            const std::vector<std::string>& sources,
            const std::set<UrlFprint>& obsoleted,
            const time_t& cutdown,
            RecordFileStat* stat);

  // Merge visting record files specifed by given "filemanager"
  // It merges the old base data file and all temporary data files into a new
  // base data file. The base data file and temporary files are specified by
  // "filemanager".
  // For "obsoleted" "cutdown" and "stat", please see above Merge method.
  // "maxsize" represents the max number of URLs contained in new base data
  // file. If the merging result exceeds "maxsize", the URLs with oldest
  // last_access time are excluded from result.
  int Merge(RecordfileManager* filemanager, const std::set<UrlFprint>& obsoleted,
            int maxsize, const time_t& cutdown, RecordFileStat* stat);

  // Merge URL fingerprint files.
  // "dest" is the file storing merging result.
  // "srcs" contains all the fingerprint files to be merged.
  // "obsoleted" contains fingerprints of all URLs which should be excluded
  // from result.
  bool MergeUrlFprint(const std::string& dest,
                      const std::vector<std::string> srcs,
                      const std::set<UrlFprint>& obsoleted);

private:

  // Remove old records from given recordfile.
  // It is a helper routine for "Merge" method above.
  // "recordfile" represents the visting record to be processed.
  // "fpfile" represents file containing fingerprints of all visiting records
  // contained in "recordfile".
  // "obsoleted" set contains fingerprints of all obsoleted URLs.
  // "stat" should return the statistics of new "recordfile".
  int RemoveOldRecords(const std::string& recordfile,
                       const std::string& fpfile,
                       const std::set<UrlFprint>& obsoleted,
                       const time_t& cutdown,
                       RecordFileStat* stat);
};

#endif // SITEMAPSERVICE_RECORDMERGER_H__

