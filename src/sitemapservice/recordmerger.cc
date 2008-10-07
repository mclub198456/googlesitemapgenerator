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


#include "sitemapservice/recordmerger.h"

#include "common/url.h"
#include "common/port.h"
#include "common/logger.h"
#include "sitemapservice/recordfileio.h"
#include "sitemapservice/urlfprintio.h"
#include "sitemapservice/recordfilemanager.h"

RecordMerger::RecordMerger() {
  // does nothing.
}

RecordMerger::~RecordMerger() {
  // does nothing.
}


// sources means the temp record files, each file has many records
// assert the source files are all sorted by the fingerprint (url)
// 'cutdown' is in unit 'sec'
int RecordMerger::Merge(const std::string& destination,
                        const std::string& fp_dest,
                        const std::vector<std::string>& sources,
                        const std::set<UrlFprint>& obsoleted,
                        const time_t& cutdown,
                        RecordFileStat* stat) {
  int n = static_cast<int>(sources.size());

  std::vector<VisitingRecord> records(n);
  std::vector<RecordFileReader*> readers(n); // NOTE, reader can't be copy constructed.

  // open the writer
  UrlFprintWriter fpwriter;
  if (!fpwriter.Open(fp_dest.c_str())) {
    return 1;
  }
  RecordFileWriter* writer = RecordFileIOFactory::CreateWriter(destination);
  if (writer == NULL) {
    return 1;
  }

  // open all the files
  for (int i = 0, j = 0; i < n; ++i, ++j) {
    readers[i] = RecordFileIOFactory::CreateReader(sources[j]);
    if (readers[i] == NULL || readers[i]->Read(&records[i]) != 0) {
      if (readers[i] != NULL) delete readers[i];
      records.erase(records.begin() + i);
      readers.erase(readers.begin() + i);
      --i; --n;
    }
  }

  // start to estimate the record data
  stat->Reset();

  VisitingRecord record;
  std::set<UrlFprint>::const_iterator obsoleted_itr = obsoleted.begin();

  // While there leaves only one file, no need to do the merge,
  // just store the records.
  while (readers.size() > 1) {
    // Select the minimum fingerprint from the first records of every file.
    UrlFprint fingerprint = records[0].fingerprint();
    for (int i = 1; i < n; ++i) {
      if (fingerprint > records[i].fingerprint()) {
        fingerprint = records[i].fingerprint();
      }
    }
    
    bool flag = 0;
    for (int i = 0; i < n; ++i) {
      if (fingerprint == records[i].fingerprint()) {
        const VisitingRecord& another = records[i];
        if (flag) {
          // Not the first fingerprint record, merge to the first.
          RecordMerger::Merge(record, another);
        } else {
          // Save first fingerprint record.
          record = records[i];
          flag = 1;
        }

        // Next record from the same file.
        if (readers[i]->Read(&records[i]) != 0) {
          // No more data available, we should remove it.
          delete readers[i];
          readers.erase(readers.begin() + i);
          records.erase(records.begin() + i);

          --i;
          --n;
        }
      }
    }

    if (record.last_access >= cutdown) {
      while (obsoleted_itr != obsoleted.end() && *obsoleted_itr < fingerprint) {
          ++obsoleted_itr;
      }
      if (obsoleted_itr != obsoleted.end() && *obsoleted_itr == fingerprint) {
        continue; // this url is obsoleted, skip it.
      }

      writer->Write(record);
      fpwriter.Write(fingerprint);
      stat->AddRecord(record);
    }
  }  // End while (readers.size() > 1).
  
  if (readers.size() == 1) {
    do {
      // Cut down all the records that exceed the max_url_life
      if (records[0].last_access >= cutdown) {
        while (obsoleted_itr != obsoleted.end() &&
          *obsoleted_itr < records[0].fingerprint()) {
          ++obsoleted_itr;
        }
        if (obsoleted_itr != obsoleted.end()
          && *obsoleted_itr == records[0].fingerprint()) {
          continue; // this url is obsoleted, skip it.
        }

        writer->Write(records[0]);
        fpwriter.Write(records[0].fingerprint());
        stat->AddRecord(records[0]);
      }

      // NotifyHandlers(records[0].second);
    } while (readers[0]->Read(&records[0]) == 0);

    delete readers[0];
  }

  delete writer;
  return 0;
}


// the record table and the current file must be saved to historical files before this merge() is called
// so when merging, the record table and the current file can also be used by OnReceive new record.
// file system should be exclusive accessed.
// 'cutdown' is in unit 'sec'
int RecordMerger::Merge(RecordfileManager* filemanager,  const std::set<UrlFprint>& obsoleted,
                        int maxsize, const time_t& cutdown,
                        RecordFileStat* stat) {

  std::string oldbase = filemanager->GetBaseFile();

  // get all the historical data files
  std::vector<std::string> files = filemanager->GetTempFiles();
  if (files.size() == 0) {
    return RemoveOldRecords(oldbase, filemanager->GetFPFile(),
                            obsoleted, cutdown, stat);
  }

  files.push_back(oldbase);

  // create the new base file name
  std::string newbase(oldbase);
  newbase.append("__new_and_unique__");

  // merge all historical data files and base file into new base file
  if (Merge(newbase, filemanager->GetFPFile(), files, obsoleted, cutdown, stat) != 0) {
    return 1;
  }

  if (stat->GetTotalCount() > maxsize) {
    time_t cutdown = stat->GetCutDownTime(maxsize);
    RemoveOldRecords(newbase, filemanager->GetFPFile(),
      std::set<UrlFprint>(), cutdown, stat);
  }
  
  // remove all the old files
  for (int i = 0; i < static_cast<int>(files.size()); ++i) {
    remove(files[i].c_str());
  }

  filemanager->RemoveTempFiles(files);
  // replace old base file with new base file
  return rename(newbase.c_str(), oldbase.c_str());
}

