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


#include "common/fileutil.h"
#include "common/logger.h"

#ifdef WIN32

#include <io.h>
#include <windows.h>

#elif defined(__linux__) || defined(__unix__)

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string>

bool FileUtil::ParentDir(std::string* path) {
  // Ignore the last path separator.
  std::string::size_type pos = path->find_last_not_of("/\\");
  if (pos == std::string::npos) return false;
  path->erase(pos);

  pos = path->find_last_of("/\\");
  if (pos == std::string::npos) return false;
  path->erase(pos);
 
  path->push_back(kPathSeparator);
  return true;
 }

bool FileUtil::Exists(const char* path) {
  FileAttribute attr;
  return GetFileAttribute(path, &attr);
}

bool FileUtil::DeleteFile(const char* filename) {
  if (Exists(filename)) {
#ifdef WIN32
    if (::DeleteFileA(filename) != TRUE) {
      Logger::Log(EVENT_ERROR, "Failed to remove file [%s]. (%d)",
                  filename, GetLastError());
      return false;
    }
#else
    if (remove(filename) != 0) {
      Logger::Log(EVENT_ERROR, "Failed to remove file %s. (%d)", filename, errno);
      return false;
    }
#endif
    return true;
  }

  remove(filename);
  return true;
}

bool FileUtil::CopyFile(const char* src, const char* dest) {
  // Open the file to read and write.
  FILE* in = fopen(src, "rb");
  FILE* out = fopen(dest, "wb");
  if (in == NULL || out == NULL) {
    return false;
  }

  // Read content from src and write it to dest.
  char buffer[1024];
  while (!ferror(in) && !ferror(out) && !feof(in)) {
    size_t count = fread(buffer, sizeof(char), 1024, in);
    if (!ferror(in) && count != 0) {
      fwrite(buffer, sizeof(char), count, out);
    }
  }

  // Check the result.
  bool success = !ferror(in) && !ferror(out);
  if (!success) {
    Logger::Log(EVENT_ERROR, "Failed copy (%s) to (%s). (%d)", src, dest, errno);
  }

  fclose(in);
  fclose(out);
  return success;
}

bool FileUtil::MoveFile(const char* src, const char* dest) {
  if (!FileUtil::CopyFile(src, dest)) {
    Logger::Log(EVENT_ERROR, "Failed move file because of copy failure.");
    return false;
  }
  if (!FileUtil::DeleteFile(src)) {
    Logger::Log(EVENT_ERROR,
              "Failed to move file because of deleting original file failure.");
    return false;
  }

  return true;
}

bool FileUtil::LoadFile(const char* filename, std::string* content) {
  // File size limitation is 1M.
  const int kMaxFileSize = (1 << 20);

  FILE* file = fopen(filename, "rb");
  if( file == NULL ) {
    return false;
  }

  do {
    // Get the length of file.
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length < 0) {
      Logger::Log(EVENT_ERROR, "LoadFile[%s] failed on ftell. (%d)",
                  filename, errno);
      break;
    }
    if (length > kMaxFileSize) {
      Logger::Log(EVENT_ERROR, "File[%s] size[%d] too large", filename, length);
      break;
    }

    // Check if it is a UTF-8 format file.
    if (length >= 3) {
      char buffer[3];

      if (fread(buffer, sizeof(char), 3, file) != 3)
        break;

      // If first 3 bytes of a file is UTF-8 marker, we should skip it.
      // Otherwise, we set read pointer to the begining of the file.
      if (buffer[0] == '\xef' && buffer[1] == '\xbb' && buffer[2] == '\xbf') {
        length -= 3;
      } else {
        fseek(file, 0, SEEK_SET);
      }
    }

    content->resize(length);
    if (content->length() != length)
      break;

    if (fread((char *)content->c_str(), sizeof(char), length, file) != length)
      break;

    fclose(file);
    return true;
  } while (false);

  fclose(file);
  return false;
}

bool FileUtil::WriteFile(const char* filename, const std::string& content) {
  // Open file.
  FILE* file = fopen(filename, "wb");
  if( file == NULL ) {
    return false;
  }

  // Write content.
  if (fwrite(content.c_str(), sizeof(char),
        content.length(), file) != content.length()) {
    fclose(file);
    return false;
  }

  fclose(file);
  return true;
}

#ifdef WIN32

bool FileUtil::MakeTemp(std::string* temp) {
  // The temp directory should be "$windows_dir\\Temp".
  std::string temp_dir;
  char win_dir[1024];
  int size = GetWindowsDirectoryA(win_dir, 1024);
  if (size == 0 || size >= 1024) {
    Logger::Log(EVENT_ERROR, "Failed to get win dir. (%d).", GetLastError());
    return false;
  }
  temp_dir.append(win_dir).append("\\Temp\\");

  // Create a temp file name.
  char temp_file[MAX_PATH + 1];
  if (GetTempFileNameA(temp_dir.c_str(), "gsg", 0, temp_file) == 0) {
    Logger::Log(EVENT_ERROR, "Failed to create temp file. (%d)",
                GetLastError());
    return false;
  }

  temp->assign(temp_file);

  return true;
}

bool FileUtil::CreateDir(const char* dirname) {
  bool result = CreateDirectoryA(dirname, NULL) != 0;
  if (!result) {
    if (GetLastError() == 183) {
      result = true;
    } else {
      Logger::Log(EVENT_ERROR, "Failed to create dir [%s]. (%d)",
                dirname, GetLastError());
    }
  }

  return result;
}

