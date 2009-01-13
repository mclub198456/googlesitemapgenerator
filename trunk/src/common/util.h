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


// This class defines some utility methods which may be common in the project.

#ifndef COMMON_UTIL_H__
#define COMMON_UTIL_H__

#include <stdio.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <string>
#include "common/basictypes.h"
#include "common/settingmanager.h"

class Util {
 public:
  // Convert a hex digit to integer.
  // -1 is returned for invalid digit.
  inline static int hex_digit_to_int(char c) {
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= '0' && c <= '9') return c - '0';
    return -1;
  }

  // Convert an integer to hex digit.
  // Only the lowest 4 bits take effect.
  inline static char int_to_hex_digit_low(int i) {
    i &= 15;
    return static_cast<char>(i < 10 ? i + '0' : i - 10 + 'a');
  }
  inline static char int_to_hex_digit_high(int i) {
    i &= 15;
    return static_cast<char>(i < 10 ? i + '0' : i - 10 + 'A');
  }

  // Character encoding conversion between system default encoding and UTF8.
  static bool SystemToUTF8(const char* system, std::string* utf8);
  static bool UTF8ToSystem(const char* utf8, std::string* system);

  // Convert encoding of a string array.
  static bool ConvertToUTF8(std::vector<std::string>& strs);
  static bool ConvertToSystemEncoding(std::vector<std::string>& strs);

  // Gzip "src" file to "dest" file.
  // The "dest" file would be over written if existing.
  static bool GZip(const char* src, const char* dest);

  // Get and set application dir.
  // Application dir must be set properly before using it.
  static std::string GetApplicationDir() {
    return application_dir_; 
  }
  static void SetApplicationDir(const std::string& dir) {
    application_dir_ = dir;
  }

  // Encrypt text with MD5 algorithm.
  static bool MD5Encrypt(const char* text, std::string* encrypted) {
    return MD5Encrypt(text, static_cast<int>(strlen(text)), encrypted);
  }
  static bool MD5Encrypt(const char* text, int size, std::string* encrypted);

  // Generate random bytes sequence and store them into buffer.
  // Invoker should ensure the "buffer" has at least "size" capacity.
  static bool GenerateRandom(char* buffer, int size);

  // Initialize flags for the application,
  // like log-level, log-file-path, and etc. 
  static bool InitFlags();  

  // Returns true if the "substr" is found at the "pos" of the "str"
  static bool Match(const std::string& str, int pos, const std::string& substr);

  // Splits the string by the 'split' char, returns the size of the result
  typedef std::vector<std::string> StringVector;
  static int StrSplit(const std::string& str, char split, StringVector* res);

#ifdef WIN32
  // This is a simple version on the name-alike system calls.
  // It should be only used in Windows, as we never use wide API of linux.
  // If you are sure the input is correct, bstr_t could be used for conversion,
  // which throws exception if input is malformed.
  static bool MultiByteToWideChar(const char* input, std::wstring* output);
  static bool WideCharToMultiByte(const wchar_t* input, std::string* output);

  // Get module's physical directory or path.
  // If given module is NULL, the current process's exe module will be used.
  static bool GetModuleDir(HMODULE module, std::string* dir);
  static bool GetModulePath(HMODULE module, std::string* path);

  // Replace environment variables in given string.
  // An environment variable is enclosed by %%.
  static bool ReplaceEnvironmentVariable(const std::string &src,
                                         std::string* str);

#elif defined(__linux__) || defined(__unix__)
  static std::string ReadApplicationDir();

#endif

 private:
  // Private constructor to prevent instantiation
  Util() {}

  // Cache application directory value.
  static std::string application_dir_;

};

#endif  // COMMON_UTIL_H__
