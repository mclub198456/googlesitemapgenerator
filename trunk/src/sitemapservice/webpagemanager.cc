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

#include "common/util.h"
#include "common/fileutil.h"
#include "sitemapservice/httpproto.h"
#include "sitemapservice/httplanguageheaderparser.h"
#include "sitemapservice/securitymanager.h"


const std::string WebPageManager::kLocalFilePath = "templates";
const std::string WebPageManager::kMainFile = "/main.html";

const WebPageManager::ParamPattern WebPageManager::kLoginFlagPattern = {
  "var DO_LOGIN = null",
  "var DO_LOGIN = %s", // bool
  1,
  250
};

const WebPageManager::ParamPattern WebPageManager::kDebugFlagPattern = {
  "var IS_DEBUG = null",
  "var IS_DEBUG = %s", // bool
  1,
  250
};

// aggressive js file to reduce the latency
const WebPageManager::ParamPattern WebPageManager::kScriptIncludePattern = {
  "<!--SCRIPT BEGIN-->*<!--SCRIPT END-->",
  "<script type='text/javascript' src='all.js'></script>", // bool
  0,
  250
};

const std::vector<std::string>& WebPageManager::getJSFilesForAggregation() {
  static std::vector<std::string> jsfiles;
  if (jsfiles.size() == 0) {
    jsfiles.push_back("browser.js");
    jsfiles.push_back("constval.js");
    jsfiles.push_back("utility.js");
    jsfiles.push_back("tips.js");
    jsfiles.push_back("transmarkset.js");
    jsfiles.push_back("ajax.js");
    jsfiles.push_back("servermanager.js");
    jsfiles.push_back("runtimeinfomanager.js");
    jsfiles.push_back("list.js");
    jsfiles.push_back("settingcomponents.js");
    jsfiles.push_back("sitesettings.js");
    jsfiles.push_back("xmlmanager.js");
    jsfiles.push_back("htmlmanager.js");
    jsfiles.push_back("translatemanager.js");
    jsfiles.push_back("main.js");
  }

  return jsfiles;
}
void WebPageManager::AccessStaticFile(HttpProto* r) {
  DLog(EVENT_IMPORTANT, "Get request: %s", 
    Util::EscapeLogMessage(r->path_.c_str()).c_str());

  // check the r-> path has no access to parent dir
  if (!SecurityManager::CheckPath(r)) {
    return;
  }

  std::string filename;
  // path preprocess  
  if (Util::Match(r->path_, 0, "/unittest/jsunit/")) {
    // google3 jsunit2.2 lib
    filename = "../../../third_party/java/jsunit/jsunit2.2/" + 
      r->path_.substr(strlen("/unittest/jsunit/"));
    // windows local jsunit lib
    // filename = GetLocalPath() + r->path_;
  } else if (Util::Match(r->path_, -1, "language.js")) {
    // language-related files
    // get language
    filename = 
      GetLocalPath() + "scripts/languages/" + GetLanguage(r) + r->path_;

  } else if (Util::Match(r->path_, -1, ".js")) {
    filename = GetLocalPath() + "scripts" + r->path_;
  } else if (Util::Match(r->path_, -1, ".jpg")) {
    filename = GetLocalPath() + "images" + r->path_;
  } else if (Util::Match(r->path_, -1, ".gif")) {
    filename = GetLocalPath() + "images" + r->path_;
  } else if (Util::Match(r->path_, -1, ".css")) {
    filename = GetLocalPath() + "styles" + r->path_;
  } else {
    filename = GetLocalPath() + r->path_;
  }


  DLog(EVENT_IMPORTANT, "File path: %s", 
       Util::EscapeLogMessage(filename.c_str()).c_str());

  // cache control
  if (!Util::Match(r->path_, -1, "language.js")) {  
    FileAttribute file_attr;
    if(FileUtil::GetFileAttribute(filename.c_str(), &file_attr)) {  
      if (r->CheckCaching(file_attr.last_modified)) {
        r->answer_status_ = "304 Not Modified";
        DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_status_.c_str());
        return;
      }
    }   
  }

  // load the file
  if (FileUtil::Exists(filename.c_str())) {
    std::string contentstring;

    if (FileUtil::LoadFile(filename.c_str(), &contentstring)) {
      if (Util::Match(filename, -1, ".xml")) {
        r->answer_content_type_ = "text/xml";
      } else if (Util::Match(filename, -1, ".jpg")) {
        r->answer_content_type_ = "image/jpeg";
      } else if (Util::Match(filename, -1, ".gif")) {
        r->answer_content_type_ = "image/gif";
      } else if (Util::Match(filename, -1, ".css")) {
        r->answer_content_type_ = "text/css";
      }
      r->answer_ = contentstring;
      DLog(EVENT_IMPORTANT, 
           "Send response: SUCCESS, size %d", r->answer_.length());

    } else {
      r->answer_status_ = "404 Not Found"; 
      r->answer_ = "can't load the file";
      Util::Log(EVENT_ERROR, "!!Failed to load the file: %s", 
                Util::EscapeLogMessage(filename.c_str()).c_str());
      DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
    }
  } else {
    r->answer_status_ = "404 Not Found"; 
    r->answer_ = "File not found";
    Util::Log(EVENT_ERROR, "!!Failed to find the file: %s", 
              Util::EscapeLogMessage(filename.c_str()).c_str());
    DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
  }    
}


