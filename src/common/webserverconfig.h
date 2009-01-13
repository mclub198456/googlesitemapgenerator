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


// WebserverConfig class defines the configuration values, which can be
// retrieved from webserver settings. These values are independent with this
// application.
// This class only defines the variables, but doesn't implement any function
// to retrieve values from webserver setting. It is the base class for all
// kinds of webserver configuration class.

#ifndef COMMON_WEBSERVERCONFIG_H__
#define COMMON_WEBSERVERCONFIG_H__

#include <string>
#include <vector>

class WebserverConfig {
 public:
  WebserverConfig() {}
  virtual ~WebserverConfig() {}

  // Load configuration from web server.
  virtual bool Load() = 0;

  // Getter/setter for configuration values.
  const std::vector<std::string> site_ids() const { return site_ids_; }
  void set_site_ids(const std::vector<std::string> & site_ids) {
    site_ids_ = site_ids;
  }

  const std::vector<std::string> names() const { return names_; }
  void set_names(const std::vector<std::string> & names) {
    names_ = names;
  }

  const std::vector<std::string> physical_paths() const {
    return physical_paths_;
  }
  void set_physical_paths(const std::vector<std::string> & physical_paths) {
    physical_paths_ = physical_paths;
  }

  const std::vector<std::string> host_urls() const { return host_urls_; }
  void set_host_urls(const std::vector<std::string> & host_urls) {
    host_urls_ = host_urls;
  }

  const std::vector<std::string> log_paths() const { return log_paths_; }
  void set_log_paths(const std::vector<std::string>& log_paths) {
    log_paths_ = log_paths;
  }

  void RemoveAdminConsoleSite();

 protected:
  // An array of site IDs.
  // These sites are currently configured sites in webserver.
  std::vector<std::string>          site_ids_;

  // Web site names.
  // This array should have same length with site_ids_.
  // And names_[i] belongs to site_ids_[i];
  // Name value is required for each site.
  std::vector<std::string>          names_;

  // Root directory of web sites.
  // This array should have same length with site_ids_.
  // Physical_path value is required for each site.
  std::vector<std::string>          physical_paths_;
  
  // Host url of web sites.
  // This array should have same length with site_ids_.
  // If site-k doesn't have host url, host_urls_[k] should be empty.
  std::vector<std::string>          host_urls_;
  
  // Root directory of web sites.
  // This array should have same length with site_ids_.
  // But the value could be empty.
  std::vector<std::string>          log_paths_;
};

#endif  // COMMON_WEBSERVERCONFIG_H__
