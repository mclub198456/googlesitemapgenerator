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


// UrlProviderInfo contains the runtime information for url providers, which
// runs periodically.  Currently, UrlProviderInfo is used by log parser and
// file scanner as their runtime information.
// It consists of three attributes, "success" flag, "last_update" time, and
// "urls_count" value.
// Note, this class is not thread-safe.

#ifndef SITEMAPSERVICE_URLPROVIDERINFO_H__
#define SITEMAPSERVICE_URLPROVIDERINFO_H__

#include <time.h>
#include <string>
#include "sitemapservice/baseruntimeinfo.h"

class UrlProviderInfo : public BaseRuntimeInfo {
 public:
  // Constructor. Reset() method is called.
  UrlProviderInfo();

  // Empty destructor.
  ~UrlProviderInfo() {}

  // "success" flag represents whether last run is successful or not.
  // Default value is "false".
  bool success() const { return success_; }
  void set_success(bool success) { success_ = success; }

  // "last_update" represents the last run time of the url provider.
  // Default value is "-1", representing the provider never works.
  time_t last_update() const { return last_update_; }
  void set_last_update(time_t last_update) { last_update_ = last_update; }

  // "urls_count" represents the total number of URLs provided by the provider
  // during last running period.
  // Default value is "0".
  int urls_count() const { return urls_count_; }
  void set_urls_count(int urls_count) { urls_count_ = urls_count; }

  // Save the runtime info to given XML element.
  virtual bool Save(TiXmlElement* element);

  // Reset all the fields to default values.
  virtual void Reset();

 private:
  // Whether last run is successful.
  bool success_;

  // The last update time.
  time_t last_update_;

  // How may url is retrieved last time.
  int urls_count_;
};

#endif  // SITEMAPSERVICE_URLPROVIDERINFO_H__
