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


#include "sitemapservice/logparser.h"

#include <errno.h>

#include "common/fileutil.h"
#include "common/logger.h"
#include "sitemapservice/runtimeinfomanager.h"

LogParser::LogParser() {
  lineparsers_.push_back(new CLFParser());
  lineparsers_.push_back(new ELFParser());
}

LogParser::~LogParser() {
  bestparser_ = NULL;
  for (int i = 0; i < static_cast<int>(lineparsers_.size()); ++i) {
    delete lineparsers_[i];
  }
  lineparsers_.clear();
}

bool LogParser::Initialize(SiteDataManager* processor, const SiteSetting& setting) {
  if (!UrlProviderService::Initialize(processor, setting)) {
    return false;
  }

  // Save the runtime_info_ for log parser.
  UrlProviderInfo* rt_info = NULL;
  SiteInfo* siteinfo =
    RuntimeInfoManager::application_info()->site_info(sitesetting_.site_id().c_str());
  if (siteinfo != NULL) rt_info = siteinfo->logparser_info();

  int cycle = setting.log_parser_setting().update_duration();
  if (!UrlProviderService::Initialize("_log_parser", rt_info, cycle)) {
    return false;
  }

  path_ = setting.log_path();
  bestparser_ = NULL;

  if (path_.length() == 0) {
    Logger::Log(EVENT_ERROR, "Log path for [%s] site is null.",
              setting.site_id().c_str());
    return false;
  }

 // If no stamp is load only include the file in last year.
 if (UrlProviderService::RefreshTimeStamp() == false) {
   time_t one_year_ago = time(NULL) - 3600 * 24 * 365;
   set_last_access_limit(one_year_ago);
   SaveLastAccessLimit();
 }

  return true;
}

bool LogParser::InternalRun() {
  Logger::Log(EVENT_CRITICAL, "Start to parse log (%s) for site (%s).",
            path_.c_str(), sitesetting_.site_id().c_str());
  FileAttribute fileattr;
  if (!FileUtil::GetFileAttribute(path_.c_str(), &fileattr)) {
    Logger::Log(EVENT_ERROR, "Can't access [%s], and stop log.", path_.c_str());
    return false;
  }

  std::vector<std::string> files;
  if (fileattr.is_dir) {
    if (!FileUtil::ListDir(path_.c_str(), false, NULL, &files)) {
      Logger::Log(EVENT_ERROR, "Can't list dir [%s]. and stop scan log.",
                path_.c_str());
      return false;
    }
  } else {
    files.push_back(path_);
  }

  // Before running, the timestamp of bestparser_ should be updated.
  if (bestparser_ != NULL) {
    bestparser_->set_oldest(last_access_limit());
  }

  // Iterate all log files.
  for (int i = 0; i < static_cast<int>(files.size()); ++i) {
    if (!FileUtil::GetFileAttribute(files[i].c_str(), &fileattr)) {
      Logger::Log(EVENT_ERROR, "Ignore log file: [%s].", files[i].c_str());
      continue;
    }

    // Very old log file.
    if (fileattr.last_modified <= last_access_limit()) {
      Logger::Log(EVENT_NORMAL, "Ignore old log file: [%s].", files[i].c_str());
      continue;
    }

    ParseLogFile(files[i].c_str());
  }

  Logger::Log(EVENT_CRITICAL, "End to parse log (%s).", path_.c_str());
  return true;
}

bool LogParser::ParseLogFile(const char* path) {
  Logger::Log(EVENT_IMPORTANT, "Process log file [%s]...", path);

  FILE* file = fopen(path, "r");
  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open log file [%s]. (%d)",
              path, errno);
    return false;
  }

  UrlRecord record;
  char buf[1024];
  while (fgets(buf, 1024, file) != NULL) {
    // Read a whole line.
    std::string strbuf;
    strbuf.append(buf);
    while (strbuf[strbuf.length() - 1] != '\n' &&
           fgets(buf, 1024, file) != NULL) {
      strbuf.append(buf);
    }

    // Remove line breaks.
    if (strbuf.length() > 0 && strbuf[strbuf.length() - 1] == '\n') {
      strbuf.erase(strbuf.length() - 1);
    }    
    if (strbuf.length() > 0 && strbuf[strbuf.length() - 1] == '\r') {
      strbuf.erase(strbuf.length() - 1);
    }

    // Skip empty line.
    if (strbuf.length() == 0) continue;

    // Try to select the best parser.
    if (bestparser_ == NULL) {
      for (int i = 0; i < static_cast<int>(lineparsers_.size()); ++i) {
        if (lineparsers_[i]->Parse(strbuf.c_str(), &record) !=
            LineParser::PARSE_FAIL) {
          bestparser_ = lineparsers_[i];
          bestparser_->set_oldest(last_access_limit());
          Logger::Log(EVENT_CRITICAL, "[%s] is selected for parser [%s].",
            bestparser_->name().c_str(), sitesetting_.site_id().c_str());
          break;
        }
      }
    }

    // Do actual parsing process.
    if (bestparser_ != NULL) {
      if (bestparser_->Parse(strbuf.c_str(), &record) == LineParser::PARSE_OK) {
        ProvideRecord(record);
      }
    }
  }

  fclose(file);
  return true;
}
