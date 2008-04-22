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


// FileScanner is a kind of UrlProviderService. It scanns the file system and
// converts the file path to a URL. Only the files, which are modified or added
// after last running time, will be provided to site data manager.
// This class is thread-safe after it is initialized.

#ifndef SITEMAPSERVICE_FILESCANNER_H__
#define SITEMAPSERVICE_FILESCANNER_H__

#include <string>
#include "common/basictypes.h"
#include "sitemapservice/sitedatamanager.h"
#include "sitemapservice/urlproviderservice.h"

class FileScanner : public UrlProviderService {
 public:
  FileScanner();
  virtual ~FileScanner() {}

  // Overriden.
  // This method will extract log parser specific setting from site setting,
  // and passes to base class.
  // In this method, site's physical path will be used as "basedir", and "/"
  // will be used as "baseurl". See overloaded method below.
  virtual bool Initialize(SiteDataManager* processor,
                          const SiteSetting& setting);

  // Initialize the method.
  bool Initialize(SiteDataManager* processor, const char* basedir,
                  const char* baseurl);

 protected:
  // Do the actual work.
  // It scans the site root directory, and constructs URL from the relative
  // file path.
  virtual bool InternalRun();

 private:
  void ScanDir(const char* dir, const char* url);

  // Only three attributes given as params could be determined by scanner.
  // All the others would be filled with common values.
  void ProcessRecord(const char* url, time_t lastwrite, int64 content);

  std::string basedir_;
  std::string baseurl_;

  DISALLOW_EVIL_CONSTRUCTORS(FileScanner);
};

#endif // SITEMAPSERVICE_FILESCANNER_H__

