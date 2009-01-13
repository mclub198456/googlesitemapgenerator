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

#include "common/cmdlineflags.h"
#include "common/port.h"

CmdLineFlags CmdLineFlags::instance_;

CmdLineFlags::CmdLineFlags() {
  check_apache_conf_ = false;
  check_apache_group_ = false;
  check_remote_admin_ = false;

  check_file_ = false;
  check_site_id_ = false;
}

static bool ParseBool(const char* argv, const char* name, bool* value) {
  std::string value_str = argv + strlen(name) + 1;
  if (stricmp("true", value_str.c_str()) == 0) {
    *value = true;
  } else if (stricmp("false", value_str.c_str()) == 0) {
    *value = false;
  } else {
    fprintf(stderr, "[%s] should be true or false.", name);
    return false;
  }
  return true;
}

bool CmdLineFlags::Parse(int argc, const char* argv[]) {
  check_apache_conf_ = false;
  check_apache_group_ = false;
  check_remote_admin_ = false;
  check_file_ = false;
  check_site_id_ = false;

  for (int i = 2; i < argc; ++i) {

    if (strstr(argv[i], "apache_conf=") != NULL) {
      apache_conf_ = argv[i] + strlen("apache_conf=");
      check_apache_conf_ = true;

    } else if (strstr(argv[i], "apache_group=") != NULL) {
      apache_group_ = argv[i] + strlen("apache_group=");
      check_apache_group_ = true;

    } else if (strstr(argv[i], "remote_admin=") != NULL) {
      check_remote_admin_ = true;
      if (!ParseBool(argv[i], "remote_admin", &remote_admin_)) {
        return false;
      }
    } else if (strstr(argv[i], "overwrite=") != NULL) {
      check_overwrite_ = true;
      if (!ParseBool(argv[i], "overwrite", &overwrite_)) {
        return false;
      }
    } else if (strstr(argv[i], "auto_submission=") != NULL) {
      check_auto_submission_ = true;
      if (!ParseBool(argv[i], "auto_submission", &auto_submission_)) {
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


