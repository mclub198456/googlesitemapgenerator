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


#include "sitemapservice/recordfilemanager.h"

#include <errno.h>
#include <algorithm>

#if defined(__linux__) || defined(__linux__)
#include <ctype.h>  // isalnum
#endif

#include "common/port.h"
#include "common/logger.h"
#include "common/util.h"
#include "common/fileutil.h"

const std::string RecordfileManager::kBaseFile = "data_base";
const std::string RecordfileManager::kTempFilePrefix = "data_temp_";
const std::string RecordfileManager::kCurrentFile = "data_current";
const std::string RecordfileManager::kFPFile = "data_fp";
const std::string RecordfileManager::kHostFile = "data_host";


// initialize the static fields.
std::string RecordfileManager::recordfile_home_ = "";

RecordfileManager::RecordfileManager() {
  // empty
}

RecordfileManager::~RecordfileManager() {
  // empty
}

bool RecordfileManager::Initialize(const char *siteid) {
  return Initialize(siteid, -1);
}

bool RecordfileManager::Initialize(const char *siteid, int64 max_tempsize) {
  // TODO: for backward compatibility, the default value for home is AppDir.
  // This should be removed in the future, and app must set it when starting.
  std::string home = RecordfileManager::recordfile_home_;
  if (home.length() == 0) {
    home.append(Util::GetApplicationDir());
    home.append("/cache/");
  }

  return Initialize(home.c_str(), siteid, max_tempsize);
}

// file save to dir/siteid/, replace the non [a-zA-Z0-9] char in siteid to '_'
bool RecordfileManager::Initialize(const char* dir, const char* siteid,
                                   int64 max_tempsize) {
  max_tempsize_ = max_tempsize;

  std::string dirname(siteid);
  for (int i = 0; i < static_cast<int>(dirname.length()); ++i) {
    if (!isalnum(dirname[i])) {
      dirname[i] = '_';
    }
  }

  directory_.clear();
  directory_.append(dir).append("/").append(dirname).append("/");
  if (!FileUtil::CreateDir(directory_.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed initialize record file dir [%s].",
              directory_.c_str());
    return false;
  }

  // Load all the old values in time order.
  std::vector<std::string> file_names;
  if (!FindFiles(directory_, kTempFilePrefix, &file_names)) {
    Logger::Log(EVENT_ERROR, "Failed to list old temp files.");
    return false;
  }
  for (int i = 0; i < static_cast<int>(file_names.size()); ++i) {
    FileAttribute file_attr;
    std::string full_name(directory_);
    full_name.append(file_names[i]);
    if (!FileUtil::GetFileAttribute(full_name.c_str(), &file_attr)) {
      Logger::Log(EVENT_ERROR, "Failed to access file [%s]. Skip.",
        file_names[i].c_str());
    } else {
      TempFile tempfile;
      tempfile.name = file_names[i];
      tempfile.size = file_attr.size;
      tempfile.write_time = file_attr.last_modified;
      temp_files_.push_back(tempfile);
    }
  }
  std::sort(temp_files_.begin(), temp_files_.end());

  return true;
}

std::string RecordfileManager::GetDirectory() {
  return directory_;
}

std::string RecordfileManager::GetHostFile() {
  std::string file = directory_;
  file.append(kHostFile);
  return file;
}

std::string RecordfileManager::GetBaseFile() {
  std::string file = directory_;
  file.append(kBaseFile);
  return file;
}

std::string RecordfileManager::GetCurrentFile() {
  std::string file = directory_;
  file.append(kCurrentFile);
  return file;
}

std::string RecordfileManager::GetFPFile() {
  std::string file = directory_;
  file.append(kFPFile);
  return file;
}

std::vector<std::string> RecordfileManager::GetTempFiles() {
  lock_.Enter(true);
  std::vector<std::string> result;
  for (int i = 0, n = static_cast<int>(temp_files_.size()); i < n; ++i) {
    result.push_back(directory_ + temp_files_[i].name);
  }
  lock_.Leave();

  return result;
}

std::vector<std::string> RecordfileManager::GetTempFiles(time_t begin, time_t end) {
  char buffer[128];
  strftime(buffer, 128, "%Y%m%d%H%M%S", localtime(&begin));
  std::string begin_str(buffer);
  strftime(buffer, 128, "%Y%m%d%H%M%S", localtime(&end));
  std::string end_str(buffer);

  std::vector<std::string> result;
  lock_.Enter(true);
  for (int i = 0, n = static_cast<int>(temp_files_.size()); i < n; ++i) {
    // Get "%Y%m%d%H%M%S" part.
    std::string temp = temp_files_[i].name.substr(kTempFilePrefix.length(), 14);
    if (temp >= begin_str && temp <= end_str) {
      result.push_back(temp_files_[i].name);
    }
  }
  lock_.Leave();

  for (int i = 0, n = static_cast<int>(result.size()); i < n; ++i) {
    result[i] = directory_ + result[i];
  }

  return result;
}

void RecordfileManager::RemoveTempFiles(const std::vector<std::string>& files) {
  // Lock the temp files vector.
  lock_.Enter(true);
  AutoLeave auto_leave(&lock_);

  for (int i = 0; i < static_cast<int>(files.size()); ++i) {
    // Extract only the file name.
    std::string::size_type pos = files[i].find_last_of("\\/");
    std::string name;
    if (pos == std::string::npos) {
      name.append(files[i]);
    } else {
      name.append(files[i].substr(pos + 1));
    }

    for (int j = 0; j < static_cast<int>(temp_files_.size()); ++j) {
      if (temp_files_[j].name == name) {
        temp_files_.erase(temp_files_.begin() + j);
      }
    }
  }
}

