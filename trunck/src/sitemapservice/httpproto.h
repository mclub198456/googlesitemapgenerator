// Copyright 2007 Google Inc. All Rights Reserved.
// Author: chaiying@google.com

#ifndef WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_HTTP_H__
#define WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_HTTP_H__

#include <string>
#include <map>
#include <vector>

#include "sitemapservice/activesocket.h"

class HttpProto {
public:
  typedef   void (*response_func) (HttpProto*); 

  static void ParseParams(std::string paramStr,
                          std::map<std::string, std::string>& params);

  HttpProto();
  void ResetRequest();
  void ResetResponse();
  bool ProcessRequest(const ActiveSocket& s);
  bool ProcessResponse(const ActiveSocket& s);
  std::string GetParam(const std::string& paramName);
  bool CheckCaching(const time_t& lastWrite);

private:
  static std::string FindSubString(const std::string& str, 
                                   const std::string& begin, 
                                   const std::string& end);
  static int Split(const std::string& str, char split, 
                   std::vector<std::string>* res);
  static void UnescapeWhitespace(std::string* val);

  bool ParseFirstLine(const ActiveSocket& s);

public:
  static const std::string kSessionIdParamName;
  static const std::string kUsernameParamName;
  static const std::string kPasswordParamName;

  std::string                        url_;
  std::string                        method_;
  std::string                        path_;
  std::map<std::string, std::string> params_;

  std::string                        accept_;
  std::string                        accept_language_;
  std::string                        accept_encoding_;
  std::string                        user_agent_;
  std::string                        referer_;
  std::string                        cookie_;
  std::string                        content_type_;
  std::string                        if_none_match_;
  std::string                        if_modified_since_;

  int                                range_first_;
  int                                range_last_;
  int                                content_length_;

  // If x_forwarded_for is not empty in http header, rempte_ip_ is set
  // to x_forwarded_for. Otherwise, it's the value read from socket.
  std::string                        remote_ip_;

  // If answer_callback_ is not null, answer_ will be ignored.
  std::string                        answer_;
  response_func                      answer_callback_;
  std::string                        answer_content_type_;
  std::string                        answer_content_range_;
  std::string                        answer_location_;
  std::string                        answer_cache_control_;
  std::string                        answer_last_modified_;
  void *                             answer_callback_context_;
  int                                answer_content_length_;

  // such as 202 OK
  std::string                        answer_status_;
};
#endif
