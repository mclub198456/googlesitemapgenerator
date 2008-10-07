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


#include "common/codesearchsitemapsetting.h"

#include <time.h>
#include "third_party/tinyxml/tinyxml.h"

CodeSearchSitemapSetting::CodeSearchSitemapSetting()
  : SitemapSetting("CodeSearchSitemapSetting") {
  ResetToDefault();
}

CodeSearchSitemapSetting::~CodeSearchSitemapSetting() {
}

void CodeSearchSitemapSetting::ResetToDefault() {
  SitemapSetting::ResetToDefault();

  file_name_ = SitemapSetting::GenerateSitemapName("codesearch_sitemap");
  suffix_to_filetype_.clear();

  // Common file suffix to file type mapping.
  suffix_to_filetype_[".java"] = "java";
  suffix_to_filetype_[".cs"] = "c#";
  suffix_to_filetype_[".h"] = "c++";
  suffix_to_filetype_[".cc"] = "c++";
  suffix_to_filetype_[".cpp"] = "c++";
  suffix_to_filetype_[".cxx"] = "c++";
  suffix_to_filetype_[".c"] = "c";
  suffix_to_filetype_[".vb"] = "basic";

  // Archive file type mapping.
  suffix_to_filetype_[".tar"] = "archive";
  suffix_to_filetype_[".tar.z"] = "archive";
  suffix_to_filetype_[".tar.gz"] = "archive";
  suffix_to_filetype_[".tgz"] = "archive";
  suffix_to_filetype_[".tar.bz2"] = "archive";
  suffix_to_filetype_[".tbz"] = "archive";
  suffix_to_filetype_[".tbz2"] = "archive";
  suffix_to_filetype_[".zip"] = "archive";

  // Set default value for included/excluded urls.
  included_urls_.ResetToDefault();
  included_urls_.AddItem(UrlSetting(Url("/*.java")));
  included_urls_.AddItem(UrlSetting(Url("/*.cc")));
  included_urls_.AddItem(UrlSetting(Url("/*.h")));
  included_urls_.AddItem(UrlSetting(Url("/*.cpp")));
  included_urls_.AddItem(UrlSetting(Url("/*.cxx")));
  included_urls_.AddItem(UrlSetting(Url("/*.c")));
  included_urls_.AddItem(UrlSetting(Url("/*.vb")));

  excluded_urls_.ResetToDefault();
}

bool CodeSearchSitemapSetting::Equals(const BaseSetting* another) const {
  // suffix_to_filetype_ comparison is ignored.
  return SitemapSetting::Equals(another);
}