void RecordfileManager::RemoveTempFile(const std::string& file) {
  std::vector<std::string> files(1, file);
  RemoveTempFiles(files);
}


// mv 'data_current' to 'data_temp_yymmddhhmmss_0~999'
bool RecordfileManager::CompleteCurrentFile() {
  // Lock the temp files vector.
  lock_.Enter(true);
  AutoLeave auto_leave(&lock_);

  // use timestamp as part of file name
  time_t current = time(NULL);
  char buffer[128];
  strftime(buffer, 128, "%Y%m%d%H%M%S", localtime(&current));
  std::string newfile(kTempFilePrefix);
  newfile.append(buffer);

  // try to get a complete new file name
  std::string nextfile = newfile;
  for (int cnt = 0; ; ++cnt) {
    int i = 0;
    for (; i < static_cast<int>(temp_files_.size()); ++i) {
      if (temp_files_[i].name == nextfile) {
        break;
      }
    }
    if (i == temp_files_.size()) {
      break;
    }

    nextfile = newfile;
    itoa(cnt, buffer);
    nextfile.append("_").append(buffer);
  }

  // Construct full file name.
  std::string currentfile = GetCurrentFile();
  std::string nextfile_full(directory_);
  nextfile_full.append(nextfile);

  // ensure the destination file doesn't exist.
  remove(nextfile_full.c_str());
  if (rename(currentfile.c_str(), nextfile_full.c_str()) != 0) {
    Logger::Log(EVENT_ERROR, "Failed to complete file [%s]. (%d)",
              nextfile_full.c_str(), errno);
    return false;
  }

  FileAttribute file_attr;
  if (!FileUtil::GetFileAttribute(nextfile_full.c_str(), &file_attr)) {
    Logger::Log(EVENT_ERROR, "Failed to access new temp file [%s].",
              nextfile_full.c_str());
  }

  TempFile new_tempfile;
  new_tempfile.name = nextfile;
  new_tempfile.size = file_attr.size;
  new_tempfile.write_time = file_attr.last_modified;
  temp_files_.push_back(new_tempfile);
  return true;
}

int64 RecordfileManager::GetTempFilesSize() {
  int64 size = 0;

  lock_.Enter(true);
  for (int i = 0; i < static_cast<int>(temp_files_.size()); ++i) {
    size += temp_files_[i].size;
  }
  lock_.Leave();
  return size;
}

void RecordfileManager::CleanUpTempFile() {
  lock_.Enter(true);
  AutoLeave autoleave_lock(&lock_);

  if (max_tempsize_ == -1) {
    return;
  }

  // Check the limitation.
  int64 size = 0;
  int i = static_cast<int>(temp_files_.size()) - 1;
  for (; i >= 0; --i) {
    size += temp_files_[i].size;
    if (size > max_tempsize_) {
      break;
    }
  }

  // Remove old files from disk.
  for (int j = 0; j <= i; ++j) {
    std::string full_name(directory_);
    full_name.append("/").append(temp_files_[j].name);
    if (!FileUtil::DeleteFile(full_name.c_str())) {
      Logger::Log(EVENT_ERROR, "Failed to delete temp file [%s] (ignore)",
                full_name.c_str());
    }
  }

  // Remove old files from internal list.
  if (i >= 0) {
    temp_files_.erase(temp_files_.begin(), temp_files_.begin() + i + 1);
    Logger::Log(EVENT_IMPORTANT, "[%d] temp files are cleaned from [%s].",
      i + 1, directory_.c_str());
  }
}

#ifdef WIN32

// WIN32 implementation of FindFiles
bool RecordfileManager::FindFiles(const std::string& dir,
                                 const std::string& prefix,
                                 std::vector<std::string>* files) {
  std::string pattern(dir);
  pattern.append("/").append(prefix).append("*");

  files->clear();
  WIN32_FIND_DATAA finddata;
  HANDLE handle = FindFirstFileA(pattern.c_str(), &finddata);
  if (handle == INVALID_HANDLE_VALUE) {
    if (ERROR_FILE_NOT_FOUND != GetLastError()) {
      Logger::Log(EVENT_ERROR, "Failed to list dir with [%s]. (%d)",
                pattern.c_str(), GetLastError());
      return false;
    } else {
      return true;
    }
  }

  files->push_back(finddata.cFileName);
  while (FindNextFileA(handle, &finddata)) {
    files->push_back(finddata.cFileName);
  }
  FindClose(handle);

  return true;
}

#elif defined(__linux__) || defined(__unix__)
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

bool RecordfileManager::FindFiles(const std::string&dir,
                                 const std::string& prefix,
                                 std::vector<std::string>* files) {
  DIR* dirhandle = opendir(dir.c_str());
  if (dirhandle == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open dir [%s]. (%d)",
              dir.c_str(), errno);
    return false;
  }
  
  files->clear();
  size_t prefixlen = static_cast<size_t>(prefix.length());

  struct dirent *entry = readdir(dirhandle);
  while (entry != NULL) {
    if (strncmp(entry->d_name, prefix.c_str(), prefixlen) == 0) {
      files->push_back(entry->d_name);
    }
    entry = readdir(dirhandle);
  }
  
  closedir(dirhandle);
  return true;
}


#endif
