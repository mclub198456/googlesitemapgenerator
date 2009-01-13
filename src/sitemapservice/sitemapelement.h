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


// This file defines all elements in sitemap and sitemap index xml file.
// See http://www.sitemaps.org/protocol.php

#ifndef SITEMAPSERVICE_SITEMAPELEMENT_H__
#define SITEMAPSERVICE_SITEMAPELEMENT_H__

#include <ctime>
#include <string>
#include <vector>
#include <map>

// Represents <url> element in sitemap xml file
// It includes four child elements:
//    <loc>         required - location of url
//    <lastmod>     optional - last modified time
//    <changefreq>  optional - change frequency
//    <priority>    optional - priority
class UrlElement {
 public:
  enum ChangeFreq {
    ALWAYS,
    HOURLY,
    DAILY,
    WEEKLY,
    MONTHLY,
    YEARLY,
    NEVER
  };
  
  // Converts frequency enum to string.
  static std::string ConvertFrequencyToString(const UrlElement::ChangeFreq& changefreq);

  
 public:
  std::string loc() const { return loc_; }
  void set_loc(const std::string& loc) { loc_ = loc; }

  time_t lastmod() const { return lastmod_; }
  void set_lastmod(time_t lastmod) { lastmod_ = lastmod; }

  ChangeFreq changefreq() const { return changefreq_; }
  void set_changefreq(ChangeFreq changefreq) { changefreq_ = changefreq; }

  double priority() const { return priority_; }
  void set_priority(double priority) { priority_ = priority; }

  void SetAttribute(const std::string& key, const std::string& value) {
    attributes_[key] = value;
  }
  const std::string& GetAttribute(const std::string& key) const {
    return attributes_.find(key)->second;
  }

  // Low efficiency. For debugging purpose.
  std::string ToString() const;

 private:
  std::string loc_;
  time_t lastmod_;
  ChangeFreq changefreq_;
  double priority_;

  std::map<std::string, std::string> attributes_;
};

// Represents <urlset> element in sitemap xml file.
// It includes one or more <url> elements.
class UrlSetElement {
 public:
  UrlSetElement() {}

  const std::vector<UrlElement>& urls() const {return urls_;}

  void AddUrl(const UrlElement& url);

  void Clear() { urls_.clear(); }

  int Size() const { return (int) urls_.size(); }

  const UrlElement& GetUrl(int index) const { return urls_[index]; }

  // Remove the range [begin, end)
  void RemoveRange(int begin, int end) {
    urls_.erase(urls_.begin() + begin, urls_.begin() + end);
  }

  // Low efficiency. For debugging purpose.
  std::string ToString() const;

 private:
  std::vector<UrlElement> urls_;
};

// Represents <sitemap> element in sitemap index xml file.
// It includes two child elements:
//    <loc>       required - location of the sitemap file
//    <lastmod>   optional - last modified time
class SitemapElement {
 public:
  SitemapElement() {}
  SitemapElement(const std::string& loc, time_t lastmod)
    : loc_(loc), lastmod_(lastmod) {}

  std::string loc() const { return loc_; }
  void set_loc(const std::string& loc) { loc_ = loc; }

  time_t lastmod() const { return lastmod_; }
  void set_lastmod(time_t lastmod) { lastmod_ = lastmod; }

  // Low efficiency. For debugging purpose.
  std::string ToString() const;

 private:
  std::string loc_;
  time_t lastmod_;
};

// Represents <sitemapindex> element in sitemap index xml file.
// It is the root elements of sitemap index file.
// It can include one or more <sitemap> element.
class SitemapIndexElement {
 public:
  const std::vector<SitemapElement>& sitemaps() const {return sitemaps_;}

  void AddSitemap(const SitemapElement& sitemap);

  int Size() const { return (int) sitemaps_.size(); }
  const SitemapElement& GetSitemap(int index) const { return sitemaps_[index]; }
  void Clear() { sitemaps_.clear(); }

  // Low efficiency. For debugging purpose.
  std::string ToString() const;

 private:
  std::vector<SitemapElement> sitemaps_;
};

#endif // SITEMAPSERVICE_SITEMAPELEMENT_H__

