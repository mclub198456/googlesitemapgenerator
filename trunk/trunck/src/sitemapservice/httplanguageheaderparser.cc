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


#include "sitemapservice/httplanguageheaderparser.h"

#include <math.h>

using namespace std;

bool HttpLanguageHeaderParser::strSplit(const std::string& str, 
                                        const std::string& split, 
                                        std::string* first, 
                                        std::string* second) {
  static const std::basic_string <char>::size_type npos = -1;
  std::basic_string <char>::size_type indexCh;

  indexCh = str.find_first_of(split);
  if (indexCh != npos) {
    if (first != NULL) 
      *first = str.substr(0, indexCh);
    if (second != NULL)
      *second = str.substr(indexCh+1, str.length()-(indexCh+1));
    return true;
  } else {
    if (first != NULL) 
      *first = "";
    if (second != NULL)
      *second = "";
    return false;
  }
}

void HttpLanguageHeaderParser::addLanguage(Map* map, 
                                           const std::string& language_range) {
  //  en-us;q=0.7
  std::string language, quality, quality_value;
  if (strSplit(language_range, ";", &language, &quality)) {
    if (!strSplit(quality, "=", NULL, &quality_value))
      quality_value = "1";
  } else {
    // default quality value is 1
    language = language_range;
    quality_value = "1";
  }

  char* stop;
  double qvalue = strtod(quality_value.c_str(), &stop);
  if (qvalue == HUGE_VAL || qvalue == -HUGE_VAL || qvalue == 0) 
    qvalue = 1;// default value

  (*map)[language] = qvalue;
}

void HttpLanguageHeaderParser::getAllLanguages(Map* lang_map, 
                                               const std::string& accept_lang) {
  static const std::basic_string <char>::size_type npos = -1;
  // it's used to find the ',' position
  std::basic_string <char>::size_type indexCh = 0; 

  // it's used to remember the last found ',' position
  std::basic_string <char>::size_type searchStart = 0; 

  lang_map->clear();

  // zh-cn, en-us;q=0.7, en;q=0.3
  indexCh = accept_lang.find_first_of(",", searchStart);
  while (indexCh != npos) {
    if (indexCh > searchStart) { 
      addLanguage(lang_map, 
          accept_lang.substr(searchStart, indexCh - searchStart));
    }
    searchStart = indexCh+1;
    indexCh = accept_lang.find_first_of(",", searchStart);
  }

  if (searchStart < accept_lang.length() - 1) {
    addLanguage(lang_map, 
        accept_lang.substr(searchStart, indexCh - searchStart));
  }
}

bool HttpLanguageHeaderParser::isSupportedLanguage(const std::string& lang) {
  return lang.compare("en-us") == 0 ||
    lang.compare("zh-cn") == 0 ||
    lang.compare("en") == 0 ||
    lang.compare("zh") == 0 ;
}

void HttpLanguageHeaderParser::filterLanguages(Map* map) {
  // can not use std::remove_if on std::map
  // first solution: store the supported elements to another map, 
  //    then copy back(can not use std::copy too).
  // second solution: direct delete the unsupported elements.

  // first:
  //Map localMap;
  //for (Map::iterator it = map->begin();it != map->end();it++) {
  //  if (isSupportedLanguage(it->first)) {
  //    localMap[it->first] = it->second;
  //  }    
  //}
  //
  //map->clear();
  //for (Map::iterator it = localMap.begin();it != localMap.end();it++) {
  //  (*map)[it->first] = it->second;
  //}

  // second:
  for(Map::iterator it = map->begin();it != map->end();) {
    if (!isSupportedLanguage(it->first))
      map->erase(it++);
    else
      ++it;
  }
}

std::string HttpLanguageHeaderParser::findPreferLanguage(Map* map) {
  double max_value = 0;
  std::string preferLanguage = "";
  for (Map::iterator it = map->begin();it != map->end();it++) {
    if (it->second > max_value) {
      preferLanguage = it->first;
      max_value = it->second;
    }    
  }
  return preferLanguage;
}

std::string HttpLanguageHeaderParser::getLanguageRegularName(
    const std::string& language) {
  if (language == "")
    return "en-us";

  if (language.compare("en") == 0) 
    return "en-us";

  if (language.compare("zh") == 0) 
    return "zh-cn";

  return language;
}

std::string HttpLanguageHeaderParser::getPreferLanguage(
    const std::string& accept_lang) {
  Map lang_map; 
  std::string language;

  getAllLanguages(&lang_map, accept_lang);
  filterLanguages(&lang_map);
  language = findPreferLanguage(&lang_map);
  language = getLanguageRegularName(language);

  return language;  
}
