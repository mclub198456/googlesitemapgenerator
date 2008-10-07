#include "common/messageconverter.h"

#include "common/logger.h"

bool MessageConverter::StringToMap(const std::string& str,
                                   MessageConverter::StringMap* strmap) {
  int len = static_cast<int>(str.length());
  for (int i = 0; i < len;) {
    // Find ':'.
    int j = i;
    for (; j < len; ++j) {
      // Skip escaped character.
      if (str[j] == '\\') {
        ++j;
        continue;
      }

      if (str[j] == ':') break;
    }
    if (j >= len) {
      Logger::Log(EVENT_ERROR, "Failed to find \':\' in [%s].", str.c_str() + i);
      return false;
    }

    // Find ','.
    int k = j + 1;
    for (; k < len; ++k) {
      // Skip escaped character.
      if (str[k] == '\\') {
        ++k;
        continue;
      }

      if (str[k] == ',') break;
    }
    if (k > len) {
      Logger::Log(EVENT_ERROR, "Unexpected mapstr end. [%s].", str.c_str() + j);
      return false;
    }

    std::string key, value;
    if (!DecodeString(str.substr(i, j - i), &key)) return false;
    ++j;
    if (!DecodeString(str.substr(j, k - j), &value)) return false;
    strmap->insert(make_pair(key, value));

    i = k + 1;
  }

  return true;
}

void MessageConverter::MapToString(const MessageConverter::StringMap& strmap,
                                   std::string* str) {
  StringMap::const_iterator itr = strmap.begin();
  for (; itr != strmap.end(); ++itr) {
    if (itr != strmap.begin()) str->push_back(',');

    EncodeString(itr->first, str);
    str->push_back(':');
    EncodeString(itr->second, str);
  }
}

void MessageConverter::EncodeString(const std::string& str,
                                    std::string* result) {
  for (int i = 0, len = static_cast<int>(str.length()); i < len; ++i) {
    // Escape special characters.
    if (str[i] == ',' || str[i] == '\\' || str[i] == ':') {
      result->push_back('\\');
    }
    result->push_back(str[i]);
  }
}

bool MessageConverter::DecodeString(const std::string& str,
                                    std::string* result) {
  for (int i = 0, len = static_cast<int>(str.length()); i < len; ++i) {
    if (str[i] == '\\') {
      ++i;
      // Unescape special characters.
      if (i == len || (str[i] != ',' && str[i] != ':' && str[i] != '\\')) {
        Logger::Log(EVENT_ERROR, "Failed to decode [%s].", str.c_str());
        return false;
      }
    }
    result->push_back(str[i]);
  }
  return true;
}


