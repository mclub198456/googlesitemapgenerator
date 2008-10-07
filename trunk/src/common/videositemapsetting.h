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


// Setting for Video Sitemap.
// See: http://www.google.com/support/webmasters/bin/topic.py?topic=10079
// Besides common settings inherited from SitemapSetting class, this class
// doesn't define any new setting field. But it has different default settings
// for some fields.

#ifndef COMMON_VIDEOSITEMAPSETTING_H__
#define COMMON_VIDEOSITEMAPSETTING_H__

#include "common/sitemapsetting.h"
#include "common/url.h"

class VideoSitemapSetting: public SitemapSetting {
 public:
  VideoSitemapSetting();
  virtual ~VideoSitemapSetting();

  // Overriden methods. Please see base class for explanation.
  virtual void ResetToDefault();

  bool Equals(const BaseSetting* another) const;
};

#endif  // COMMON_VIDEOSITEMAPSETTING_H__
