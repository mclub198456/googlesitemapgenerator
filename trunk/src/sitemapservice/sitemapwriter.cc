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


#include "sitemapservice/sitemapwriter.h"

#include <errno.h>
#include <cstdio>

#include "common/version.h"
#include "common/logger.h"
#include "common/util.h"
#include "common/timesupport.h"

XmlSitemapWriter::XmlSitemapWriter() {
  add_generator_info_ = true;
}

int XmlSitemapWriter::WriteSitemap(const std::string& file_name, 
                                   const UrlSetElement& urlset,
                                   int maxvolum) {
  FILE* file = fopen(file_name.c_str(), "w");

  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Writer can't open file: %s (%d)\n",
              file_name.c_str(), errno);
    return -1;
  }

  volumleft_ = maxvolum;

  // "\xEF\xBB\xBF" is UTF-8 file prefix
  std::string head("\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

  // Write the <urlset> starting tag.
  head.append("<urlset ");
  std::vector<std::string> xmlns = GetXmlns();
  for (int i = 0, _size = static_cast<int>(xmlns.size()); i < _size; ++i) {
    head.append((i == 0) ? " " : "\n");
    head.append(xmlns[i]);
  }
  head.append(">\n");

  GetGeneratorInfo(head);

  fputs(head.c_str(), file);
  
  // Whatever the volum limitation is, the xml should be a complete one.
  head.append("</urlset>\n");
  decrease_volumleft(static_cast<int>(head.length()));

  // Write the urls.
  int written_urls = 0;
  for (int _size = urlset.Size(); written_urls < _size; ++written_urls) {
    if (!WriteUrlElement(urlset.GetUrl(written_urls), file)) {
      break;
    }
  }

  // Close the urlset.
  fputs("</urlset>\n", file);
  fclose(file);

  return written_urls;
}

int XmlSitemapWriter::WriteSitemapIndex(
      const std::string& file_name,
      const SitemapIndexElement& sitemap_index) {
  FILE* file = fopen(file_name.c_str(), "w");
  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Writer can't open file: %s (%d)\n",
              file_name.c_str(), errno);
    return 1;
  }

  // "\xEF\xBB\xBF" is UTF-8 file prefix
  std::string head("\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  head.append("<sitemapindex xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\"");
  head.append(">\n");
  fputs(head.c_str(), file);

  // Add generator information.
  if (add_generator_info_) {
    std::string generator_info;
    GetGeneratorInfo(generator_info);
    fputs(generator_info.c_str(), file);
  }

  for (int i = 0, _size = sitemap_index.Size(); i < _size; ++i) {
    WriteSitemapElement(sitemap_index.GetSitemap(i), file);
  }

  fputs("</sitemapindex>", file);
  return fclose(file);
}

bool XmlSitemapWriter::WriteUrlElement(const UrlElement &url, FILE *file) {
  std::string buffer;
  buffer.append("<url>\n");
  buffer.append("  <loc>").append(EscapeEntity(url.loc())).append("</loc>\n");

  buffer.append("  <lastmod>").append(
    ::FormatW3CTime(url.lastmod())).append("</lastmod>\n");

  buffer.append("  <changefreq>").append(
    UrlElement::ConvertFrequencyToString(url.changefreq())).append("</changefreq>\n");


  char prioritystr[128];
  sprintf(prioritystr, "%.1lf", url.priority());
  buffer.append("  <priority>").append(prioritystr).append("</priority>\n");

  AddUrlElementExtension(url, &buffer);
  buffer.append("</url>\n");

  if (static_cast<int>(buffer.length()) > volumleft()) {
    return false;
  }

  fputs(buffer.c_str(), file);
  decrease_volumleft(static_cast<int>(buffer.length()));
  return true;
}

void XmlSitemapWriter::AddUrlElementExtension(const UrlElement& url,
                                                    std::string* buffer) {
  // does nothing.
}


int XmlSitemapWriter::WriteSitemapElement(const SitemapElement &sitemap, 
                                          FILE *file) {
  std::string buffer;
  buffer.append("<sitemap>\n");
  buffer.append("  <loc>").append(EscapeEntity(sitemap.loc())).append("</loc>\n");

  buffer.append("  <lastmod>").append(
    ::FormatW3CTime(sitemap.lastmod())).append("</lastmod>\n");

  buffer.append("</sitemap>\n");

  fputs(buffer.c_str(), file);
  return 0;
}

std::vector<std::string> XmlSitemapWriter::GetXmlns() {
  std::vector<std::string> xmlns;
  xmlns.push_back("xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\"");
  return xmlns;
}

// for url.loc
std::string XmlSitemapWriter::EscapeEntity(const std::string& url) {
  std::string buffer;
  buffer.reserve(url.length() * 6 + 1);

  for (int i = 0, _length = static_cast<int>(url.length());
      i < _length; ++i) {
    int value = static_cast<unsigned char>(url[i]);
    if (value <= 32 || value >= 128) {
      // Escape all non-printable entities.
      // But actually, this rarely happen because url is already filtered.
      buffer.append("&#x");
      buffer.push_back(Util::int_to_hex_digit_low(value >> 4));
      buffer.push_back(Util::int_to_hex_digit_low(value));
    } else {
      switch (url[i]) {
        case '&': // Ampersand
          buffer.append("&amp;");
          break;

        case '\'': // Single Quote
          buffer.append("&apos;");
          break;

        case '\"': // Double Quote
          buffer.append("&quot;");
          break;

        case '>': // Greater Than
          buffer.append("&gt;");
          break;

        case '<': // Less Than
          buffer.append("&lt;");
          break;

        default:
          buffer.push_back(url[i]);
          break;
      }
    }
  }

  return buffer;
}

