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

#include "common/util.h"
#include "common/port.h"
#include "common/timesupport.h"

BaseFilter::BaseFilter() {
  pipe_ = new UrlPipe();
}

BaseFilter::BaseFilter(UrlPipe* pipe) {
  pipe_ = pipe;
}

BaseFilter::~BaseFilter() {
  delete pipe_;
}

bool BaseFilter::Initialize(const char* settingfile) {
  // Load site settings from file and webserver.
  SiteSettings settings;
  if (!settings.LoadWebserverConfig()) {
    Util::Log(EVENT_ERROR, "Failed to load webserver config.");
    return false;
  }
  if (!settings.LoadFromFile(settingfile)) {
    Util::Log(EVENT_ERROR, "Failed to load setting from file.");
    return false;
  }

  // Call the overloaded method.
  return BaseFilter::Initialize(settings);
}

bool BaseFilter::Initialize(const SiteSettings& settings) {
  settings_ = settings;

  if (!BuildSiteIdMap()) {
    Util::Log(EVENT_ERROR, "Failed to build site IDs mapping.");
    return false;
  }

  if (!pipe_->Initialize(false)) {
    Util::Log(EVENT_ERROR, "Failed to initialize url pipe.");
    return false;
  }

  return true;
}

bool BaseFilter::BuildSiteIdMap() {
  site_ids_.clear();
  const std::vector<SiteSetting>& sites = settings_.site_settings();

  for (int i = 0, isize = static_cast<int>(sites.size());
    i < isize; ++i) {
    // Ignore un-enabled site.
    if (sites[i].enabled() == false) continue;

    // Ignore site, which doesn't get url from webserver plugin.
    if (sites[i].webserver_filter_setting().enabled() == false) continue;

    if (sites[i].site_id().length() == 0) {
      Util::Log(EVENT_ERROR, "Site id can't be empty.");
      return false;
    }

    if (site_ids_.find(sites[i].site_id()) != site_ids_.end()) {
      Util::Log(EVENT_ERROR, "Duplicated site id: %s.", sites[i].site_id().c_str());
      return false;
    }

    site_ids_[sites[i].site_id()] = i;
  }

  Util::Log(EVENT_IMPORTANT, "[%d] sites are enabled in filter.",
            site_ids_.size());
  return true;
}

int BaseFilter::MatchSite(const char* siteid) {
  std::map<std::string, int>::iterator itr = site_ids_.find(siteid);
  if (itr == site_ids_.end()) {
    return -1;
  } else {
    return itr->second;
  }
}

bool BaseFilter::Send(UrlRecord *record, int siteindex) {
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

