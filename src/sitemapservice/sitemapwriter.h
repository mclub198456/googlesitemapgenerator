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


// This file defines SitemapWriter interface used to generate sitemap file
// and sitemap index file from sitemap elements.
// Currently, implementations to generate xml sitemap files for different
// kinds of sitemap are provided.

#ifndef SITEMAPSERVICE_SITEMAPWRITER_H__
#define SITEMAPSERVICE_SITEMAPWRITER_H__

#include <cstdio>
#include <string>
#include <vector>

#include "sitemapservice/sitemapelement.h"

class SitemapWriter {
 public:
  SitemapWriter() {}
  virtual ~SitemapWriter() {}

  // Indicates whether the gnerator information should be added into sitemap.
  virtual void SetAddGeneratorInfo(bool enable) = 0;

  // Write a sitemap file.
  // "file" is the full path name of the sitemap file.
  // "urlset" contains all URLs which should be written into sitemap file.
  // "maxvolum" is the maxvolum size of the sitemap file.
  // Returns the number of URLs written into file. The number must be equal
  // or less than the number in urlset because of the "maxvolum" limitation.
  // -1 should be returned on failure.
  virtual int WriteSitemap(const std::string& file,
                           const UrlSetElement& urlset,
                           int maxvolum) = 0;

  // Write a sitemap index file.
  // "file" is the full path name of the index file.
  // "sitemap_index" contains all sitemaps which should be indexed.
  virtual int WriteSitemapIndex(const std::string& file,
                                const SitemapIndexElement& sitemap_index) = 0;
};

// This implementation of SitemapWriter can generate sitemap file in xml format
// which is defined in http://www.sitemaps.org/protocal.php
class XmlSitemapWriter : public SitemapWriter {
 public:
  XmlSitemapWriter();
  ~XmlSitemapWriter() {}

  // Escape characters which should be escaped in an xml document.
  static std::string EscapeEntity(const std::string& str);

  // Overridden methods.
  // See base class.
  virtual void SetAddGeneratorInfo(bool enable) {
    add_generator_info_ = enable;
  }
  virtual int WriteSitemap(const std::string& file,
                           const UrlSetElement& urlset,
                           int maxvolum);
  virtual int WriteSitemapIndex(const std::string& file,
                        const SitemapIndexElement& sitemap_index);

 protected:
  // Add generator information to buffer.
  void GetGeneratorInfo(std::string& buffer);

  // Write a single Url element to the file.
  virtual bool WriteUrlElement(const UrlElement& url, FILE* file);

  // Add <url> element extension. It is used by sitemap extension.
  virtual void AddUrlElementExtension(const UrlElement& url,
                                      std::string* buffer);
 
  // Write a sitemap element to the file, which should be an index file.
  virtual int WriteSitemapElement(const SitemapElement& sitemap, FILE* file);

  // Get custom xml namespace which can extend the sitemaps protocol.
  // It should be something like:
  // xmlns:example="http://www.example.com/schema/example_schema"
  // This implementation just returns an empty vector.
  virtual std::vector<std::string> GetXmlns();

  // Methods to limit the file volumn.
  inline const int& volumleft() {return volumleft_;}
  inline void decrease_volumleft(int value) {
    volumleft_ -= value;
  }

 private:
  // How many volums are left on current opening FILE.
  int volumleft_;

  // Whether generator information should be added.
  bool add_generator_info_;
};


// This class extends XmlSitemapWriter to support Google news extension.
class XmlNewsSitemapWriter : public XmlSitemapWriter {
 protected:
  virtual bool WriteUrlElement(const UrlElement& url, FILE* file);

  // Besides common namespace, it adds:
  // xmlns:news="http://www.google.com/schemas/sitemap-news/0.9"
  virtual std::vector<std::string> GetXmlns();
};

// This class extends XmlSitemapWriter to support Google video extension.
class XmlVideoSitemapWriter : public XmlSitemapWriter {
 protected:
  virtual bool WriteUrlElement(const UrlElement& url, FILE* file);

  // Besides common namespace, it adds:
  // xmlns:video="http://www.google.com/schemas/sitemap-video/1.0"
  virtual std::vector<std::string> GetXmlns();
};

// This class extends XmlSitemapWriter to support Google code search extension.
class XmlCodeSearchSitemapWriter : public XmlSitemapWriter {
 protected:
  virtual bool WriteUrlElement(const UrlElement& url, FILE* file);

  // Besides common namespace, it adds:
  // xmlns:codesearch="http://www.google.com/codesearch/schemas/sitemap/1.0"
  virtual std::vector<std::string> GetXmlns();
};

// This class extends XmlSitemapWriter to support Google mobile extension.
class XmlMobileSitemapWriter : public XmlSitemapWriter {
 protected:
  virtual void AddUrlElementExtension(const UrlElement& url,
                                      std::string* buffer);

  // Besides common namespace, it adds:
  // xmlns:mobile="http://www.google.com/schemas/sitemap-mobile/1.0"
  virtual std::vector<std::string> GetXmlns();
};


#endif // SITEMAP_SITEMAPSERVICE_SITEMAPWRITER_H__

