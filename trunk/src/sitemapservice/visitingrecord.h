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

// This struct reprents a record for a specific URL's visiting history.
// It is used by application to store the basic information of URL.
// Besides url string, which is has a volatile length, all the other fields
// are exposed to user directly. And for url string, get/set method is provided
// for easy usage.

#ifndef SITEMAPSERVICE_VISITINGRECORD_H__
#define SITEMAPSERVICE_VISITINGRECORD_H__

#include <time.h>
#include "common/basictypes.h"
#include "common/url.h"

class VisitingRecord {

 public:
  VisitingRecord() {
    url_ = NULL;
  }

  // Copy constructor.
  // Url string is deeply copied.
  VisitingRecord(const VisitingRecord& another) {
    url_ = NULL;
    *this = another;
  }

  ~VisitingRecord() {
    if (url_ != NULL) delete[] url_;
  }

  // Override operator=.
  // Url string is deeply copied.
  const VisitingRecord& operator= (const VisitingRecord& another) {
    if (url_ != NULL) delete[] url_;
    memcpy(this, &another, sizeof(VisitingRecord));

    if (url_ != NULL) {
      url_ = new char[url_length_ + 1];
      strcpy(url_, another.url());
    }

    return another;
  }

  // Update url string.
  // Both url finger print and url length are updated.
  void update_url(const char* url) {
    if (url_ != NULL) delete[] url_;

    url_length_ = static_cast<int>(strlen(url));
    fingerprint_ = Url::FingerPrint(url);

    url_ = new char[url_length_ + 1];
    strcpy(url_, url);
  }

  void set_url(char* url) {
    url_ = url;
  }
  void set_url_length(int length) {
    url_length_ = length;
  }
  void set_fingerprint(const UrlFprint& fingerprint) {
    fingerprint_ = fingerprint;
  }

  inline const char* url() const {
    return url_;
  }
  inline int url_length() const {
    return url_length_;
  }
  inline const UrlFprint& fingerprint() const {
    return fingerprint_;
  }

  // Time when the url visiting record first comes.
  time_t first_appear;

  // The latest time when the url was visited.
  time_t last_access;

  // The last time when content pointed by the url is changed.
  time_t last_change;

  // Number of visiting times.
  int count_access;

  // Number of changing times.
  int count_change;

  // Hash code of latest content pointed by the url.
  int64 last_content;

 private:
  // The raw, encoded url, for example: "/dir1/query?key=value"
  char* url_;

  int url_length_;

  UrlFprint fingerprint_;
};

#endif // SITEMAPSERVICE_VISITINGRECORD_H__

