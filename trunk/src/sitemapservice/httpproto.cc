
#include "sitemapservice/httpproto.h"
#include <sstream>
#include "common/url.h"
#include "common/util.h"

const std::string HttpProto::kSessionIdParamName = "sessionId";
const std::string HttpProto::kUsernameParamName = "username";
const std::string HttpProto::kPasswordParamName = "password";

HttpProto::HttpProto() : range_first_(-1),
range_last_(-1),
content_length_(-1),
answer_callback_(NULL),
answer_content_length_(-1) {
}

void HttpProto::ResetRequest() {
  // Default value
  content_type_ = "Content-Type: text/html; charset=utf-8";
}

void HttpProto::ResetResponse() {
  // Set default answer attributes
  answer_status_ = "200 OK";
  answer_content_type_ = "text/html; charset=utf-8";
  answer_cache_control_ = "no-cache";
}

bool HttpProto::ParseFirstLine(const ActiveSocket& s) {
  std::string line = s.ReceiveLine();

  if (line.empty()) {
    return false;
  }

  // GET/POST
  if (line.find("GET") == 0) {
    method_="GET";
  } else if (line.find("POST") == 0) {
    method_="POST";
  }

  // get url
  line = line.substr(method_.length());

  std::string::size_type pos = line.find_first_not_of(" ", 0);
  if (pos == std::string::npos)
    return false;
  line = line.substr(pos);

  std::string pattern = " HTTP/1.";
  size_t len = line.length() - 1 - pattern.length();
  if (Util::Match(line, (int)len, pattern)) {
    line = line.substr(0, len);
  }

  url_ = line;
  return true;
}

bool HttpProto::ProcessRequest(const ActiveSocket& s) {
  /* parse first line */
  if (!ParseFirstLine(s))
    return false;

  /* parse HTTP header */
  static const std::string authorization   = "Authorization: Basic ";
  static const std::string accept          = "Accept: "             ;
  static const std::string accept_language = "Accept-Language: "    ;
  static const std::string accept_encoding = "Accept-Encoding: "    ;
  static const std::string user_agent      = "User-Agent: "         ;
  static const std::string content_length  = "Content-Length: "     ;
  static const std::string referer         = "Referer: "            ;
  static const std::string x_forwarded_for = "X-Forwarded-For: "    ;
  static const std::string content_type    = "Content-Type: "       ;
  static const std::string range           = "Range: "              ;
  static const std::string cookie          = "Cookie: "             ;
  static const std::string if_modified_since = "If-Modified-Since: ";
  static const std::string if_none_match     = "If-None-Match: "    ;

  while(true) {
    std::string line=s.ReceiveLine();
    if (line.empty()) 
      break;

    if (line.compare(0, authorization.size(), authorization) == 0) {
      Util::Log(EVENT_ERROR, "not implement base64 for HTTP authorization");

    } else if (line.compare(0, if_modified_since.size(), 
                            if_modified_since) == 0) {
      if_modified_since_ = line.substr(if_modified_since.size());

    } else if (line.compare(0, if_none_match.size(), if_none_match) == 0) {
      if_none_match_ = line.substr(if_none_match.size());

    } else if (line.compare(0, accept.size(), accept) == 0) {
      accept_ = line.substr(accept.size());

    } else if (line.compare(0, accept_language.size(), accept_language) == 0) {
      accept_language_ = line.substr(accept_language.size());

    } else if (line.compare(0, accept_encoding.size(), accept_encoding) == 0) {
      accept_encoding_ = line.substr(accept_encoding.size());

    } else if (line.compare(0, user_agent.size(), user_agent) == 0) {
      user_agent_ = line.substr(user_agent.size());

    } else if (line.compare(0, referer.size(), referer) == 0) {
      referer_ = line.substr(referer.size());
      // TODO: if referrer is relative path, we should convert it to
      // absolute path.

    } else if (line.compare(0, x_forwarded_for.size(), x_forwarded_for) == 0) {
      remote_ip_ = line.substr(x_forwarded_for.size());

    } else if (line.compare(0, cookie.size(), cookie) == 0) {
      cookie_ = line.substr(cookie.size());

    } else if (line.compare(0, content_type.size(), content_type) == 0) {
      content_type_ = line.substr(content_type.size());

    } else if (line.compare(0, content_length.size(), content_length) == 0) {
      content_length_ = atoi(line.substr(content_length.size()).c_str());

    } else if (line.compare(0, range.size(), range) == 0) {
      size_t bytes_pos = line.find("bytes", range.size());
      if (bytes_pos != line.npos) {
        size_t equal_pos = line.find_first_of('=', bytes_pos);
        size_t dash_pos = line.find_first_of('-', equal_pos);
        if (equal_pos != line.npos && dash_pos != line.npos) {
          std::string first = line.substr(equal_pos + 1,
            dash_pos - equal_pos - 1);
          std::string last = line.substr(dash_pos + 1);

          if (first.length() > 0)
            range_first_ = atoi(first.c_str());
          else
            range_first_ = 0;

          if (last.length() > 0)
            range_last_ = atoi(last.c_str());
        }
      }
    }
  }

  /* get remote IP */
  if (remote_ip_.length() == 0)
    remote_ip_ = s.GetRemoteIp();

  /* get path and url params */
  std::string::size_type pos = url_.find("?");
  if (pos != std::string::npos) {    
    path_ = url_.substr(0, pos);
    ParseParams(url_.substr(pos+1));
  } else {
    path_ = url_;
  }

  /* get params in post content */
  if(method_.compare("POST") == 0) {
    ParseParams(s.ReceiveBytes(content_length_));
  }


  /* get params in referer */
  // will have lower priority than url, since it 
  // appends after the params of url
  pos = referer_.find("?");
  if (pos != std::string::npos) {
    ParseParams(referer_.substr(pos+1));
  }

  return true;
}

