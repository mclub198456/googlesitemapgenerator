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


// BlogSearchPingServiceInfo contains the runtime information for
// BlogSearchPingService.
// It consists of three attributes, "success" flag, "last_ping" time, and
// "last_url" value.
// Note, this class is not thread-safe.

#ifndef SITEMAPSERVICE_BLOGSEARCHPINGSERIVCEINFO_H__
#define SITEMAPSERVICE_BLOGSEARCHPINGSERIVCEINFO_H__

#include <time.h>
#include <string>
#include "sitemapservice/baseruntimeinfo.h"

class BlogSearchPingServiceInfo : public BaseRuntimeInfo {
 public:
  // Constructor. Reset() method is called.
  BlogSearchPingServiceInfo();

  // Empty destructor.
  ~BlogSearchPingServiceInfo() {}

  // "success" flag represents whether last ping is successful or not.
  // Default value is "false".
  bool success() const { return success_; }
  void set_success(bool success) { success_ = success; }

  // "last_update" represents the last ping time of the service.
  // Default value is "-1", representing the blogsearch ping service is not
  // enabled.
  time_t last_ping() const { return last_ping_; }
  void set_last_ping(time_t last_ping) { last_ping_ = last_ping; }

  // "last_url" repsents the last URL sent to the ping server.
  // Default value is "N/A", repsenting no URL is sent.
  std::string last_url() const { return last_url_; }
  void set_last_url(const std::string last_url) {
    last_url_ = last_url;
  }

  // Save the runtime info to given XML element.
  virtual bool Save(TiXmlElement* element);

  // Reset all the fields to default values.
  virtual void Reset();

 private:
  // Whether last ping is successful.
  bool success_;

  // The last ping time.
  time_t last_ping_;

  // last url which is pinged.
  std::string last_url_;
};

#endif  // SITEMAPSERVICE_BLOGSEARCHPINGSERIVCEINFO_H__