bool FileUtil::RemoveDir(const char* dirname) {
  std::string pattern(dirname);
  pattern.append("/*");

  // Search all the files under path.
  WIN32_FIND_DATAA finddata;
  HANDLE handle = FindFirstFileA(pattern.c_str(), &finddata);
  if (handle == INVALID_HANDLE_VALUE) {
    return false;
  }

  // Remove all the files under directory.
  std::string nextfile(dirname);
  nextfile.append("/").append(finddata.cFileName);
  DeleteFileA(nextfile.c_str());

  while (FindNextFileA(handle, &finddata)) {
    nextfile = dirname;
    nextfile.append("/").append(finddata.cFileName);
    DeleteFileA(nextfile.c_str());
  }
  FindClose(handle);

  // Remove the directory itself.
  return RemoveDirectoryA(dirname) == 0;
}

bool FileUtil::GetFileAttribute(const char* path, FileAttribute* attr) {
  if (path == NULL) return false;

  WIN32_FILE_ATTRIBUTE_DATA attribute_data;
  if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attribute_data)) {
    return false;
  }

  // Check whether it is a dir.
  attr->is_dir = (attribute_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                 == FILE_ATTRIBUTE_DIRECTORY;

  // Get size info.
  LARGE_INTEGER largeint;
  largeint.HighPart = attribute_data.nFileSizeHigh;
  largeint.LowPart = attribute_data.nFileSizeLow;
  attr->size = largeint.QuadPart;

  // Get last write info.
  largeint.HighPart = attribute_data.ftLastWriteTime.dwHighDateTime;
  largeint.LowPart = attribute_data.ftLastWriteTime.dwLowDateTime;
  attr->last_modified = largeint.QuadPart / 10000000 - 11644473600LL;
  if (attr->last_modified < 0) return false;

  return true;
}

bool FileUtil::ListDir(const char* dir, bool onlyname,
                       std::vector<std::string>* dirs,
                       std::vector<std::string>* files) {
  // Use pattern ${dir}/* to search all items in dir.
  std::string pattern(dir);
  pattern.append("/").append("*");

  WIN32_FIND_DATAA finddata;
  HANDLE handle = FindFirstFileA(pattern.c_str(), &finddata);
  if (handle == INVALID_HANDLE_VALUE) {
    Logger::Log(EVENT_ERROR, "Failed to list dir %s. (%d).",
              dir, GetLastError());
    return false;
  }

  // Walk through all items under dir.
  do {
    if (strcmp(finddata.cFileName, ".") == 0 ||
        strcmp(finddata.cFileName, "..") == 0) {
      continue;
    }

    if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      // Save child file name.
      if (dirs != NULL) {
        if (onlyname) {
          dirs->push_back(finddata.cFileName);
        } else {
          std::string fullchild(dir);
          fullchild.append("/").append(finddata.cFileName);
          dirs->push_back(fullchild);
        }
      }
    } else {
      // Save child directory name.
      if (files != NULL) {
        if (onlyname) {
          files->push_back(finddata.cFileName);
        } else {
          std::string fullfile(dir);
          fullfile.append("/").append(finddata.cFileName);
          files->push_back(fullfile);
        }
      }
    }
  } while (FindNextFileA(handle, &finddata));

  FindClose(handle);
  return true;
}

#elif defined(__linux__) || defined(__unix__)

bool FileUtil::MakeTemp(std::string* temp) {
  char temp_file[32] = "/tmp/sitemap_tmp_XXXXXX";
  int file_desc = mkstemp(temp_file);
  if (file_desc == -1) {
    Logger::Log(EVENT_ERROR, "Failed to create temp file. (%d)", errno);
    return false;
  } else {
    close(file_desc);
  }

  *temp = temp_file;
  return true;
}

bool FileUtil::CreateDir(const char *dirname) {
  int result = mkdir(dirname, 0777);
  if (result != 0) {
    if (errno == EEXIST) result = 0;
  }

  return  result == 0;
}

bool FileUtil::RemoveDir(const char* dirname) {
  DIR* dirhandle = opendir(dirname);
  if (dirhandle == NULL) {
    return false;
  }

  struct dirent *entry = readdir(dirhandle);
  while (entry != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      std::string fullpath(dirname);
      fullpath.append("/").append(entry->d_name);
      remove(fullpath.c_str());
    }
    entry = readdir(dirhandle);
  }
  closedir(dirhandle);

  return rmdir(dirname) == 0;
}

bool FileUtil::GetFileAttribute(const char* path, FileAttribute* attr) {
  struct stat filestatus;
  if (stat(path, &filestatus) != 0) {
    Logger::Log(EVENT_NORMAL, "Failed to stat file [%s] (%d).", path, errno);
    return false;
  }

  attr->is_dir = S_ISDIR(filestatus.st_mode);
  attr->size = filestatus.st_size;
  attr->last_modified = filestatus.st_mtime;

  return true;
}

bool FileUtil::ListDir(const char* dir, bool onlyname,
                    std::vector<std::string>* dirs,
                    std::vector<std::string>* files) {
  DIR* dirhandle = opendir(dir);
  if (dirhandle == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to list dir [%s]. (%d)",
              dir, errno);
    return false;
  }

  FileAttribute attr;
  struct dirent *entry = readdir(dirhandle);
  for (;entry != NULL; entry = readdir(dirhandle)) {
    // Skip "." and "..".
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Skip un-accessible file.
    std::string fullpath(dir);
    fullpath.append("/").append(entry->d_name);
    if (!GetFileAttribute(fullpath.c_str(), &attr)) {
      continue;
    }

    // Store file names.
    std::vector<std::string>* target = attr.is_dir ? dirs : files;
    if (target != NULL) {
      if (onlyname) {
        target->push_back(entry->d_name);
      } else {
        target->push_back(fullpath);
      }
    }
  }

  closedir(dirhandle);
  return true;
}

#endif
