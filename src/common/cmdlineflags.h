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

// This class represents command line flags. It defines all command line options
// provided by sitemap generator application. Besides the flags themselves,
// there is a boolean value used to indicate whether corresponding flag is
// provided by user of not. Besides accessors for those flags, this class also
// provides a method to parse from command line.

#ifndef COMMON_CMDLINEFLAGS_H__
#define COMMON_CMDLINEFLAGS_H__

#include <string>

class CmdLineFlags {
 public:
  CmdLineFlags();
  ~CmdLineFlags() {}

  // Parse command line flags from given args.
  // Usually, these args should come from main function.
  bool Parse(int argc, const char* argv[]);

  // Check whether "apache_conf" flag exists.
  const bool check_apache_conf() const {
    return check_apache_conf_;
  }
  // Get "apache_conf" flag.
  const std::string& apache_conf() const {
    return apache_conf_;
  }

  // Check whether "apache_group" flag exists.
  const bool check_apache_group() const {
    return check_apache_group_;
  }
  // Get "apache_group" flag.
  const std::string& apache_group() const {
    return apache_group_;
  }

  // Check whether "remote_admin" flag exists.
  const bool check_remote_admin() const {
    return check_remote_admin_;
  }
  // Get "remote_admin" flag value.
  const bool remote_admin() const {
    return remote_admin_;
  }

  // Check whether "site_id" flag exists.
  const bool check_site_id() const {
    return check_site_id_;
  }
  // Get "site_id" flag vlaue.
  const std::string& site_id() const {
    return site_id_;
  }

  // Check whether "file" flag exists.
  const bool check_file() const {
    return check_file_;
  }
  // Get "file" flag value.
  const std::string& file() const {
    return file_;
  }

  // Check whether "overwrite" flag exists.
  const bool check_overwrite() const {
    return check_overwrite_;
  }
  // Get "overwrite" flag value.
  const bool overwrite() const {
    return overwrite_;
  }

  // Check whether "auto_submission" flag exists.
  const bool check_auto_submission() const {
    return check_auto_submission_;
  }
  // Get "auto_submission" flag value.
  const bool auto_submission() const {
    return auto_submission_;
  }

  // Get global instance of CmdLineFlags.
  // The flags are parsed in the program entry point.
  // No other instance should be instantiated in binary code.
  static CmdLineFlags* GetInstance() {
    return &instance_;
  }

 private:
  // Global instance of this class.
  static CmdLineFlags instance_;

  bool check_remote_admin_;
  bool remote_admin_;

  bool check_apache_conf_;
  std::string apache_conf_;

  bool check_apache_group_;
  std::string apache_group_;

  bool check_site_id_;
  std::string site_id_;

  bool check_file_;
  std::string file_;

  bool check_overwrite_;
  bool overwrite_;

  bool check_auto_submission_;
  bool auto_submission_;
};

#endif  // COMMON_CMDLINEFLAGS_H__
