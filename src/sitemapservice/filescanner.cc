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


#include "sitemapservice/filescanner.h"

#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#endif

#include "common/logger.h"
#include "common/util.h"
#include "common/fileutil.h"
#include "sitemapservice/runtimeinfomanager.h"

FileScanner::FileScanner() {
}

bool FileScanner::Initialize(SiteDataManager *processor, const SiteSetting& setting) {
  if (!UrlProviderService::Initialize(processor, setting)) {
    return false;
  }

  return Initialize(processor, setting.physical_path().c_str(), "/");
}

bool FileScanner::Initialize(SiteDataManager *processor,
                             const char *basedir, const char *baseurl) {
 if (processor == NULL) return false;
 if (basedir == NULL || baseurl == NULL) return false;

 // Save the runtime_info_ for log parser.
 UrlProviderInfo* rt_info = NULL;
 SiteInfo* siteinfo =
   RuntimeInfoManager::application_info()->site_info(sitesetting_.site_id().c_str());
 if (siteinfo != NULL) rt_info = siteinfo->filescanner_info();

 int cycle = sitesetting_.file_scanner_setting().update_duration();
 if (!UrlProviderService::Initialize("_file_scanner", rt_info, cycle)) {
   return false;
 }

 basedir_ = basedir;
 baseurl_ = baseurl;

 // Normalize url to remove its ending "/".
 while (baseurl_.length() > 0 && baseurl[baseurl_.length() - 1] == '/') {
   baseurl_.erase(baseurl_.length() - 1);
 }

 // if no stamp is load only include the file in last year
 if (UrlProviderService::RefreshTimeStamp() == false) {
   time_t one_year_ago = time(NULL) - 3600 * 24 * 365;
   set_last_access_limit(one_year_ago);
   SaveLastAccessLimit();
 }

 return true;
}

bool FileScanner::InternalRun() {

  Logger::Log(EVENT_CRITICAL, "Start to scan directory (%s) for site (%s).",
            basedir_.c_str(), sitesetting_.site_id().c_str());
  ScanDir(basedir_.c_str(), baseurl_.c_str());
  Logger::Log(EVENT_CRITICAL, "End to scan directory (%s).", basedir_.c_str());

  return true;
}


#ifdef WIN32
void FileScanner::ScanDir(const char *dir, const char *url) {
  std::string pattern(dir);
  pattern.append("/").append("*");

  WIN32_FIND_DATAA finddata;

  HANDLE handle = FindFirstFileA(pattern.c_str(), &finddata);
  if (handle == INVALID_HANDLE_VALUE) {
    Logger::Log(EVENT_ERROR, "Failed to scan dir %s. (%d).",
              dir, GetLastError());
    return;
  }

  do {
    if (strcmp(finddata.cFileName, ".") == 0 ||
        strcmp(finddata.cFileName, "..") == 0) {
      continue;
    }

    std::string childfile(dir);
    childfile.append("/").append(finddata.cFileName);
    std::string childurl(url);
    childurl.append("/");
    std::string utf8_filename;
    if (!Util::SystemToUTF8(finddata.cFileName, &utf8_filename)) {
      Logger::Log(EVENT_ERROR, "Failed to convert [%s] to utf8.",
                finddata.cFileName);
      continue;
    }
    Url::EscapeUrlComponent(utf8_filename.c_str(), &childurl);

    if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      ScanDir(childfile.c_str(), childurl.c_str());
    } else {
      FileAttribute file_attr;
      if (FileUtil::GetFileAttribute(childfile.c_str(), &file_attr)) {
        if (file_attr.last_modified > last_access_limit()) {
          ProcessRecord(childurl.c_str(),
                        file_attr.last_modified, file_attr.size);
        }
      }
    }
  } while (FindNextFileA(handle, &finddata));

  FindClose(handle);
}

#elif defined(__linux__) || defined(__unix__)
void FileScanner::ScanDir(const char *dir, const char *url) {
  DIR* dirhandle = opendir(dir);
  if (dirhandle == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to scan dir [%s]. (%d)",
              dir, errno);
    return;
  }

  struct dirent *entry = readdir(dirhandle);
  for (;entry != NULL; entry = readdir(dirhandle)) {
    // skip "." and ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // build child path and url
    std::string childfile(dir);
    childfile.append("/").append(entry->d_name);
    std::string childurl(url);
    childurl.append("/");
    Url::EscapeUrlComponent(entry->d_name, &childurl);

    // skip un-accessible file
    FileAttribute attr;
    if (!FileUtil::GetFileAttribute(childfile.c_str(), &attr)) {
      continue;
    }

    if (attr.is_dir) {
      ScanDir(childfile.c_str(), childurl.c_str());
    } else {
      if (attr.last_modified > last_access_limit()) {
        ProcessRecord(childurl.c_str(), attr.last_modified, attr.size);
      }
    }
  }
  closedir(dirhandle);
}
#endif

void FileScanner::ProcessRecord(const char* url,
                                time_t lastwrite, int64 content) {
  UrlRecord record;
  record.last_filewrite = lastwrite;
  record.contentHashCode = content;
  record.host[0] = '\0';
  time(&record.last_access);
  record.last_modified = lastwrite;
  record.statuscode = 200;

  strncpy(record.url, url, kMaxUrlLength);
  record.url[kMaxUrlLength - 1] = '\0';

  ProvideRecord(record);
}
