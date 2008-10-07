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


#include "sitemapservice/lineparser.h"
#include "common/timesupport.h"
#include "common/logger.h"
#include "common/port.h"

const char* CLFParser::kName = "Common Log Format";
const char* ELFParser::kName = "W3C Extended Log Format";


///////////////////////////////////////////////////////////
// Implementations for CLFParser

CLFParser::CLFParser()
: LineParser(CLFParser::kName), entries_(7) {
}

bool CLFParser::SplitEntries(const char* str) {
  if (str == NULL) return false;
  for (int k = 0; k < static_cast<int>(entries_.size()); ++k) {
    entries_[k].clear();
  }

  // split the entries one by one
  int i = 0, len = static_cast<int>(strlen(str));
  int entryk = 0;
  for (; i < len && entryk < static_cast<int>(entries_.size()); ++i) {
    // skip all whitespaces
    while (i < len && str[i] == ' ') ++i;
    if (i == len) return false;

    char end = ' ';
    int j = i;
    if (str[i] == '"') {
      end = '"';
      j = ++i;
    } else if (str[i] == '[') {
      end = ']';
      j = ++i;
    }

    while (i < len && str[i] != end) ++i;
    entries_[entryk++].append(str + j, str + i);
  }

  // not all entires_ are filled.
  if (entryk != entries_.size()) {
    return false;
  }

  // there is still some non-whitespace left
  for (++i; i < len && str[i] == ' '; ++i);
  if (i < len) return false;

  return true;
}

// the format should be:
// remotehost rfc931 authuser [date] "request" status bytes
// The first three value may be absent with "-" instead.
LineParser::ParseResult CLFParser::Parse(const char* line, UrlRecord* record) {
  if (line == NULL || record == NULL) return PARSE_FAIL;

  if (SplitEntries(line) == false) return PARSE_FAIL;

  // handle date, like [10/Oct/2000:13:55:36 -0700]
  if (!ParseTime(entries_[3].c_str(), &record->last_access)) {
    return PARSE_FAIL;
  }
  if (record->last_access <= oldest()) {
    return PARSE_IGNORE;
  }

  // handle request, like "GET /page HTTP/1.0"
  ParseResult result = ParseRequest(entries_[4].c_str(), record->url);
  if (result != PARSE_OK) return result;

  // handle status and bytes
  record->statuscode = atoi(entries_[5].c_str());
  record->contentHashCode = atoi(entries_[6].c_str());

  // Set attributes, which can't be determined by CLF parser.
  record->host[0] = '\0';
  record->last_filewrite = -1;
  record->last_modified = -1;
  record->siteid[0] = '\0';

  return PARSE_OK;
}

bool CLFParser::ParseTime(const char* str, time_t* t) {
  tm tm;
  const char* p = strptime(str, "%e/%b/%Y:%H:%M:%S", &tm);
  if (p == NULL) return false;

  // NOTE, timezone offset is like "-0730"
  // we should convert it to -(7 * 60 + 30)
  int val = atoi(p);
  val = (val / 100) * 60 + (val % 100);

  // Convert it to GMT time
  *t = _mkgmtime(&tm);
  *t -= val * 60;  // Note the operator here (-= instead of +=).
  return true;
}

LineParser::ParseResult CLFParser::ParseRequest(const char *str, char *url) {
  if (str == NULL || strlen(str) < 3) return PARSE_FAIL;
  if (strnicmp("get", str, 3) != 0) return PARSE_IGNORE;

  // skip "GET" and whitespaces
  const char* p = str + 3;
  while (*p != '\0' && *p == ' ') ++p;
  if (*p == '\0') return PARSE_IGNORE;

  int i = 0;
  for (; i < kMaxUrlLength && *p != ' '; ++i, ++p) {
    url[i] = *p;
  }
  if (i == kMaxUrlLength) return PARSE_IGNORE;

  url[i] = '\0';
  return PARSE_OK;
}


///////////////////////////////////////////////////////////
// Implementations for ELFParser

ELFParser::ELFParser()
: LineParser(ELFParser::kName) {
  last_line_is_directive_ = false;
}


bool ELFParser::SplitEntries(const char* str) {
  if (str == NULL) return false;
  for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
    entries_[i].clear();
  }

  // split the entries one by one
  int i = 0, len = static_cast<int>(strlen(str));
  int entryk = 0;
  for (; i < len && entryk < static_cast<int>(entries_.size()); ++i) {
    // skip all whitespaces
    while (i < len && str[i] == ' ') ++i;
    if (i == len) return false;

    char end = ' ';
    int j = i;
    if (str[i] == '"') {
      end = '"';
      j = ++i;
    }

    while (i < len && str[i] != end) ++i;
    entries_[entryk++].append(str + j, str + i);
  }

  // not all entires_ are filled.
  if (entryk != entries_.size()) {
    return false;
  }

  // there is still some non-whitespace left
  for (++i; i < len && str[i] == ' '; ++i);
  if (i < len) return false;

  return true;
}

