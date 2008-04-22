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


// LineParser can parse only a line of log.
// And it also can parse only a specific kind of log.
// Currently, two kinds of LineParser is provided:
// NCSA Common Log Format and W3C Extended Log Format.
// All these parsers are NOT thread-safe.

#ifndef SITEMAPSERVICE_LINEPARSER_H__
#define SITEMAPSERVICE_LINEPARSER_H__

#include "common/urlrecord.h"

#include <string>
#include <vector>

// abstract LineParser class.
class LineParser {
public:
  enum ParseResult {
    PARSE_OK,
    PARSE_FAIL,
    PARSE_IGNORE
  };

  LineParser(const char* name) { name_ = name; }
  ~LineParser() {}

  const std::string& name() const { return name_; }

  const time_t& oldest() const { return oldest_; }
  void set_oldest(time_t oldest) { oldest_ = oldest; }

  virtual ParseResult Parse(const char* line, UrlRecord* record) = 0;

private:
  std::string name_;

  // indicates the oldest 
  time_t oldest_;
};

// Common Log Format
// http://www.w3.org/Daemon/User/Config/Logging.html#common_logfile_format
// This is usually the default log format of apache httpd.
class CLFParser : public LineParser {
public:
  CLFParser();
  ~CLFParser() {}

  virtual ParseResult Parse(const char* line, UrlRecord* record);


  bool ParseTime(const char* str, time_t* t);

  ParseResult ParseRequest(const char* str, char* url);

private:

  // split the line, and put it into entries
  bool SplitEntries(const char* str);

  static const char* kName;

  // should always has size of 6 for CLF.
  std::vector<std::string> entries_;
};

// W3C Extended Log Format
// http://www.w3.org/TR/WD-logfile.html
// This is usually the default log format of Miscrosoft IIS.
class ELFParser : public LineParser {
public:
  ELFParser();
  ~ELFParser() {}

  virtual ParseResult Parse(const char* line, UrlRecord* record);

private:

  bool SplitEntries(const char* str);

  bool ParseFields(const char* line);

  bool ParseDate(const char* line);

  static const char* kName;

  // fields_ represents "#Fields: entry1 entry2 ..."
  // entries_ represents "value1 value2..." corresponding to fields_
  std::vector<std::string> fields_;
  std::vector<std::string> entries_;

  // represents "#Date 01-Jan-1900 00:00:00"
  time_t date_;

  // indicating last_line_is_directive
  bool last_line_is_directive_;

  // represents index of the corresponding field in fileds_ array.
  int elf_status_;
  int elf_method_;
  int elf_uri_;
  int elf_uristem_;
  int elf_uriquery_;
};

#endif // SITEMAPSERVICE_LINEPARSER_H__

