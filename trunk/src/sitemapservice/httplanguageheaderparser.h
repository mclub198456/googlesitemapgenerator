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


#ifndef SITEMAPSERVICE_HTTPLANGUAGEHEADERPARSER_H__
#define SITEMAPSERVICE_HTTPLANGUAGEHEADERPARSER_H__

#include <map>
#include <string>

// Parses the HTTP 'accept-language' header content
class HttpLanguageHeaderParser {
  // The type of maps for language name and its weight
  typedef std::map<std::string, double> Map;

public:
  // Returns the language which has the heaviest weight in the "accept_lang". 
  // "accept_lang" is the 'accept-language' header content
  static std::string getPreferLanguage(const std::string& accept_lang);

  // Returns true if the "lang" is supported by the Generator
  static bool isSupportedLanguage(const std::string& lang);

  // Returns the formal name of the language that used in the Generator
  static std::string getLanguageRegularName(const std::string& language);

private:
  // Parses all the language in the "accept_lang" and 
  // put them into the "lang_map"
  static void getAllLanguages(Map* lang_map, const std::string& accept_lang);

  // Removes all the unsupported language out of the "map"
  static void filterLanguages(Map* map);

  // Returns the the language which has the heaviest weight in the "map"
  static std::string findPreferLanguage(Map* map);

};
#endif
