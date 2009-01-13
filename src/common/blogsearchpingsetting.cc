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
  update_duration_ = 24 * 60 * 60;  // half hour.
  
  blog_name_ = "";
  blog_url_ = "";
  blog_changes_url_ = "";

  included_urls_.ResetToDefault();
  included_urls_.AddItem(UrlSetting(Url("/*.htm")));
  included_urls_.AddItem(UrlSetting(Url("/*.html")));
  excluded_urls_.ResetToDefault();
}

bool BlogSearchPingSetting::LoadSetting(TiXmlElement* element) {
  xml_node_ = element;
  LoadAttribute("enabled", enabled_);
  LoadAttribute("update_duration_in_seconds", update_duration_);
  if (update_duration_ < 60)
    update_duration_ = 60;

  LoadAttribute("blog_name", blog_name_);
  LoadAttribute("blog_url", blog_url_);
  LoadAttribute("blog_changes_url", blog_changes_url_);

  included_urls_.LoadFromParent(element);
  excluded_urls_.LoadFromParent(element);

  return true;
}

TiXmlElement* BlogSearchPingSetting::SaveSetting() {
  xml_node_ = new TiXmlElement(setting_name_.c_str());
  SaveAttribute("enabled", enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_);

  SaveAttribute("blog_name", blog_name_);
  SaveAttribute("blog_url", blog_url_);
  SaveAttribute("blog_changes_url", blog_changes_url_);

  SaveChild(&included_urls_);
  SaveChild(&excluded_urls_);

  return xml_node_;
}

TiXmlElement* BlogSearchPingSetting::SaveSetting(const BaseSetting* global) {
  xml_node_ = NULL;
  const BlogSearchPingSetting* another = (const BlogSearchPingSetting*) global;

  SaveAttribute("enabled", enabled_, another->enabled_);
  SaveAttribute("update_duration_in_seconds", update_duration_,
    another->update_duration_);

  SaveAttribute("blog_name", blog_name_, another->blog_name_);
  SaveAttribute("blog_url", blog_url_, another->blog_url_);
  SaveAttribute("blog_changes_url", blog_changes_url_,
    another->blog_changes_url_);

  SaveChild(&included_urls_, &(another->included_urls_));
  SaveChild(&excluded_urls_, &(another->excluded_urls_));

  return xml_node_;
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

  return included_urls_.Validate() && excluded_urls_.Validate();
}

bool BlogSearchPingSetting::Equals(const BaseSetting* a) const {
  const BlogSearchPingSetting* another = (const BlogSearchPingSetting*) a;

  if (enabled_ != another->enabled_ ||
    update_duration_ != another->update_duration_ ||
    blog_name_ != another->blog_name_ ||
    blog_url_ != another->blog_url_ ||
    blog_changes_url_ != another->blog_changes_url_) {
    return false;
  }

  return included_urls_.Equals(&(another->included_urls_)) &&
    excluded_urls_.Equals(&(another->excluded_urls_));
}