bool HttpProto::ProcessResponse(const ActiveSocket& s) {
  // prepare response
  if (answer_content_length_ == -1 && answer_callback_ == NULL)
    answer_content_length_ = (int)answer_.length();

  std::string rs;

  rs.reserve(400);
  rs.append("HTTP/1.1 ");
  rs.append(answer_status_ + "\n");

  time_t ltime = time(NULL);
  char* asctime_remove_nl = std::asctime(gmtime(&ltime));
  asctime_remove_nl[24] = 0;

  rs.append(std::string("Date: ") + asctime_remove_nl + " GMT\n")
    .append("Server: GoogleSitemapGeneratorWebserver (Windows)\n")
    .append("Connection: close\n")
    .append("Accept-Ranges: bytes\n");

  if (answer_cache_control_.length() != 0) {
    rs.append("Cache-control: ").append(answer_cache_control_).append("\n");
  }

  if (answer_last_modified_.length() != 0) {
    rs.append("Last-Modified: ").append(answer_last_modified_).append("\n");
  }

  if (answer_content_type_.length() != 0)
    rs.append("Content-Type: ").append(answer_content_type_ + "\n");

  if (answer_content_length_ != -1) {
    std::stringstream str_str;
    str_str << answer_content_length_;
    rs.append("Content-Length: " + str_str.str() + "\n");
  }

  if (answer_content_range_.length() != 0)
    rs.append("Content-Range: " + answer_content_range_ + "\n");

  if (answer_location_.length() != 0)
    rs.append("Location: " + answer_location_ + "\n");

  rs.append("\n");
  s.SendBytes(rs.data(), (int)rs.length());

  if (answer_callback_ != NULL) {
    answer_callback_(this);
  } else {
    s.SendBytes(answer_.data(), answer_content_length_);
  }
  return true;
}



void HttpProto::UnescapeWhitespace(std::string* val) {
  // replace '+' to ' '
  std::string::size_type pos;
  while ( (pos = val->find_first_of('+')) != std::string::npos ) {
    val->replace(pos, 1, " ");
  }
}

void HttpProto::ParseParams(std::string paramStr) {
  Util::StringVector name_value_pairs;
  Util::StrSplit(paramStr, '&', &name_value_pairs);
  for (size_t i = 0; i < name_value_pairs.size(); i++) {
    Util::StringVector name_value;
    if (Util::StrSplit(name_value_pairs[i], '=', &name_value) != 2) {
      return;// wrong param
    }
    std::string nam = name_value[0];
    std::string val = name_value[1];

    // unescapeURL
    UnescapeWhitespace(&val);
    std::string unencoded;
    Url::UnescapeUrl(val.c_str(), &unencoded);

    // insert params
    params_.insert(
        std::map<std::string,std::string>::value_type(nam, unencoded));
  }
}

std::string HttpProto::GetParam(const std::string& paramName) {
  // get param from URL, Referer and POST
  std::map<std::string, std::string>::iterator pfind = 
      params_.find(paramName);
  if (pfind != params_.end()) {
    return pfind->second;
  }
  
  //get param from cookie
  const std::string kCookieName = "sitemapsetting=";
  const std::string kCookieSplit = ";";
  std::string cookie = FindSubString(cookie_, kCookieName, kCookieSplit);
  if (!cookie.empty()) {
    // get session id
    if (paramName == HttpProto::kSessionIdParamName) {
      const std::string kSessionIdCookieName = "sid:";
      const std::string kCookieValueSplit = "&";
      std::string sessionId = 
          FindSubString(cookie, kSessionIdCookieName, kCookieValueSplit);
      return sessionId;
    }
  }  

  return "";
}


std::string HttpProto::FindSubString(const std::string& str, 
                                     const std::string& begin, 
                                     const std::string& end) {
  std::string::size_type begin_pos = str.find(begin);
  if (begin_pos != std::string::npos) {
    begin_pos += begin.size();
    std::string::size_type end_pos = str.find(end, begin_pos);
    if (end_pos == std::string::npos) {
      return str.substr(begin_pos);
    } else {
      return str.substr(begin_pos, end_pos - begin_pos);
    }
  }
  return "";
}

bool HttpProto::CheckCaching(const time_t& lastWrite) {
  char buff[128];

  // convert 'time_t' to struct tm (GMT)
  tm* gmt_p = gmtime(&lastWrite);

  // convert struct tm to HTTP-date string
  size_t res = strftime(buff, sizeof(buff), 
    "%a, %d %b %Y %H:%M:%S GMT", gmt_p);
  if (res == 0) {
    Util::Log(EVENT_ERROR, "fail to call strftime");
    return false;
  }

  // add file last write time to 'last_modified' header
  answer_last_modified_.assign(buff);
  answer_cache_control_ = "max-age=86400"; //one-day  

  // validate the cache
  // if_modified_since_ from remote IE7 will have 'length=xxx' append str, just
  // ignore it by only check the substr from beginning.
  if (if_modified_since_.substr(0, answer_last_modified_.length()).compare(
      answer_last_modified_) == 0) {
    // cache is OK, not send content
    return true;
  }

  return false;
}