bool ELFParser::ParseFields(const char* line) {
  if (strstr(line, "#Fields:") != line) return false;

  fields_.clear();
  const char* p1 = line + strlen("#Fields:");
  while (*p1 != '\0') {
    // skip all whitespaces
    while (*p1 != '\0' && *p1 == ' ') ++p1;

    const char* p2 = p1;
    while (*p1 != '\0' && *p1 != ' ') ++p1;

    if (p1 != p2) {
      fields_.push_back(std::string(p2, p1 - p2));
    }
  }
  entries_.resize(fields_.size());

  // reset index field index
  elf_status_ = -1;
  elf_method_ = -1;
  elf_uri_ = -1;
  elf_uristem_ = -1;
  elf_uriquery_ = -1;

  for (int i = 0; i < static_cast<int>(fields_.size()); ++i) {
    if (fields_[i] == "sc-status") {
      elf_status_ = i;
    } else if (fields_[i] == "cs-method") {
      elf_method_ = i;
    } else if (fields_[i] == "cs-uri") {
      elf_uri_ = i;
    } else if (fields_[i] == "cs-uri-stem") {
      elf_uristem_ = i;
    } else if (fields_[i] == "cs-uri-query") {
      elf_uriquery_ = i;
    }
  }

  if (elf_uri_ == -1 && elf_uristem_ == -1) {
    return false;
  }
  if (elf_status_ == -1) {
    return false;
  }

  return true;
}

// Date looks like:
// #Date: 1900-12-31 HH:MM:SS.S
// The SS.S is omissible.
bool ELFParser::ParseDate(const char* line) {
  if (strstr(line, "#Date:") != line) return false;

  // trim the date string
  std::string datestr(line + strlen("#Date:"));
  while (datestr.length() > 0 && datestr[0] == ' ') datestr.erase(0, 1);
  while (datestr.length() > 0 && datestr[datestr.length() - 1] == ' ') {
    datestr.erase(datestr.length() - 1);
  }

  // determine whether the time contains SS.S or not
  int cnt = 0;
  for (int i = 0; i < static_cast<int>(datestr.length()); ++i) {
    if (datestr[i] == ':') ++cnt;
  }
  if (cnt == 1) datestr.append(":00");

  // parse date value
  tm tm;
  const char* p = strptime(datestr.c_str(), "%Y-%m-%e %H:%M:%S", &tm);
  if (p == NULL) return false;
  date_ = _mkgmtime(&tm);

  return true;
}

LineParser::ParseResult ELFParser::Parse(const char* line, UrlRecord* record) {
  if (line == NULL || record == NULL) return PARSE_FAIL;
  
  if (line[0] == '#') {
    // new log entry begins.
    if (last_line_is_directive_ == false) {
      last_line_is_directive_ = true;
      date_ = -1;
      fields_.clear();
    }

    if (strstr(line, "#Fields:") == line) {
      if (!ParseFields(line)) {
        Logger::Log(EVENT_CRITICAL, "#Fields in log is not sufficient. [%s]",
                  line);
      }
    } else if (strstr(line, "#Date:") == line) {
      if (!ParseDate(line)) {
        Logger::Log(EVENT_CRITICAL, "#Date in log is unrecognized. [%s]",
                  line);
      }
    }
    return PARSE_IGNORE;
  } else {
    last_line_is_directive_ = false;
    if (SplitEntries(line) == false) {
      return PARSE_FAIL;
    }

    if (fields_.size() == 0 || date_ == -1 || date_ <= oldest()) {
      return PARSE_IGNORE;
    }

    if (elf_method_ != -1) {
      if (stricmp("GET", entries_[elf_method_].c_str()) != 0) {
        return PARSE_IGNORE;
      }
    }

    if (elf_status_ != -1) {
      record->statuscode = atoi(entries_[elf_status_].c_str());
    } else {
      return PARSE_IGNORE;
    }

    if (elf_uri_ != -1) {
      record->url[kMaxUrlLength - 1] = '\0';
      strncpy(record->url, entries_[elf_uri_].c_str(), kMaxUrlLength);
      // url is too long
      if (record->url[kMaxUrlLength - 1] != '\0') return PARSE_IGNORE;
    } else if (elf_uristem_ != -1) {
      const char* stem = entries_[elf_uristem_].c_str();
      size_t stemlen = strlen(stem);
      if (elf_uriquery_ != -1) {
        const char* query = entries_[elf_uriquery_].c_str();
        if (stemlen + strlen(query) + 1 >= kMaxUrlLength) {
          return PARSE_IGNORE;
        }
        strcpy(record->url, stem);
        record->url[stemlen] = '?';
        strcpy(record->url + stemlen + 1, query);
      } else {
        if (stemlen >= kMaxUrlLength) return PARSE_IGNORE;
        strcpy(record->url, stem);
      }
    } else {
      return PARSE_IGNORE;
    }

    record->last_access = date_;

    // set attribute, which can't be determined from ELF log.
    record->contentHashCode = -1;
    record->host[0] = '\0';
    record->last_filewrite = -1;
    record->last_modified = -1;
    record->siteid[0] = '\0';

    return PARSE_OK;
  }
}
