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


#include "common/blogsearchpingsetting.h"

#include <stdlib.h>
#include <vector>

#include "third_party/tinyxml/tinyxml.h"
#include "common/basesetting.h"
#include "common/util.h"

const char* BlogSearchPingSetting::kPingServiceUrl =
  "http://blogsearch.google.com/ping";

BlogSearchPingSetting::BlogSearchPingSetting()
  : BaseSetting("BlogSearchPingSetting") {
}

BlogSearchPingSetting::~BlogSearchPingSetting() {
}

void BlogSearchPingSetting::ResetToDefault() {
  enabled_ = false;
  update_duration_ = 30 * 60;  // half hour.
  
  blog_name_ = "";
  blog_url_ = "";
  blog_changes_url_ = "";

  included_urls_.clear();
  excluded_urls_.clear();
}

bool BlogSearchPingSetting::LoadSetting() {
  LoadAttribute("enabled", enabled_);
  LoadAttribute("update_duration_in_seconds", update_duration_);
  if (update_duration_ < 60)
    update_duration_ = 60;

  LoadAttribute("blog_name", blog_name_);
  LoadAttribute("blog_url", blog_url_);
  LoadAttribute("blog_changes_url", blog_changes_url_);

  LoadUrls("IncludedUrls", included_urls_);
  LoadUrls("ExcludedUrls", excluded_urls_);

  return true;
}

void BlogSearchPingSetting::SaveSetting() {
  SaveAttribute("enabled", enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_);

  SaveAttribute("blog_name", blog_name_);
  SaveAttribute("blog_url", blog_url_);
  SaveAttribute("blog_changes_url", blog_changes_url_);

  SaveUrls("IncludedUrls", included_urls_);
  SaveUrls("ExcludedUrls", excluded_urls_);
}

bool BlogSearchPingSetting::Validate() const {
  // Skips validation if enabled_ is false.
  if (!enabled_)
    return true;

  if (update_duration_ <= 0)
    return false;

  // The blog_url_ and blog_changes_url_ is not required.
  // But if it did exist, the URL should be valid.
  if (blog_url_.length() != 0 && !Url::Validate(blog_url_.c_str())) {
    return false;
  }
  if (blog_changes_url_.length() != 0 &&
    !Url::Validate(blog_changes_url_.c_str())) {
    return false;
  }

  // Validate include_urls
  for (std::vector<Url>::const_iterator it = included_urls_.begin();
       it != included_urls_.end(); ++it) {
    if (!it->Validate())
      return false;
  }

  // Validate excluded_urls_
  for (std::vector<Url>::const_iterator it = excluded_urls_.begin();
       it != excluded_urls_.end(); ++it) {
    if (!it->Validate())
      return false;
  }

  return true;
}

