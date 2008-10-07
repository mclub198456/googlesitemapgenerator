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


// Setting for Code Search Sitemap.
// See: http://www.google.com/support/webmasters/bin/topic.py?topic=12640
// Besides the common sitemap settings, there is also suffix_to_filetype_ field,
// which could map a file suffix to filetype field specified in protocol, like:
// ".cs"->"c#", ".java.txt"->"java".
// Currently, this field is not configurable, and more flexible settings should
// be added per new requirement. See ResetToDefault() for default values.
// This class is not thread-safe.

#ifndef COMMON_CODESEARCHSITEMAPSETTING_H__
#define COMMON_CODESEARCHSITEMAPSETTING_H__

#include <string>
#include <map>

#include "common/sitemapsetting.h"
#include "common/url.h"

class CodeSearchSitemapSetting: public SitemapSetting {
 public:
  // Maps a file suffix to a file type name.
  typedef std::map<std::string, std::string> FileTypeMap;

  CodeSearchSitemapSetting();
  virtual ~CodeSearchSitemapSetting();

  const FileTypeMap& suffix_to_filetype() const {
    return suffix_to_filetype_;
  }
  void set_suffix_to_filetype(const FileTypeMap& suffix_to_filetype) {
    suffix_to_filetype_ = suffix_to_filetype;
  }

  // Overrode methods.
  virtual void ResetToDefault();

  bool Equals(const BaseSetting* another) const;

 private:
   // File suffix to filetype map, like ".cs" to "c#", ".jsp.txt" to "jsp"
   // Only filetypes in this map are supported by sitemap generator.
   FileTypeMap suffix_to_filetype_;
};

#endif  // COMMON_CODESEARCHSITEMAPSETTING_H__
