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


// This class is used to manage news database.
// A news database consists of two files. One is a file containing all
// historical URL fingerprint. The other is a file containing details URL
// information, which are included in last news sitemap file.
// When a news database should be updated, it reads all temporary data file
// for the site, and updates the fingerprint file and latest URL information.

#ifndef SITEMAPSERVICE_NEWSDATAMANAGER_H__
#define SITEMAPSERVICE_NEWSDATAMANAGER_H__

#include <string>
#include "sitemapservice/sitedatamanager.h"

class NewsDataManager {
public:
  NewsDataManager();
  ~NewsDataManager();

  // Initialize news database manager.
  // "sitedata_manager" is the manager used to manage site database.
  // "dir" is a sub-directory of site data directory.
  bool Initialize(SiteDataManager* sitedata_manager, const std::string& dir);

  // Update news database.
  bool UpdateData();

  // Get file containing latest URL information.
  std::string GetDataFile();

  // Get directory (full path) containing the news database.
  std::string GetDirectory();

private:
  // Constants for file name.
  static const std::string kFprintFile;
  static const std::string kDataFile;

  // Merge Url fingerprint.
  // "newentry" will contain detailed new url information.
  // "new_fp" is the new fingerprint file.
  // "old_fp" is the old fingerprint file.
  // "srcs" are the temporary site data file.
  bool MergeUrlFprint(const std::string& newentry, const std::string& new_fp,
                      const std::string& old_fp,
                      const std::vector<std::string> srcs);

  SiteDataManager* sitedata_manager_;

  // The directory used to hold news database.
  std::string data_dir_;

  // The last update time of news database.
  time_t last_update_;
};

#endif // SITEMAPSERVICE_NEWSDATAMANAGER_H__