void WebPageManager::AccessAggregateJSFile(HttpProto* r) {
  DLog(EVENT_IMPORTANT, "Get request: %s", 
       Util::EscapeLogMessage(r->path_.c_str()).c_str());

  // get last modify timestamp, it's the latest file time of all the js files
  std::string filename;
  time_t latest = 0;
  const std::vector<std::string>& jsfiles = getJSFilesForAggregation();
  std::vector<std::string>::const_iterator itr = jsfiles.begin();
  for (;itr!=jsfiles.end();itr++) {
    filename = GetLocalPath() + "scripts/" + *itr;
    DLog(EVENT_IMPORTANT, "File path: %s", 
         Util::EscapeLogMessage(filename.c_str()).c_str());

    FileAttribute file_attr;
    if(!FileUtil::GetFileAttribute(filename.c_str(), &file_attr)) {
      // if one file access time cannot get, all the files should be sent
      latest = 0;
      break;
    }

    if(file_attr.last_modified > latest) {
      latest = file_attr.last_modified;
    }
  }

  // check the timestamp
  if (latest != 0) {
    if (r->CheckCaching(latest)) {
      r->answer_status_ = "304 Not Modified";
      DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_status_.c_str());
      return;
    }
  }

  // load the file
  std::string allcontent;
  itr = jsfiles.begin();
  for (;itr!=jsfiles.end();itr++) {
    filename = GetLocalPath() + "scripts/" + *itr;
    if (FileUtil::Exists(filename.c_str())) {
      std::string contentstring;

      if (FileUtil::LoadFile(filename.c_str(), &contentstring)) {
        allcontent.append(contentstring);

      } else {
        r->answer_status_ = "404 Not Found"; 
        r->answer_ = "can't load the file";
        Util::Log(EVENT_ERROR, "!!Failed to load the file: %s", 
                  Util::EscapeLogMessage(filename.c_str()).c_str());
        DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
        return;
      }
    } else {
      r->answer_status_ = "404 Not Found"; 
      r->answer_ = "File not found";
      Util::Log(EVENT_ERROR, "!!Failed to find the file: %s", 
                Util::EscapeLogMessage(filename.c_str()).c_str());
      DLog(EVENT_IMPORTANT, "Send response: %s", r->answer_.c_str());
      return;
    }
  }

  r->answer_ = allcontent;
  DLog(EVENT_IMPORTANT, "Send response: SUCCESS, size %d", 
       r->answer_.length());
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

    std::string beginStr = pattern.substr(0, pos);
    std::string endStr = pattern.substr(pos + 1);
    std::string::size_type beginPos, endPos;
    
    pos = 0;
    if ((beginPos=html_string->find(beginStr, pos)) != std::string::npos) {
      if ((endPos=html_string->find(endStr, beginPos)) != std::string::npos) {
        html_string->replace(beginPos, endPos - beginPos + endStr.size(), 
                             replace_string);
        pos = beginPos + replace_string.size();
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

  // login flag
  params.clear();
  params.push_back(login ? "true" : "false");
  if (!InsertParamToHtml(html, kLoginFlagPattern, params)) {
    Util::Log(EVENT_ERROR, "Set login flag to main page failed!!");
    return false;
  }

  // debug flag
  params.clear();

#ifdef _DEBUG
  params.push_back("true");
  if (!InsertParamToHtml(html, kDebugFlagPattern, params)) {
    Util::Log(EVENT_ERROR, "Set debug flag to main page failed!!");
    return false;
  }
#endif 

#ifndef _DEBUG
  // js script file 'all.js'
  if (!InsertParamToHtml(html, kScriptIncludePattern, params)) {
    Util::Log(EVENT_ERROR, "Replace js file to main page failed!!");
    return false;
  }
#endif

  return true;
}

std::string WebPageManager::GetLocalPath() {
#ifdef _DEBUG
  // restart from command should failed before the unittest component checkin.

  // It allows the server access the UI js files directly, so developers don't
  // need to build the solution each time they change the js files.
  // It also allows the access to custom unittest HTML files and the external 
  // jsunit lib on the Windows platform.

  FILE* fd = fopen("config", "r");
  if (fd == NULL)
    return kLocalFilePath + "/";

  char buff[250];
  size_t numread = fread( buff, sizeof( char ), 250, fd );
  if (ferror(fd)) {
    fclose(fd);
    return kLocalFilePath + "/";
  } else {
    fclose(fd);
  }

  buff[numread] = 0;

  std::string path;
  path.append(buff).append("/");
  return path;
#else
  return Util::GetApplicationDir().append("/")
    .append(kLocalFilePath)
    .append("/");
#endif
}

std::string WebPageManager::GetLanguage(HttpProto* r) {
  // get user prefer language
  std::string language = r->GetParam("hl");
  if (language == "" || 
    !HttpLanguageHeaderParser::isSupportedLanguage(language)) {
      language = 
        HttpLanguageHeaderParser::getPreferLanguage(r->accept_language_);
  } else {
    language = 
      HttpLanguageHeaderParser::getLanguageRegularName(language);
  }
  return language;
}

int WebPageManager::StringReplace(std::string* strBig, 
                                  const std::string & strsrc, 
                                  const std::string &strdst) {
  std::string::size_type pos = 0;
  std::string::size_type srclen=strsrc.size();
  std::string::size_type dstlen=strdst.size();
  int count = 0;
  while( (pos=strBig->find(strsrc, pos)) != std::string::npos){
    strBig->replace(pos, srclen, strdst);
    pos += dstlen;
    count++;
  }
  return count;
}

bool WebPageManager::GetMainFile(std::string* html_string, 
                                 const std::string& session_id, 
                                 bool need_login) {
  std::string filename = GetLocalPath() + kMainFile;
  if (!FileUtil::LoadFile(filename.c_str(), html_string)) {
    Util::Log(EVENT_ERROR,
      "!!Failed to load main html file: %s", 
      filename.c_str());
    return false;
  }
  return FillMainPageParam(html_string, session_id, need_login);
}
