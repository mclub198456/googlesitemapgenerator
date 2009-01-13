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


#include "sitemapservice/httplanguageheaderparser.h"

#include <math.h>

#include "common/logger.h"
#include "common/util.h"


void HttpLanguageHeaderParser::GetAllLanguages(Map* lang_map, 
                                               const std::string& accept_lang) {
  lang_map->clear();

  // zh-cn, en-us;q=0.7, en;q=0.3
  Util::StringVector segments;
  Util::StrSplit(accept_lang, ',', &segments);


  //  en-us;q=0.7
  for (size_t i = 0; i < segments.size(); i++) {
    std::string language, quality_value;
    Util::StringVector res, res1;
    if (Util::StrSplit(segments[i], ';', &res) == 2) {
      language = res[0];
      if (Util::StrSplit(res[1], '=', &res1) == 2) {
        quality_value = res1[1];
      } else {
        quality_value = "1";
      }
    } else {
      // default quality value is 1
      language = segments[i];
      quality_value = "1";
    }

    char* stop;
    double qvalue = strtod(quality_value.c_str(), &stop);
    if (qvalue == HUGE_VAL || qvalue == -HUGE_VAL || qvalue == 0) 
      qvalue = 1;// default value

    (*lang_map)[language] = qvalue;
  }  
}

bool HttpLanguageHeaderParser::IsSupportedLanguage(const std::string& lang) {
  return lang.compare("en-us") == 0 ||
    lang.compare("zh-cn") == 0 ||
    lang.compare("en") == 0 ||
    lang.compare("zh") == 0 ;
}

void HttpLanguageHeaderParser::FilterLanguages(Map* map) {
  // can not use std::remove_if on std::map
  // first solution: store the supported elements to another map, 
  //    then copy back(can not use std::copy too).
  // second solution: direct delete the unsupported elements.

  // first:
  //Map localMap;
  //for (Map::iterator it = map->begin();it != map->end();it++) {
  //  if (IsSupportedLanguage(it->first)) {
  //    localMap[it->first] = it->second;
  //  }    
  //}
  //
  //map->clear();
  //for (Map::iterator it = localMap.begin();it != localMap.end();it++) {
  //  (*map)[it->first] = it->second;
  //}

  // second:
  for (Map::iterator it = map->begin();it != map->end();) {
    if (!IsSupportedLanguage(it->first))
      map->erase(it++);
    else
      ++it;
  }
}

std::string HttpLanguageHeaderParser::FindPreferLanguage(Map* map) {
  double max_value = 0;
  std::string prefer_language = "";
  for (Map::iterator it = map->begin();it != map->end();it++) {
    if (it->second > max_value) {
      prefer_language = it->first;
      max_value = it->second;
    }    
  }
  return prefer_language;
}

std::string HttpLanguageHeaderParser::GetLanguageRegularName(
    const std::string& language) {
  if (language == "")
    return "en-us";

  if (language.compare("en") == 0) 
    return "en-us";

  if (language.compare("zh") == 0) 
    return "zh-cn";

  return language;
}

std::string HttpLanguageHeaderParser::GetPreferLanguage(
    const std::string& accept_lang) {
  Map lang_map; 
  std::string language;

  GetAllLanguages(&lang_map, accept_lang);
  FilterLanguages(&lang_map);
  language = FindPreferLanguage(&lang_map);
  language = GetLanguageRegularName(language);

  return language;  
}