void XmlSitemapWriter::GetGeneratorInfo(std::string& buffer) {
  if (!add_generator_info_) return;

  buffer.append("<!--\n");
  buffer.append("  <gen:info>\n");

  buffer.append("    <gen:name>Google Sitemap Generator</gen:name>\n");

  buffer.append("    <gen:version>");
  buffer.append(SITEMAP_VERSION1);
  buffer.append("</gen:version>\n");

  buffer.append("    <gen:date>");
  buffer.append(::FormatW3CTime(time(NULL)));
  buffer.append("</gen:date>\n");

  buffer.append("  </gen:info>\n");
  buffer.append("-->\n");
}

/////////////////////////////////////////////////////////////////////
// Implementations for XmlNewsSitemapWriter

bool XmlNewsSitemapWriter::WriteUrlElement(const UrlElement &url, FILE *file) {
  std::string buffer;
  buffer.append("<url>\n");
  buffer.append("  <loc>").append(EscapeEntity(url.loc())).append("</loc>\n");
  buffer.append("  <news:news>\n");

  buffer.append("    <news:publication_date>")
    .append(url.GetAttribute("publication_date"))
    .append("</news:publication_date>\n");

  buffer.append("  </news:news>\n");
  buffer.append("</url>\n");

  if (static_cast<int>(buffer.length()) > volumleft()) {
    return false;
  }

  fputs(buffer.c_str(), file);
  decrease_volumleft(static_cast<int>(buffer.length()));
  return true;
}

std::vector<std::string> XmlNewsSitemapWriter::GetXmlns() {
  std::vector<std::string> xmlns = XmlSitemapWriter::GetXmlns();
  xmlns.push_back("xmlns:news=\"http://www.google.com/schemas/sitemap-news/0.9\"");
  return xmlns;
}

/////////////////////////////////////////////////////////////////////
// Implementations for XmlVideoSitemapWriter

bool XmlVideoSitemapWriter::WriteUrlElement(const UrlElement &url, FILE *file) {
  std::string buffer;
  buffer.append("<url>\n");
  buffer.append("  <loc>").append(EscapeEntity(url.loc())).append("</loc>\n");
  buffer.append("  <video:video />\n");  // This is required by protocol.
  buffer.append("</url>\n");

  if (static_cast<int>(buffer.length()) > volumleft()) {
    return false;
  }

  fputs(buffer.c_str(), file);
  decrease_volumleft(static_cast<int>(buffer.length()));
  return true;
}

std::vector<std::string> XmlVideoSitemapWriter::GetXmlns() {
  std::vector<std::string> xmlns = XmlSitemapWriter::GetXmlns();
  xmlns.push_back("xmlns:video=\"http://www.google.com/schemas/sitemap-video/1.0\"");
  return xmlns;
}

/////////////////////////////////////////////////////////////////////
// Implementations for XmlCodeSearchSitemapWriter

bool XmlCodeSearchSitemapWriter::WriteUrlElement(const UrlElement &url, FILE *file) {
  std::string buffer;
  buffer.append("<url>\n");
  buffer.append("  <loc>").append(EscapeEntity(url.loc())).append("</loc>\n");
  buffer.append("  <codesearch:codesearch>\n");

  // Handle filetype attribute.
  buffer.append("    <codesearch:filetype>")
    .append(url.GetAttribute("filetype"))
    .append("</codesearch:filetype>\n");

  buffer.append("  </codesearch:codesearch>\n");
  buffer.append("</url>\n");

  if (static_cast<int>(buffer.length()) > volumleft()) {
    return false;
  }

  fputs(buffer.c_str(), file);
  decrease_volumleft(static_cast<int>(buffer.length()));
  return true;
}

std::vector<std::string> XmlCodeSearchSitemapWriter::GetXmlns() {
  std::vector<std::string> xmlns = XmlSitemapWriter::GetXmlns();
  xmlns.push_back("xmlns:codesearch=\"http://www.google.com/codesearch/schemas/sitemap/1.0\"");
  return xmlns;
}

/////////////////////////////////////////////////////////////////////
// Implementations for XmlMobileSitemapWriter
void XmlMobileSitemapWriter::AddUrlElementExtension(const UrlElement& url,
                                                    std::string* buffer) {
  buffer->append("  <mobile:mobile/>");
}

std::vector<std::string> XmlMobileSitemapWriter::GetXmlns() {
  std::vector<std::string> xmlns = XmlSitemapWriter::GetXmlns();
  xmlns.push_back("xmlns:mobile=\"http://www.google.com/schemas/sitemap-mobile/1.0\"");
  return xmlns;
}
