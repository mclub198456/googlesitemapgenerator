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


#include "common/url.h"

#include <string>

#include "common/port.h"
#include "common/charmask.h"
#include "common/hash.h"
#include "common/util.h"

UrlFprint Url::FingerPrint(const char* url) {
#ifdef WIN32
  char copy[kMaxUrlLength];
  strncpy(copy, url, kMaxUrlLength);
  copy[kMaxUrlLength - 1] = '\0';

  // find the question mark
  char* p = copy;
  for (; *p != '\0' && *p != '?'; ++p);
  if (*p == '?') {
    *p = '\0';
    strlwr(copy);
    *p = '?';
  } else {
    strlwr(copy);
  }

  return ::FingerPrint(copy);
#else
  return ::FingerPrint(url);
#endif
}

bool Url::Validate(const char* url) {
  Url instance(url);
  return instance.Validate();
}

Url::Url() {
  Clear();
}

Url::Url(const char* url) {
  Parse(url);
}

void Url::Clear() {
  http_ = true;
  user_.clear();
  password_.clear();
  host_.clear();
  port_ = -1;       // -1 means invalid value
  path_.clear();
  parameter_.clear();

  url_.clear();
  host_url_.clear();
  path_url_.clear();
}

bool Url::Parse(const char* url) {
  Clear();

  url_ = url;
  std::string::size_type pos = 0;

  // Parse the protocol type.
  // Only http and https are supported.
  if (strnicmp(url, "http://", 7) == 0) {
    http_ = true;
    pos += 7;
  } else if (strnicmp(url, "https://", 8) == 0) {
    http_ = false;
    pos += 8;
  } else if (strstr(url, "://") != NULL) {
    return false;
  } else {
    // default protocol is http
    http_ = true;
  }

  // parta contains (user:pwd@host:port)
  // partb contains (path?query)
  std::string::size_type pos1 = url_.find('/', pos);
  std::string parta, partb;
  if (pos1 != std::string::npos) {
    parta = url_.substr(pos, pos1 - pos);
    partb = url_.substr(pos1);
  } else {
    parta = url_.substr(pos);
  }

  // Parse user@password
  pos1 = parta.find('@');
  if (pos1 != std::string::npos) {
    user_ = parta.substr(0, pos1);

    // Find end character of password segment.
    std::string::size_type pos2 = parta.find(':', pos1);
    if (pos2 == std::string::npos)
      return false;

    password_ = parta.substr(pos1 + 1, pos2 - pos1 - 1);
    pos = pos2 + 1;
  } else {
    pos = 0;
  }

  // Parse host
  pos1 = parta.find(':', pos);
  host_ = parta.substr(pos, pos1 - pos);
  
  // Parse port
  if (pos1 != std::string::npos) {
    port_ = atoi(parta.substr(pos1 + 1).c_str());
  } else if (host_.length() != 0) {
    // Default port_ is set only when host_ is not empty.
    if (http_) {
      port_ = 80;
    } else {
      port_ = 443;
    }
  }

  // Parse path and parameter
  if (partb.length() != 0) {
    pos1 = partb.find('?');
    if (pos1 == std::string::npos) {
      path_ = partb;
    } else {
      path_ = partb.substr(0, pos1);
      parameter_ = partb.substr(pos1 + 1);
    }
  }

  // Convert host to lower case.
  strlwr((char *)host_.c_str());

  if (!Validate())
    return false;

  UpdateUrl();
  return true;
}

void Url::UpdateUrl() {
  url_.clear();

  if (!host_.empty()) {
    if (http_)
      url_.append("http://");
    else
      url_.append("https://");

    host_url_ = url_;

    if (!user_.empty() || !password_.empty()) {
      url_.append(user_).append("@").append(password_).append(":");
    }

    url_.append(host_);
    host_url_.append(host_);
  }

  // Append port_ if it's a valid value, and it's not default value or
  // host_ is empty. We check whether host_ is empty because we want to
  // return ":80" like url when host_ is empty.
  if (port_ > 0 && port_ < 65536) {
    if (host_.empty() || (http_ && port_ != 80) || (!http_ && port_ != 443)) {
      char buffer[16];
      itoa(port_, buffer);
      url_.append(":").append(buffer);
      host_url_.append(":").append(buffer);
    }
  }

  url_.append(path_);
  path_url_.append(path_);

  if (!parameter_.empty()) {
    url_.append("?").append(parameter_);
    path_url_.append("?").append(parameter_);
  }
}

bool Url::Validate() const {
  if (host_.empty()) {
    // user_ and password_ should be empty if host_ is empty.
    if (!user_.empty() || !password_.empty())
      return false;

    // Either host or path or port should not be empty.
    if (path_.empty() && (port_ < 0 || port_ >= 65536))
      return false;
  } else {
    // port_ should be between 1 and 65535 if host_ is not empty.
    if (port_ <= 0 || port_ >=65536)
      return false;
  }

  // parameter_ should be empty if path_ is empty.
  if (path_.empty() && !parameter_.empty())
    return false;

  return true;
}


// Only following chars are not reserved.
// "-", "_", ".", "!", "~", "*", "'", "(", ")",
// "0".."9", "A".."Z", "a".."z"
static const CharMask kRFC2396ReservedCharMask(
  0xffffffffL, 0xfc00987dL, 0x78000001L, 0xb8000001L,
  0xffffffffL, 0xffffffffL, 0xffffffffL, 0xffffffffL
);

void Url::EscapeUrlComponent(const char* comp, int len, std::string* dest) {
  for (int i = 0; i < len; ++i) {
      if (kRFC2396ReservedCharMask.contains(comp[i])) {
        dest->push_back('%');
        dest->push_back(Util::int_to_hex_digit_high(comp[i] >> 4));
        dest->push_back(Util::int_to_hex_digit_high(comp[i] & 15));
      } else {
        dest->push_back(comp[i]);
      }
  }
}


static const CharMask kValidUrlCharMask(
  0x0L, 0xaffffffaL, 0x87ffffffL, 0x07fffffeL,
  0x0L, 0x0L, 0x0L, 0x0L
);

bool Url::ValidateUrlChars(const char* src) {
  int srcn = static_cast<int>(strlen(src));
  for (int i = 0; i < srcn; ++i) {
    if (!kValidUrlCharMask.contains(src[i])) {
      return false;
    }
  }

  return true;
}


bool Url::UnescapeUrlComponent(const char* comp, int len, std::string* dest) {
  if (comp == NULL || dest == NULL) return false;

  for (int i = 0; i < len; ++i) {
    if (comp[i] == '%') {
      int high = Util::hex_digit_to_int(comp[++i]);
      if (high == -1) return false;

      int low = Util::hex_digit_to_int(comp[++i]);
      if (low == -1) return false;

      char value = (high << 4) | low;
      dest->push_back(value);
    } else {
      dest->push_back(comp[i]);
    }
  }

  return true;
}







