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


// A class used to manage record files.
// A record file is used to store url visiting records.
// There are many kinds of record files: current file, base file and 

#ifndef SITEMAPSERVICE_RECORDFILEMANAGER_H__
#define SITEMAPSERVICE_RECORDFILEMANAGER_H__

#include <string>
#include <vector>

#include "common/basictypes.h"
#include "common/criticalsection.h"

class RecordfileManager {
  
 public:
  // set the dir to hold all record data files
  static void SetRecordfileHome(const char* dir) {
    recordfile_home_ = std::string(dir);
  }

  RecordfileManager();
  ~RecordfileManager();

  // host -- the host url.
  // Note, http://www.example.com and www.example.com is not same here.
  // A directory under {SERVICE_HOME}/data will be used for the given host.
  bool Initialize(const char* siteid, int64 max_tempsize);

  // For backward compatibility, no restriction is put max_tempsize.
  bool Initialize(const char* siteid);

  // A directory whose name is like the host name will be created under dir.
  // Or if the dir exists, it will be reused.
  bool Initialize(const char* dir, const char* siteid, int64 max_tempsize);

  // following methods can only be called after initialization.
  std::string GetDirectory();

  std::string GetBaseFile();

  std::string GetCurrentFile();

  std::string GetHostFile();

  std::string GetFPFile();

  std::vector<std::string> GetTempFiles();

  // Get all temporary files in the ranges of [begin, end].
  std::vector<std::string> GetTempFiles(time_t begin, time_t end);

  void RemoveTempFiles(const std::vector<std::string>& files);

  void RemoveTempFile(const std::string& file);

  bool CompleteCurrentFile();

  int64 GetTempFilesSize();

  void CleanUpTempFile();

  int64 GetMaxTempFilesSize() {
    return max_tempsize_;
  }

 private:
  struct TempFile {
    std::string name;
    time_t write_time;
    int64 size;

    bool operator<(const TempFile& another) const {
      if (write_time != another.write_time) {
        return write_time < another.write_time;
      } else {
        return name < another.name;
      }
    }
  };

  static const std::string kBaseFile;
  static const std::string kHostFile;
  static const std::string kTempFilePrefix;
  static const std::string kCurrentFile;
  static const std::string kFPFile;

  // the dir to store all the record data files by default
  // Record data for {host} will be stored in {record_file_home}/{host}.
  // When it is not set, Util::GetApplicationDir() will be used as default.
  static std::string recordfile_home_;

  // find all files in dir whose name starts with prefix
  bool FindFiles(const std::string& dir, const std::string& prefix,
                std::vector<std::string>* files);

  int64 max_tempsize_;

  // the directory used to hold record file for given host
  std::string directory_;

  std::vector<TempFile> temp_files_;

  CriticalSection lock_;

  DISALLOW_EVIL_CONSTRUCTORS(RecordfileManager);
};

#endif // SITEMAPSERVICE_RECORDFILEMANAGER_H__

