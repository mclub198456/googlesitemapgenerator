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


#include "sitemapservice/runtimeinfomanager.h"

#include <vector>

// Define static memembers.
ApplicationInfo RuntimeInfoManager::application_info_;
CriticalSection critical_section_;

void RuntimeInfoManager::Reset() {
  application_info_.Reset();
  application_info_.ClearSites();
}

bool RuntimeInfoManager::Initialize(SiteSettings settings) {
  Reset();

  // Register all sites according to site settings.
  std::vector<SiteSetting>::const_iterator itr =
    settings.site_settings().begin();
  for (; itr != settings.site_settings().end(); ++itr) {
    // ignore un-enabled site
    if (itr->enabled() == false) continue;

    application_info_.AddSiteInfo(itr->site_id().c_str());
  }
  return true;
}

bool RuntimeInfoManager::GetRuntimeInfoString(std::string* xmlstr) {
  TiXmlDocument* xmldoc = Save();
  if (xmldoc == NULL) return false;

  TIXML_OSTREAM out;
  xmldoc->StreamOut(&out);
  *xmlstr = out.c_str();

  delete xmldoc;
  return true;
}

TiXmlDocument* RuntimeInfoManager::Save() {
  // Build the skeleton of xml document.
  TiXmlDocument* xmldoc = new TiXmlDocument();
  xmldoc->LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
  TiXmlElement* root = new TiXmlElement("ApplicationInfo");
  xmldoc->LinkEndChild(root);

  // Refresh the values.
  application_info_.AutomaticUpdate();

  // Save application_info to xml document.
  bool result = application_info_.Save(root);

  if (!result) {
    delete xmldoc;
    return NULL;
  } else {
    return xmldoc;
  }
}

bool RuntimeInfoManager::Lock(bool block) {
  return critical_section_.Enter(block);
}

void RuntimeInfoManager::Unlock() {
  critical_section_.Leave();
}
