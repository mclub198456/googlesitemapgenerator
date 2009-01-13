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


#include "sitemapservice/newsdatamanager.h"

#include <errno.h>
#include "common/logger.h"
#include "common/fileutil.h"
#include "sitemapservice/urlfprintio.h"

const std::string NewsDataManager::kDataFile = "new_entries";
const std::string NewsDataManager::kFprintFile = "old_fprint";
const int NewsDataManager::kMaxNewsEntry = 64 * 1024;

NewsDataManager::NewsDataManager() {
  // does nothing.
}

NewsDataManager::~NewsDataManager() {
  // does nothing.
}

bool NewsDataManager::Initialize(SiteDataManager* sitedata_mgr, const std::string& dir) {
  sitedata_manager_ = sitedata_mgr;
  time(&last_update_);
  
  data_dir_ = sitedata_mgr->GetFileManager()->GetDirectory();
  data_dir_.append(dir).append("/");
  if (FileUtil::CreateDir(data_dir_.c_str()) == false) {
    Logger::Log(EVENT_ERROR, "Failed to create data dir for news sitemap.");
    return false;
  }

  std::string base_fp = sitedata_manager_->GetFileManager()->GetFPFile();
  std::string news_fp = data_dir_ + std::string("old_fprint");
  if (FileUtil::Exists(base_fp.c_str()) && !FileUtil::Exists(news_fp.c_str())) {
    FileUtil::CopyFile(base_fp.c_str(), news_fp.c_str());
  }

  return true;
}

bool NewsDataManager::UpdateData() {
  // Flush the data in memory.
  if (!sitedata_manager_->SaveMemoryData(true, true)) {
    Logger::Log(EVENT_ERROR, "Failed to save memory data for news sitemap.");
    return false;
  }

  if (!sitedata_manager_->LockDiskData(true)) {
    Logger::Log(EVENT_ERROR, "Failed to lock site data to update news data.");
    return false;
  }

  RecordfileManager* filemanager = sitedata_manager_->GetFileManager();
  std::vector<std::string> current_temps =
    filemanager->GetTempFiles(last_update_, time(NULL));

  std::string old_entries(data_dir_);
  old_entries.append(kDataFile);
  std::string new_entries(old_entries);
  new_entries.append("_new");

  std::string old_fprint(data_dir_);
  old_fprint.append(kFprintFile);
  std::string new_fprint(old_fprint);
  new_fprint.append("_new");

  bool result = false;
  do {
    if (current_temps.size() == 0) {
      Logger::Log(EVENT_CRITICAL, "No new record to update news data.");
      result = true;
      break;
    }

    if (!MergeUrlFprint(new_entries, new_fprint, old_fprint, current_temps)) {
      Logger::Log(EVENT_ERROR, "Failed to merge latest temp files.");
      break;
    }

    FileUtil::DeleteFile(old_fprint.c_str());
    if (rename(new_fprint.c_str(), old_fprint.c_str()) != 0) {
      Logger::Log(EVENT_ERROR, "Failed to update news finger print, fatal. (%d)",
                errno);
      break;
    }

    // Get the latest new entries and save them.
    Heap heap;
    if (!AddRecordToHeap(old_entries, &heap)) {
      Logger::Log(EVENT_ERROR, "Failed to add records to heap (%s).",
                  old_entries.c_str());
      break;
    }
    if (!AddRecordToHeap(new_entries, &heap)) {
      Logger::Log(EVENT_ERROR, "Failed to add records to heap (%s).",
                  new_entries.c_str());
      break;
    }
    if (!SaveHeap(&heap, old_entries)) {
      Logger::Log(EVENT_ERROR, "Failed to save news entries to file (%s).",
                  old_entries.c_str());
      break;
    }

    result = true;
  } while (false);

  sitedata_manager_->UnlockDiskData();
  time(&last_update_);

  return result;
}

bool NewsDataManager::AddRecordToHeap(const std::string& file, Heap* heap) {
  if (!FileUtil::Exists(file.c_str())) {
    Logger::Log(EVENT_CRITICAL, "No records file for heap. (%s)",
                file.c_str());
    return true;
  }

  // Open the file to read.
  RecordFileReader* reader = RecordFileIOFactory::CreateReader(file);
  if (reader == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to create reader for %s.", file.c_str());
    return false;
  }

  VisitingRecord record;
  while (reader->Read(&record) == 0) {
    heap->push(record);
    if (heap->size() > kMaxNewsEntry) {
      heap->pop();
    }
  }
  delete reader;
  return true;
}

bool NewsDataManager::SaveHeap(Heap* heap, const std::string& file) {
  // Open the file to write.
  RecordFileWriter* writer = RecordFileIOFactory::CreateWriter(file);
  if (writer == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to create writer for %s.", file.c_str());
    return false;
  }

  while (!heap->empty()) {
    writer->Write(heap->top());
    heap->pop();
  }
  delete writer;
  return true;
}

std::string NewsDataManager::GetDataFile() {
  std::string file(data_dir_);
  file.append(kDataFile);
  return file;
}

std::string NewsDataManager::GetDirectory() {
  return data_dir_;
}


bool NewsDataManager::MergeUrlFprint(const std::string& newentry,
                                     const std::string& new_fp,
                                     const std::string& old_fp,
                                     const std::vector<std::string> srcs) {

  // Open the final url fprint file to write.
  UrlFprintWriter fp_writer;
  if (!fp_writer.Open(new_fp.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to open [%s] to write url fprint.",
                           new_fp.c_str());
    return false;
  }

  // Open the original url fprint file to read.
  UrlFprintReader fp_reader;
  UrlFprint old_fprint;
  fp_reader.Open(old_fp.c_str());
  bool has_old = fp_reader.Read(&old_fprint);

  // Open the new url record entries file to write.
  RecordFileWriter* writer = RecordFileIOFactory::CreateWriter(newentry);
  if (writer == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open [%s] to write new URLs.");
    return false;
  }

  // Open the new temp files to read.
  int n = static_cast<int>(srcs.size());
  std::vector<VisitingRecord> records(n);
  std::vector<RecordFileReader*> readers(n);

  for (int i = 0, j = 0; i < n; ++i, ++j) {
    readers[i] = RecordFileIOFactory::CreateReader(srcs[j]);
    // No record is availalbe, simply remove the reader.
    if (readers[i] == NULL || readers[i]->Read(&records[i]) != 0) {
      if (readers[i] != NULL) {
        delete readers[i];
      }
      readers.erase(readers.begin() + i);
      records.erase(records.begin() + i);
      --i, --n;
    }
  }

  UrlFprint fingerprint = 0;
  VisitingRecord record;
  while (readers.size() > 0) {
    fingerprint = records[0].fingerprint();
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
          RecordMerger::Merge(record, another);
        } else {
          record = records[i];
          flag = true;
        }

        // Proceeds to the next record.
        if (readers[i]->Read(&records[i]) != 0) {
          delete readers[i];
          readers.erase(readers.begin() + i);
          records.erase(records.begin() + i);
          --i, --n;
        }
      }
    }

    while (has_old && old_fprint < fingerprint) {
      fp_writer.Write(old_fprint);
      has_old = fp_reader.Read(&old_fprint);
    }
    // Only save the entries we've never met before.
    if (has_old == false || old_fprint > fingerprint) {
      writer->Write(record);
      fp_writer.Write(fingerprint);
    }
  }

  // Add all the left old fprints.
  while (has_old) {
    fp_writer.Write(old_fprint);
    has_old = fp_reader.Read(&old_fprint);
  }

  delete writer;

  return true;
}
