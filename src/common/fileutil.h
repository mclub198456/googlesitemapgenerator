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


// This class defines portalbe common file operations. Most of them are built
// on system APIs, and some of them are directly built on C standard IO
// functions.

#ifndef COMMON_FILEUTIL_H__
#define COMMON_FILEUTIL_H__

#include "common/basictypes.h"

#include <string>
#include <vector>

// If some other attributes are required, add it freely with updating
// GetFileAttribute method below.
struct FileAttribute {
  bool is_dir;
  int64 size;
  time_t last_modified;
};

class FileUtil {
 public:
  // Check whether given "path" exist, which could be a file or a directory.
  static bool Exists(const char* path);

  // Create directory with "dirname".
  // The parent directory must exist, otherwise, false is returned.
  // If the dir already exists, simply true is returned.
  static bool CreateDir(const char* dirname);

  // Remove directory with "dirname".
  // The directory can't be removed recursively.
  // But the directory can contain files, which are removed together.
  static bool RemoveDir(const char* dirname);

  // Load/write given contents to the file
  static bool LoadFile(const char* filename, std::string* content);
  static bool WriteFile(const char* filename, const std::string& content);

  // Load/save file contents as lines.
  static bool LoadLines(const char* filename, std::vector<std::string>* lines);
  static bool SaveLines(const char* filename,
                        const std::vector<std::string>& lines);

  // Get file attribute.
  // The path could be a common file or a directory.
  static bool GetFileAttribute(const char* path, FileAttribute* attribute);

  // List a dir,
  // if dirs param is not null, child dir names are pushed into it.
  // if files param is not null, child file names are pushed into it.
  // if onlyname is true, only name with no path prefix is returned.
  static bool ListDir(const char* dir, bool onlyname,
                      std::vector<std::string>* dirs,
                      std::vector<std::string>* files);

  // Get temp file name in system temporary directory.
  static bool MakeTemp(std::string* temp);

  // Copy a file from "src" to "dest".
  // If "dest" exists, it will be over written.
  static bool CopyFile(const char* src, const char* dest);

  // Move a file from "src" to "dest".
  static bool MoveFile(const char* src, const char* dest);

  // Delete the "file".
  // If the file doesn't exist, true is returned.
  static bool DeleteFile(const char* file);

  // Find the parent dir of this path.
  // Path seperator is left in the end of result path.
  static bool ParentDir(std::string* path);

#ifdef WIN32
  static const char kPathSeparator = '\\';
#else
  static const char kPathSeparator = '/';
#endif

 private:
  FileUtil() {}
};

#endif  // COMMON_FILEUTIL_H__
