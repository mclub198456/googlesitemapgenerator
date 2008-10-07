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

// This class provides methods to get/set setting for a single site.
// It treats setting for a site as a stand-alone xml document.
// For conveniece, the xml document can be passed as a string or a file.

// This file provides several utility methods to manipulate settings for
// an individual site.

#ifndef SITEMAPSERVICE_SITESETTINGMANAGER_H__
#define SITEMAPSERVICE_SITESETTINGMANAGER_H__

#include <string>

class SiteSettingManager {
public:
  // Get individual site setting as an XML string.
  // The returned setting value is a completed one, including both customized
  // value defined for this site, and default value for all sites.
  static bool GetSiteSetting(const std::string& site_id, std::string* xml);

  // Set individual site setting.
  // The setting xml element can be incompleted. Only values given in xml
  // element will be changed, while all others still have old values.
  static bool SetSiteSetting(const std::string& site_id,
                             const std::string& xml);

  // Get individual site setting and save it to a file.
  // See "GetSiteSetting" above.
  static bool GetSiteSettingToFile(const std::string& site_id,
                                   const std::string& file);

  // Load individual site setting from a file and merge it to application
  // setting file.
  // See "SetSiteSetting" above.
  static bool SetSiteSettingFromFile(const std::string& site_id,
                                     const std::string& file);

private:
  SiteSettingManager() {}
};

#endif // SITEMAPSERVICE_SITESETTINGMANAGER_H__
