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


#include "sitemapservice/recordtable.h"
#include "sitemapservice/recordfileio.h"
#include "sitemapservice/recordfilemanager.h"
#include "common/port.h"

#include <cassert>
#include <algorithm>

RecordTable::RecordTable(const std::string& base_url, int max_size) {
  assert(base_url.length() > 0);
  assert(max_size > 0);

  base_url_ = base_url;
  max_size_ = max_size;

  // set last_gc_cutdown_ to current time.
  // Beucause all the visiting record added later will have last-visiting-time
  // equal to that time, which is of course newer than current time,
  // so initially, the last_gc_cutdown_ could filter no record.
  time(&last_gc_cutdown_);

  // ensure there is no ending '/',
  // because url added in AddRecord is like "/path/page1"
  if (base_url_[base_url_.length() - 1] == '/') {
    base_url_.erase(base_url_.length() - 1);
  }

  // remove protocol.
  if (base_url_.find("http://") == 0) {
    base_url_.erase(0, 7);
  } else if (base_url_.find("https://") == 0) {
    base_url_.erase(0, 8);
  }
}

RecordTable::~RecordTable() {
  // does nothing
}

const VisitingRecord* RecordTable::GetRecord(const char* url) const {
  UrlFprint fingerprint = Url::FingerPrint(url);
  HashTable::const_iterator itr = records_.find(fingerprint);
  return itr == records_.end() ? NULL : itr->second;
}


int RecordTable::AddRecord(const char *url, int64 content,
                           const time_t& lastmodified,
                           const time_t& filewrite) {

  // ignore null url or too long url
  if (url == NULL) {
    return 1;
  }

  std::string url_str(url);
  VisitingRecord* record = NULL;

  time_t current_time = time(NULL);
  UrlFprint fprint = Url::FingerPrint(url);

  // no old record with same url is found
  if (records_.find(fprint) == records_.end()) {
    // only allow max_size_ entries in table.
    if (static_cast<int>(records_.size()) >= max_size_) {
      return 1;
    }

    // the url first appears in this table,
    record = new VisitingRecord();
    records_[fprint] = record;

    record->update_url(url);
    record->first_appear = record->last_access = current_time;
    record->count_access = record->count_change = 1;
    record->last_content = content;

    // determine the last_change value.
    if (filewrite != -1) {
      record->last_change = filewrite;
    } else {
      record->last_change = (lastmodified != -1 ? lastmodified : current_time);
    }
  } else {
    // update the old entry.
    record = records_[fprint];
    record->last_access = current_time;
    record->count_access += 1;

    // update the last_content, count_change, and last_change
    if (filewrite == -1) {
      if (content != -1 &&
        llabs(record->last_content - content) > kChangeLengthThreshold) {
        record->count_change += 1;
        record->last_content = content;
        record->last_change = (lastmodified != -1 ?
            lastmodified : current_time);
      }
    } else {
      if (record->last_change != filewrite) {
        record->count_change += 1;
        record->last_content = content;
        record->last_change = filewrite;
      }
    }
  }

  return 0;
}

int RecordTable::HeuristicGC() {
  time_t current_time = time(NULL);
  int count = 0;

  // ensure at least 10% space is availabe
  while (records_.size() > 0.9 * max_size_) {

    // try to release 25% space at a time.
    // But it is really an estimated value.
    time_t offset = (current_time - last_gc_cutdown_) / 4;

    // ensure offset not zero,
    // otherwise there would be an infinite loop
    if (offset == 0) offset = 1;

    count += GC(last_gc_cutdown_ + offset);
  }

  return count;
}

int RecordTable::GC(time_t oldest) {
  // no record older than oldest exits.
  if (oldest <= last_gc_cutdown_) {
    return 0;
  }

  // iterate through the table to remove out-of-date records.
  std::vector<UrlFprint> toremove;
  HashTable::iterator iterator = records_.begin();
  while (iterator != records_.end()) {
    if (iterator->second->last_access < oldest) {
      VisitingRecord* record = iterator->second;
      delete record;
      toremove.push_back(iterator->first);
    }
    ++iterator;
  }

  int count = 0;
  for (std::vector<UrlFprint>::iterator itr = toremove.begin();
  itr != toremove.end(); ++itr) {
    records_.erase(*itr);
    ++count;
  }

  last_gc_cutdown_ = oldest;
  return count;
}

int RecordTable::Save(const char *path) const {
  // put all records into a vector and sort them by url finger print
  std::vector<std::pair<UrlFprint, VisitingRecord*> > records(records_.begin(),
                                                              records_.end());
  std::sort(records.begin(), records.end());

  // Write the sorted records to file.
  RecordFileWriter* writer = RecordFileIOFactory::CreateWriter(path);
  if (writer == NULL) {
    return 1;
  }
  for (int i = 0, n = static_cast<int>(records.size()); i < n; ++i) {
    writer->Write(*records[i].second);
  }

  delete writer;
  return 0;
}

int RecordTable::Load(const char *path) {
  // clear all the old records
  Clear();

  RecordFileReader* reader = RecordFileIOFactory::CreateReader(path);
  if (reader == NULL) {
    return 1;
  }

  // read record from file
  VisitingRecord* record = new VisitingRecord();
  while (reader->Read(record) == 0) {
    HashTable::iterator itr = records_.find(record->fingerprint());

    // to avoid duplicated record in file
    if (itr != records_.end()) {
      delete itr->second;
    }

    records_[record->fingerprint()] = record;
    record = new VisitingRecord();
  }
  delete record;

  delete reader;
  return 0;
}

void RecordTable::Clear() {
  HashTable::const_iterator iterator = records_.begin();
  while (iterator != records_.end()) {
    delete iterator->second;
    ++iterator;
  }
  records_.clear();
}
