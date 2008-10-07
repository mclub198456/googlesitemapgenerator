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


#include "sitemapservice/robotstxtfilter.h"

#include <fstream>
#include "common/port.h"
#include "common/logger.h"
#include "common/url.h"

void RobotsTxtFilter::Initialize(const char* path) {
  // Open robots.txt.
  std::ifstream file_in(path);
  if (file_in.fail()) {
    Logger::Log(EVENT_CRITICAL, "Robots.txt in (%s) can't be opened.", path);
    return;
  } else {
    Logger::Log(EVENT_CRITICAL, "Try to parse robots.txt at (%s).", path);
  }

  // Clear previous rules.
  rules_.clear();

  std::string line;
  bool all_agent_found = false;  // Whether user-agent "*" found
  bool last_line_rule = true;  // whether last line is a rule line
  while (std::getline(file_in, line)) {
    std::string::size_type pos1 = 0, pos2 = 0;

    // Erase comments.
    pos1 = line.find_first_of("#");
    if (pos1 != std::string::npos) {
      line.erase(pos1);
    }

    // Skip heading white spaces.
    pos1 = line.find_first_not_of(" \t");
    if (pos1 == std::string::npos) continue;

    // Get everything before first ":", besides whitespaces.
    pos2 = line.find_first_of(" \t:", pos1);
    if (pos2 == std::string::npos) continue;
    std::string field = line.substr(pos1, pos2 - pos1);

    // Get everything after ":", besides whitespaces.
    std::string value;
    pos1 = line.find_first_not_of(" \t:", pos2);
    if (pos1 != std::string::npos) {
      pos2 = line.find_first_of(" \t", pos1);
      if (pos2 == std::string::npos) {
        value = line.substr(pos1);
      } else {
        value = line.substr(pos1, pos2 - pos1);
      }
    }

    if (stricmp(field.c_str(), "User-agent") == 0) {
      if (strcmp(value.c_str(), "*") == 0) {
        all_agent_found = true;
      } else if (last_line_rule) {
        all_agent_found = false;
      }
      last_line_rule = false;

    } else if (all_agent_found && stricmp(field.c_str(), "Allow") == 0) {
      last_line_rule = true;
      rules_.push_back(std::make_pair(true, value));
    } else if (all_agent_found&& stricmp(field.c_str(), "Disallow") == 0) {
      last_line_rule = true;
      rules_.push_back(std::make_pair(false, value));
    }
  }

  file_in.close();
}

bool RobotsTxtFilter::Accept(const char* url, int urllen) {
  std::string unencoded;
  if (!Url::UnescapeUrl(url, &unencoded)) {
    return false;
  }

  for (int i = 0; i < static_cast<int>(rules_.size()); ++i) {
    if (rules_[i].second.length() == 0) {
      return !rules_[i].first;
    } else {
      if (unencoded.length() >= rules_[i].second.length()) {
        if (strncmp(unencoded.c_str(), rules_[i].second.c_str(),
                    rules_[i].second.length()) == 0) {
          return rules_[i].first;
        }
      }
    }
  }

  return true;
}

