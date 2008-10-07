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


#include "common/webserverconfig.h"

#include "common/util.h"

void WebserverConfig::RemoveAdminConsoleSite() {
  for (int i = 0; i < static_cast<int>(physical_paths_.size()); ++i) {
    if (physical_paths_[i].find(Util::GetApplicationDir()) == 0) {
      physical_paths_.erase(physical_paths_.begin() + i);
      site_ids_.erase(site_ids_.begin() + i);
      names_.erase(names_.begin() + i);
      host_urls_.erase(host_urls_.begin() + i);
      log_paths_.erase(log_paths_.begin() + i);
      break;
    }
  }
}

