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


// Application contains the runtime information for the whole application.
// It contains application level informations, like used memory size, used disk
// size and application start time. It also contains runtime information for
// every site.
// This class is not thread-safe.

#ifndef SITEMAPSERVICE_APPLICATIONINFO_H__
#define SITEMAPSERVICE_APPLICATIONINFO_H__

#include <time.h>
#include <string>
#include <map>

#include "common/basictypes.h"
#include "sitemapservice/baseruntimeinfo.h"
#include "sitemapservice/siteinfo.h"

class ApplicationInfo : public BaseRuntimeInfo {
 public:
  // Constructor. Reset() method is invoked.
  ApplicationInfo();

  // Empty destructor.
  virtual ~ApplicationInfo() {}

  // "memory_used" represents the total memory used by application.
  // Default value is "0".
  int64 memory_used() const { return memory_used_; }
  void set_memory_used(int64 memory_used) { memory_used_ = memory_used; }

  // "disk_used" represents the total disk space used by application.
  // Default value is "0"
  int64 disk_used() const { return disk_used_; }
  void set_disk_used(int64 disk_used) { disk_used_ = disk_used; }

  // "start_time" represents the application starting time.
  time_t start_time() const { return start_time_; }
  void set_start_time(time_t start_time) { start_time_ = start_time; }

  // "last_reloading" represents the last time when setting is reloaded.
  time_t last_reloading() const { return last_reloading_; }
  void set_last_reloading(time_t last_reloading) {
    last_reloading_ = last_reloading;
  }

  // "is_reloading" represents whether the setting is reloaded now.
  bool is_reloading() const { return is_reloading_; }
  void set_is_reloading(bool is_reloading) { is_reloading_ = is_reloading; }

  // Get SiteInfo instance for given "site_id".
  // If no such site, NULL is returned.
  // Note, the returned pointer is available during the whole life time of this
  // ApplicationInfo object. The caller shouldn't free the returned pointer.
  SiteInfo* site_info(const std::string& site_id);

  // Add a site.
  // The SiteInfo instance for given "site_id" is returned.
  SiteInfo* AddSiteInfo(const std::string& site_id);

  // Remove site info.
  void RemoveSiteInfo(const std::string& site_id);

  // Remove all site info contained in it.
  void ClearSites();

  // Reset all the fields to default value.
  // NOTE, the contained SiteInfo instances are not cleared, but are reset
  // instead.
  virtual void Reset();

  // Save the inner runtime information to an XML element.
  virtual bool Save(TiXmlElement* element);

  // Update memory_used_ and disk_used automatically.
  // Current implementation just summarize disk space used by all sites, and
  // use the result as application leve "disk_used_" value. This is also the
  // case for "memory_used_" value.
  // As a result, the application level "disk_used_" and "memory_used_" is
  // less than the real value.
  void AutomaticUpdate();

 private:
  int64 memory_used_;
  int64 disk_used_;

  time_t start_time_;
  time_t last_reloading_;

  bool is_reloading_;

  // It maps site-id to its corresponding SiteInfo instance.
  std::map<std::string, SiteInfo> site_infos_;
};

#endif  // SITEMAPSERVICE_APPLICATIONINFO_H__
