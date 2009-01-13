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


#include <sstream>
#include "sitemapservice/sitemapelement.h"


// I think rename it to ConvertFrequencyToString is better!
std::string UrlElement::ConvertFrequencyToString(const UrlElement::ChangeFreq& changefreq) {
  switch (changefreq) {
    case UrlElement::ALWAYS:   return "always";
    case UrlElement::HOURLY:   return "hourly";
    case UrlElement::DAILY:    return "daily";
    case UrlElement::WEEKLY:   return "weekly";
    case UrlElement::MONTHLY:  return "monthly";
    case UrlElement::YEARLY:   return "yearly";
    case UrlElement::NEVER:    return "never";
    default:            return ""; // log error here!
  }
}


// These ToString methods can't be used for the purpose of generating xml file.
// They have very low efficiency, but more user friendly.
// Thus, they can be used for debugging purpose.
std::string UrlElement::ToString() const {
  std::ostringstream ostrstream;
  ostrstream << "Url: "
    << "changefreq: " << changefreq_
    << "lastmod: " << lastmod_
    << "loc_: " << loc_
    << "priority_" << priority_;
  return ostrstream.str();
}

std::string UrlSetElement::ToString() const {
  std::ostringstream ostrstream;
  ostrstream << "UrlSet: " << urls_.size() << std::endl;
  for (int i = 0, _size = static_cast<int>(urls_.size());
    i < _size; ++i) {
      ostrstream << urls_[i].ToString() << std::endl;
  }

  return ostrstream.str();
}

std::string SitemapElement::ToString() const {
  std::ostringstream ostrstream;
  ostrstream << "Url: "
    << "lastmod: " << lastmod_
    << "loc_: " << loc_;
  return ostrstream.str();
}

std::string SitemapIndexElement::ToString() const {
  std::ostringstream ostrstream;
  ostrstream << "SitemapIndex: " << sitemaps_.size() << std::endl;
  for (int i = 0, _size = static_cast<int>(sitemaps_.size());
    i < _size; ++i) {
      ostrstream << sitemaps_[i].ToString() << std::endl;
  }

  return ostrstream.str();
}

void UrlSetElement::AddUrl(const UrlElement& url) {
  urls_.push_back(url);
}

void SitemapIndexElement::AddSitemap(const SitemapElement& sitemap) {
  sitemaps_.push_back(sitemap); 
}
