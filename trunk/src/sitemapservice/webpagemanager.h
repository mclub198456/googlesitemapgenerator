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

// This file defines a class to deal with web page files.
// It finds the right file to send to client according to the request.

#ifndef WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_WEBPAGEMANAGER_H__
#define WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_WEBPAGEMANAGER_H__

#include "common/util.h"
#include "common/httprequest.h"

class WebPageManager {
public:
  // The pattern for parameters that will be inserted into main.html
  // It's a way to pass some data to UI web files
  struct ParamPattern {
    std::string find_pattern;
    std::string replace_pattern;
    int param_num;
    int max_length;
  };

  // Returns the root path of web files
  static std::string GetLocalPath();

  // Load main.html file and replace param in main HTML page.
  static bool ConstructMainPage(std::string* html_string, 
                                const std::string& session_id,
                                bool need_login);

  // Translate URL path to file path according to language.
  static std::string TranslatePath(const std::string& path, const std::string& lang);

  ///////////////////////////////////////////////////////////
  // Following methods are not expected to be used directly.
  // Inserts the session id and login flag into the main.html
  static bool FillMainPageParam(std::string* html, 
                                const std::string& session_id, 
                                bool login);

  // Inserts the parameters into HTML string
  static bool InsertParamToHtml(std::string* html_string, 
                                const ParamPattern& param_pattern, 
                                Util::StringVector params);

private:

  // Replaces all the 'str_src' sub-strings in the 'str_big' with 'str_dst' string.
  // Returns the number of the 'str_src' being replaced.
  static int StringReplace(std::string* str_big, 
    const std::string& str_src, 
    const std::string& str_dst);

  // file paths
  static const std::string kMainFile;
  static const std::string kLocalFilePath;

  // ParamPatterns for main.html
  static const ParamPattern kLoginFlagPattern;
  static const ParamPattern kSessionIdPattern;
  static const ParamPattern kDebugFlagPattern;
};
#endif
