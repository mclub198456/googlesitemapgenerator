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


#include "common/basefilter.h"

#include "common/port.h"
#include "common/timesupport.h"
#include "common/logger.h"

BaseFilter::BaseFilter() {
  pipe_ = new UrlPipe();
}

BaseFilter::BaseFilter(UrlPipe* pipe) {
  pipe_ = pipe;
}

BaseFilter::~BaseFilter() {
  delete pipe_;
}

bool BaseFilter::Initialize(const SiteSettings& settings) {
  settings_ = settings;

  if (!BuildSiteIdMap()) {
    Logger::Log(EVENT_ERROR, "Failed to build site IDs mapping.");
    return false;
  }

  if (!pipe_->Initialize(false)) {
    Logger::Log(EVENT_ERROR, "Failed to initialize url pipe.");
    return false;
  }

  return true;
}

bool BaseFilter::BuildSiteIdMap() {
  site_ids_.clear();
  const std::vector<SiteSetting>& sites = settings_.site_settings();

  default_enabled_ = settings_.auto_add() &&
    settings_.global_setting().enabled()&&
    settings_.global_setting().webserver_filter_setting().enabled();

  for (int i = 0, isize = static_cast<int>(sites.size());
    i < isize; ++i) {
    if (sites[i].site_id().length() == 0) {
      Logger::Log(EVENT_ERROR, "Site id can't be empty.");
      return false;
    }

    // Ignore un-enabled site.
    bool enabled = sites[i].enabled() &&
      sites[i].webserver_filter_setting().enabled();
    if (enabled != default_enabled_) {
      site_ids_.insert(sites[i].site_id());
    }
  }

  Logger::Log(EVENT_IMPORTANT, "[%d] sites are [%s] in filter.",
    site_ids_.size(), (default_enabled_ ? "disabled" : "enabled"));
  return true;
}

// Because settings may be changed in service side,
// currently records from all sites are sent to service side.
bool BaseFilter::MatchSite(const char* siteid) {
  return true;
  /*
  std::set<std::string>::iterator itr = site_ids_.find(siteid);
  if (default_enabled_) {
    return itr == site_ids_.end();
  } else {
    return itr != site_ids_.end();
  }
  */
}

bool BaseFilter::Send(UrlRecord *record) {
  // Host should always be in lower case.
  strlwr(record->host);

  return pipe_->Send(record, 1) == 1;
}

bool BaseFilter::TreatAsStatic(const char* file) {
  // The non-static files which we want to treat them as static.
  static const char* kSupportedTypes[] = {
    "shtml", NULL
  };

  // Find the '.' for extension.
  const char* p = strlen(file) + file;
  while (p != file && *p != '.') --p;
  if (p == file) return false;
  ++p; // skip '.'

  // Find the matched type.
  const char** type = kSupportedTypes;
  for (; *type != NULL; ++type) {
    if (stricmp(*type, p) == 0) {
      break;
    }
  }

  return *type != NULL;
}

bool BaseFilter::CopySiteId(int siteindex, char* dest, int maxlen) {
  // Check whether the "siteindex" is valid.
  const std::vector<SiteSetting>& sites = settings_.site_settings();
  if (siteindex < 0 || siteindex > static_cast<int>(sites.size())) {
    return false;
  }

  // Copy the site-id to "dest".
  dest[maxlen - 1] = '\0';
  strncpy(dest, sites[siteindex].site_id().c_str(), maxlen);
  return dest[maxlen - 1] == '\0';
}

bool BaseFilter::CheckStatusCode(int status) {
  return status == 200 || status == 301 || status == 302
	 || status == 307 || status == 404;
}

bool BaseFilter::ParseTime(const char *str, time_t* time) {
  struct tm tm;
  bool result = ParseRfcTime(str, &tm);
  if (result && time != NULL) {
    *time = _mkgmtime(&tm);
    return *time != -1;
  }
  return false;
}

