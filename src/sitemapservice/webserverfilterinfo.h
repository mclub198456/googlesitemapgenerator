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


// WebServerFilterInfo contains the runtime information for webserver filter.
// This runtime information is not provided by webserver filter itself, but is
// calculated from URLs received by service.
// It consists of one attribute: "urls_count" value.
// Note, this class is not thread-safe.

#ifndef SITEMAPSERVICE_WEBSERVERFILTERINFO_H__
#define SITEMAPSERVICE_WEBSERVERFILTERINFO_H__

#include <time.h>
#include <string>
#include "sitemapservice/baseruntimeinfo.h"

class WebServerFilterInfo : public BaseRuntimeInfo {
 public:
  // Constructor. Reset() is invoked.
  WebServerFilterInfo();

  // Empty destructor.
  ~WebServerFilterInfo() {}

  // "urls_count" represents the total number of URLs sent from webserver
  // filter since application is started.
  // Default value is "0".
  int64 urls_count() const { return urls_count_; }
  void set_urls_count(int64 urls_count) { urls_count_ = urls_count; }

  // Save the runtime info to given XML element.
  virtual bool Save(TiXmlElement* element);

  // Reset all the fields to default values.
  virtual void Reset();

 private:
  // How may url is retrieved.
  int64 urls_count_;
};

#endif  // SITEMAPSERVICE_WEBSERVERFILTERINFO_H__
