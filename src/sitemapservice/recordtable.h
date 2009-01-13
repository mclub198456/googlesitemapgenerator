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

//
// RecordTable is used to mantain url visiting records in memory.
// It automatically merges visiting records with same url string.
// Besides the common methods, like add/get/clear, this class also
// provides methods to automatically remove out-of-date records, as well as
// java style iterator. User can also load/save records from/to a file through
// methods exposed by this class.
//
// Note, this class is not thread safe.

#ifndef SITEMAPSERVICE_RECORDTABLE_H__
#define SITEMAPSERVICE_RECORDTABLE_H__

#include <string>
#include "common/basictypes.h"
#include "common/url.h"
#include "common/hashmap.h"
#include "sitemapservice/visitingrecord.h"


class RecordTable {
  // Data structure used to hold visiting records
  typedef HashMap<UrlFprint, VisitingRecord*>::Type HashTable;

 public:
  // This is the threshold for change in content length.
  // If the difference in content length between two subsequent visits exceeds
  // this threshold, the page will be treated as having been updated.
  static const int64 kChangeLengthThreshold = 100;

  // A java-style iterator class used to iterate all records in the table.
  // It is used to hide internal implementation details.
  class Iterator {
   public:
    explicit Iterator(const HashTable& hashtable) 
        : hashtable_(hashtable) {
      iterator_ = hashtable_.begin();
    }

    bool HasNext() const {
      return iterator_ != hashtable_.end();
    }

    const VisitingRecord& Next() {
      const VisitingRecord& result = *(iterator_->second);
      ++iterator_;
      return result;
    }

   private:
    HashTable::const_iterator iterator_;

    // The hash table to be iterated.
    const HashTable& hashtable_;

    DISALLOW_EVIL_CONSTRUCTORS(Iterator);
  };

  // Constructor. Simply store the params to name-like data members.
  RecordTable(const std::string& base_url, int max_size);

  ~RecordTable();

  // Returns the base url (common prefix) for urls in this table.
  const std::string& base_url() const { return base_url_; }

  // Returns the capacity of this record table.
  int max_size() const {return max_size_;}

  // Returns number of records contained in this table.
  int Size() const { return (int) records_.size(); }

  // Clears all visiting records contained in this table.
  void Clear();

  // Adds a url visiting record.
  // lastmodified represents "Last-Modified" field of the HTTP header.
  // filewrite is last write attribute of the static page file.
  // Returns 0 if successful, other a non-zero error code.
  //
  // If the same url is already contained in this table, the old entry will be
  // updated.
  // Visiting count is increased by one, last accessing time is updated as the
  // current time, last modified time will be updated if necessary.
  // If there is no such url currently, a new entry is created for this one.
  //
  // The policy for calculating last_change time is as follows.
  // If filewrite is positive, it will be used.
  // Or else if lastmodified is positive, it will be used.
  // If none of above is positive, and if this url is first visited, the current
  // time will be used. Otherwise, contentlength will be used to compare to old
  // contentlength, if the difference exceeds kChangeThreshold, the current time
  // is used as last_change time.
  int AddRecord(const char* url, int64 contentlength,
                const time_t& lastmodified, const time_t& filewrite);

  // Get the visiting record for the specified url.
  // or null if there is no visiting record for the url.
  // Note, the returned pointer is managed by this table.
  const VisitingRecord* GetRecord(const char* url) const;

  // Collects gabarge, and removes out of date visiting records.
  // All the visiting records, for which the last visit time is older than given
  // value, will be removed from this table.
  // Returns how many records are actually removed during this GC operation.
  int GC(time_t oldest);

  // Do GC operation heuristicly. (See GC(time_t))
  // The internal implementation "guesses" oldest param for GC(time_t oldest)
  // automatically.
  // After this method is invoked, at least 10% space will be availabe for use.
  // But there is no upper bound.
  // Returns how many records are actually removed during this GC oepration.
  int HeuristicGC();

  // Saves this table to a file.
  // Records in the result file will be in ascending order by the record's
  // finger print value.
  // Returns 0 if successful, or a non-zero error code.
  int Save(const char* path) const;

  // Loads visiting records from a file.
  // NOTE, all the existing records will be cleared.
  // Returns 0 if successful, or a non-zero error code.
  int Load(const char* path);

  // Gets an iterator to walk through this table.
  // NOTE, the returned pointer should be deleted by caller after use.
  Iterator* GetIterator() const { return new Iterator(records_); }

 private:
  // Holds the visiting records.
  HashTable records_;

  // Represents max size of this->records_ table.
  int max_size_;

  // Represents the common prefix for all visited url in this record table.
  // Usually, it would be a host name, like "http://www.example.org"
  std::string base_url_;

  // Represents the cutdown time in last GC operation, including both
  // GC(time_t) and SupressGC() methods.
  // All visiting records older than this value were removed from this table
  // in last GC operation.
  time_t last_gc_cutdown_;

  DISALLOW_EVIL_CONSTRUCTORS(RecordTable);
};

#endif // SITEMAPSERVICE_RECORDTABLE_H__

