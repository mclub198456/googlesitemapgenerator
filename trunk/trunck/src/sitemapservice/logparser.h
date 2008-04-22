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


// LogParser is a kind of UrlProviderService. It parses webserver's log file
// and provide log entry to site data manager.
// Only the entries, which are logged after last running time, will be
// provided to site data manager.
// It can try to guess log format automatically. It also supports a dir of log
// files as well as a single log file.
// This class is thread-safe after it is initialized.

#ifndef SITEMAPSERVICE_LOGPARSER_H__
#define SITEMAPSERVICE_LOGPARSER_H__

#include "common/basictypes.h"
#include "sitemapservice/sitedatamanager.h"
#include "sitemapservice/urlproviderservice.h"
#include "sitemapservice/lineparser.h"

#include <string>
#include <vector>

class LogParser : public UrlProviderService {
 public:
  LogParser();
  virtual ~LogParser();

  // Overriden.
  // This method will extract log parser specific setting from site setting,
  // and passes to base class.
  virtual bool Initialize(SiteDataManager* processor,
                          const SiteSetting& setting);

 protected:
  // Do the actual work.
  // It try to parse the log file/directory, and add the log entry with
  // UrlProviderService::ProvideRecord method.
  virtual bool InternalRun();

 private:
  // Parse a single log file.
  bool ParseLogFile(const char* path);

  // The log file path.
  // It could be a single file, or a directory. If it is a directory, all log
  // files under this directory will be parsed, but not recursively.
  std::string path_;

  // Lineparsers contains all available line parsers.
  // bestparser_ is the most fittable parser selected from lineparsers_;
  std::vector<LineParser*> lineparsers_;
  LineParser* bestparser_;

  DISALLOW_EVIL_CONSTRUCTORS(LogParser);
};

#endif // SITEMAPSERVICE_LOGPARSER_H__

