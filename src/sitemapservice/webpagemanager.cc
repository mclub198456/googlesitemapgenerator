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

#include "sitemapservice/webpagemanager.h"

#include <stdlib.h> 

#include "common/logger.h"
#include "common/fileutil.h"
#include "sitemapservice/httplanguageheaderparser.h"
#include "sitemapservice/securitymanager.h"

const std::string WebPageManager::kLocalFilePath = "admin-console";

const std::string WebPageManager::kMainFile = "/main.html";

const WebPageManager::ParamPattern WebPageManager::kLoginFlagPattern = {
  "var DO_LOGIN = null",
  "var DO_LOGIN = %s", // bool
  1,
  250
};

const WebPageManager::ParamPattern WebPageManager::kSessionIdPattern = {
  "var SESSION_ID = null",
  "var SESSION_ID = '%s'", // string
  1,
  250
};

const WebPageManager::ParamPattern WebPageManager::kDebugFlagPattern = {
  "var IS_DEBUG = null",
  "var IS_DEBUG = %s", // bool
  1,
  250
};

std::string WebPageManager::TranslatePath(const std::string& path,
                                          const std::string& lang) {
  std::string filename = GetLocalPath();
  filename.append("i18n/").append(lang).append(path);
  return filename;
}

bool WebPageManager::InsertParamToHtml(std::string* html_string, 
                                       const ParamPattern &param_pattern, 
                                       Util::StringVector params) {
  // Generate the Replace string
  // TODO: implement multiple params format
  std::string replace_string;  
  if (param_pattern.param_num == 1) {
    char* temp_str = new char[param_pattern.max_length];
    sprintf(temp_str, param_pattern.replace_pattern.c_str(), params[0].c_str());
    replace_string = temp_str;
    delete [] temp_str;
  } else if (param_pattern.param_num == 0) {
    replace_string = param_pattern.replace_pattern;
  }
  

  // do replacement for the HTML page
  std::string::size_type pos;
  const std::string& pattern = param_pattern.find_pattern;
  // only support single '*' in find pattern, can use "\\*" to ignore the '*'
  if ((pos=pattern.find("*")) != std::string::npos &&
      (pos == 0 || pattern[pos-1] != '\\')) {

    std::string begin_string = pattern.substr(0, pos);
    std::string end_string = pattern.substr(pos + 1);
    std::string::size_type begin_pos, end_pos;
    
    pos = 0;
    if ((begin_pos=html_string->find(begin_string, pos)) != std::string::npos) {
      if ((end_pos=html_string->find(end_string, begin_pos)) 
          != std::string::npos) {
        html_string->replace(begin_pos, end_pos - begin_pos + end_string.size(), 
                             replace_string);
        pos = begin_pos + replace_string.size();
        return true;
      }
    }
    return false;
  } else { // no '*'
    int count = StringReplace(html_string, param_pattern.find_pattern, 
      replace_string);

    return count > 0;
  }
}

bool WebPageManager::FillMainPageParam(std::string* html, 
                                       const std::string& session_id, 
                                       bool login) {
  // session id
  Util::StringVector params;
  params.push_back(session_id);
  if (!InsertParamToHtml(html, kSessionIdPattern, params)) {
    Logger::Log(EVENT_ERROR, "Set login flag to main page failed!!");
    return false;
  }

  // login flag
  params.clear();
  params.push_back(login ? "true" : "false");
  if (!InsertParamToHtml(html, kLoginFlagPattern, params)) {
    Logger::Log(EVENT_ERROR, "Set login flag to main page failed!!");
    return false;
  }

  // debug flag
  params.clear();

#ifdef _DEBUG
  params.push_back("true");
  if (!InsertParamToHtml(html, kDebugFlagPattern, params)) {
    Logger::Log(EVENT_ERROR, "Set debug flag to main page failed!!");
    return false;
  }
#endif 

  return true;
}

std::string WebPageManager::GetLocalPath() {
#ifdef _DEBUG
  std::string path;
  FileUtil::LoadFile(Util::GetApplicationDir().append("/config").c_str(), &path);
  path.resize(path.find_first_of("\r\n"));
  path.push_back(FileUtil::kPathSeparator);
  return path;
#else
  std::string path(Util::GetApplicationDir());
  path.push_back(FileUtil::kPathSeparator);
  path.append(kLocalFilePath).push_back(FileUtil::kPathSeparator);
  return path;
#endif
}

int WebPageManager::StringReplace(std::string* str_big, 
                                  const std::string & str_src, 
                                  const std::string &str_dst) {
  std::string::size_type pos = 0;
  std::string::size_type src_len=str_src.size();
  std::string::size_type dst_len=str_dst.size();
  int count = 0;
  while( (pos=str_big->find(str_src, pos)) != std::string::npos){
    str_big->replace(pos, src_len, str_dst);
    pos += dst_len;
    count++;
  }
  return count;
}

bool WebPageManager::ConstructMainPage(std::string* html_string, 
                                       const std::string& session_id, 
                                       bool need_login) {
  std::string filename = GetLocalPath() + kMainFile;
  if (!FileUtil::LoadFile(filename.c_str(), html_string)) {
    Logger::Log(EVENT_ERROR,
      "!!Failed to load main html file: %s", 
      filename.c_str());
    return false;
  }
  return FillMainPageParam(html_string, session_id, need_login);
}
