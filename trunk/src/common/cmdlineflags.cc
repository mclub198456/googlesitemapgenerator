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

#include "common/cmdlineflags.h"
#include "common/port.h"

CmdLineFlags CmdLineFlags::instance_;

CmdLineFlags::CmdLineFlags() {
  check_apache_conf_ = false;
  check_remote_admin_ = false;

  check_file_ = false;
  check_site_id_ = false;
}

bool CmdLineFlags::Parse(int argc, const char* argv[]) {
  check_apache_conf_ = false;
  check_remote_admin_ = false;
  check_file_ = false;
  check_site_id_ = false;

  for (int i = 2; i < argc; ++i) {

    if (strstr(argv[i], "apache_conf=") != NULL) {
      apache_conf_ = argv[i] + strlen("apache_conf=");
      check_apache_conf_ = true;

    } else if (strstr(argv[i], "remote_admin=") != NULL) {
      check_remote_admin_ = true;
      std::string value = argv[i] + strlen("remote_admin=");
      if (stricmp("true", value.c_str()) == 0) {
        remote_admin_ = true;
      } else if (stricmp("false", value.c_str()) == 0) {
        remote_admin_ = false;
      } else {
        fprintf(stderr, "remote_admin should be true or false.");
        return false;
      }
    } else if (strstr(argv[i], "site_id=") != NULL) {
      site_id_ = argv[i] + strlen("site_id=");
      check_site_id_ = true;

    } else if (strstr(argv[i], "file=") != NULL) {
      file_= argv[i] + strlen("file=");
      check_file_ = true;

    } 
  }

  return true;
}
