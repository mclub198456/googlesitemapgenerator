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


// This class filters URL according to robots.txt.
// It follows http://www.robotstxt.org/norobots-rfc.txt, but not so strict.
// 1) "User-agent", "Allow", "Disallow" is case insensitive.
// 2) No special handling on robots.txt itself.
// It is assumed that a valid robots.txt is given. The result in un-expected
// if the robots.txt is malformed.
// This class is thread-safe after it is initialized.

#ifndef SITEMAPSERVICE_ROBOTSTXTFILTER_H__
#define SITEMAPSERVICE_ROBOTSTXTFILTER_H__

#include "sitemapservice/urlfilter.h"

#include <string>
#include <utility>
#include <vector>

class RobotsTxtFilter : public UrlFilter {
public:
  RobotsTxtFilter() {}
  virtual ~RobotsTxtFilter() {}

  // Initialize with given path, like "/var/www/robots.txt"
  // Error is ignored and logged.
  void Initialize(const char* path);

  // Returns true if the url is allowed by robots.txt.
  bool Accept(const char* url) {
    return Accept(url, static_cast<int>(strlen(url)));
  }

  // Returns true if the url is allowed by robots.txt.
  virtual bool Accept(const char* url, int urllen);

private:
  // "Allow" and "Disallow" rules.
  // rules_[i].first: the flag for Allow/Disallow.
  // rules_[i].second: the actual rule string.
  std::vector<std::pair<bool, std::string> > rules_;
};


#endif  // SITEMAPSERVICE_ROBOTSTXTFILTER_H__