int RecordMerger::RemoveOldRecords(const std::string& recordfile,
                                   const std::string& fpfile,
                                   const std::set<UrlFprint>& obsoleted,
                                   const time_t& cutdown,
                                   RecordFileStat* stat) {
  std::string swapfile(recordfile);
  swapfile.append("_merger_to_swap");

  // Open fpfile to write.
  UrlFprintWriter fpwriter;
  if (!fpwriter.Open(fpfile.c_str())) {
    return -1;
  }

  // Open old record file.
  RecordFileReader* reader = RecordFileIOFactory::CreateReader(recordfile);
  if (reader == NULL) {
    return 1;
  }

  // Open new record file.
  RecordFileWriter* writer = RecordFileIOFactory::CreateWriter(swapfile);
  if (writer == NULL) {
    delete reader;
    return 2;
  }

  // do removing
  VisitingRecord record;
  stat->Reset();

  std::set<UrlFprint>::const_iterator itr = obsoleted.begin();
  while (reader->Read(&record) == 0) {
    while (itr != obsoleted.end() && (*itr) < record.fingerprint()) {
      ++itr;
    }
    if (itr != obsoleted.end() && (*itr) == record.fingerprint()) {
      ++itr;
    } else if (record.last_access > cutdown) {
      stat->AddRecord(record);
      writer->Write(record);
      fpwriter.Write(record.fingerprint());
    }
  }

  fpwriter.Close();
  delete reader;
  delete writer;

  // replace old file with new file
  remove(recordfile.c_str());
  rename(swapfile.c_str(), recordfile.c_str());
  return 0;
}


bool RecordMerger::MergeUrlFprint(const std::string& dest,
                                  const std::vector<std::string> srcs,
                                  const std::set<UrlFprint>& obsoleted) {
  UrlFprintWriter writer;
  if (!writer.Open(dest.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to open [%s] to write url fprint.",
                           dest.c_str());
    return false;
  }

  int n = static_cast<int>(srcs.size());

  std::vector<UrlFprint> fprints(n);
  std::vector<UrlFprintReader*> readers(n);

  // open all the files to read.
  for (int i = 0; i < n; ++i) {
    readers[i] = new UrlFprintReader();
    if (!readers[i]->Open(srcs[i].c_str()) || !readers[i]->Read(&fprints[i])) {
      delete readers[i];
      readers.erase(readers.begin() + i);
      fprints.erase(fprints.begin() + i);
      --i;
      --n;
    }
  }

  UrlFprint fingerprint = 0;
  std::set<UrlFprint>::const_iterator obsoleted_itr = obsoleted.begin();

  while (readers.size() > 1) {
    fingerprint = fprints[0];
    for (int i = 1; i < n; ++i) {
      if (fingerprint > fprints[i]) {
        fingerprint = fprints[i];
      }
    }
    
    for (int i = 0; i < n; ++i) {
      if (fingerprint == fprints[i]) {
        // next record from the same file 
        if (!readers[i]->Read(&fprints[i])) {
          delete readers[i];
          readers.erase(readers.begin() + i);
          fprints.erase(fprints.begin() + i);

          --i, --n;
        }
      }
    }

    while (obsoleted_itr != obsoleted.end() && *obsoleted_itr < fingerprint) {
        ++obsoleted_itr;
    }
    if (obsoleted_itr != obsoleted.end() && *obsoleted_itr == fingerprint) {
      continue; // this url is obsoleted, skip it.
    }

    writer.Write(fingerprint);
  }
  
  if (readers.size() == 1) {
    do {
        while (obsoleted_itr != obsoleted.end() && *obsoleted_itr < fprints[0]) {
            ++obsoleted_itr;
        }
        if (obsoleted_itr != obsoleted.end() && *obsoleted_itr == fprints[0]) {
          continue; // this url is obsoleted, skip it.
        }

        writer.Write(fprints[0]);
    } while (readers[0]->Read(&fprints[0]));

    delete readers[0];
  }

  writer.Close();
  return true;
}

void RecordMerger::Merge(VisitingRecord& record, const VisitingRecord& another) {
  record.count_access += another.count_access;
  
  if (record.first_appear > another.first_appear) {
    record.first_appear = another.first_appear;
  }

  record.count_change += another.count_change;
  if (record.last_content != another.last_content) {
    if (record.last_access < another.last_access) {
      record.last_content = another.last_content;
      record.last_change = another.last_change;
    }
  } else {
    --record.count_change;
  }

  if (record.last_access < another.last_access) {
    record.last_access = another.last_access;
  }

}
