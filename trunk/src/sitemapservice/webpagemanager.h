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

#ifndef WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_WEBPAGEMANAGER_H__
#define WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_WEBPAGEMANAGER_H__

#include "common/util.h"

class HttpProto;

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

  // Inserts the parameters into HTML string
  static bool InsertParamToHtml(std::string* html_string, 
    const ParamPattern& param_pattern, 
    Util::StringVector params);

  // Inserts the session id and login flag into the main.html
  static bool FillMainPageParam(std::string* html, 
    const std::string& session_id, 
    bool login);

  // Returns the root path of web files
  static std::string GetLocalPath();

  // Returns the user's prefer language
  static std::string GetLanguage(HttpProto* r);

  // Accesses the static web files such as JavaScript/image/CSS 
  static void AccessStaticFile(HttpProto *r);

  // Accesses the 'all.js' file, which is an aggregation of the JS files in
  // main.html
  static void AccessAggregateJSFile(HttpProto *r);

  // Gets main.html file and set param in main HTML page.
  static bool GetMainFile(std::string* html_string, const std::string& session_id, bool need_login);

private:
  // Returns the paths of all the JavaScript files that will be included in the 
  // main.html, will merge them into 'all.js'.
  static const Util::StringVector& getJSFilesForAggregation();

  // Replaces all the 'strsrc' sub-strings in the 'strBig' with 'strdst' string.
  // Returns the number of the 'strsrc' being replaced.
  static int StringReplace(std::string* strBig, 
    const std::string & strsrc, 
    const std::string &strdst);

  // file paths
  static const std::string kMainFile;
  static const std::string kLocalFilePath;

  // ParamPatterns for main.html
  static const ParamPattern kLoginFlagPattern;
  static const ParamPattern kDebugFlagPattern;
  static const ParamPattern kScriptIncludePattern;
};
#endif
