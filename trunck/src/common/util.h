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
#include "common/sitesettings.h"

// Log method which is only compiled into debug version.
#ifdef _DEBUG
#define DLog(level, format, ...) \
  Util::Log(level, "%s %d: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DLog(leve, format, ...)
#endif

// Logging level definition.
#define EVENT_ERROR       100
#define EVENT_CRITICAL    75
#define EVENT_IMPORTANT   50
#define EVENT_NORMAL      25


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

  // All chars except which in the range of [0x32, 0x127] are escaped.
  // This message should be escaped before passed into Log method to avoid
  // malicious corrupting, if the logging message comes from user.
  // The char will be escaped like "{FF}".
  static std::string EscapeLogMessage(const char* message);

  // Set log file name.
  // It should be full file path.
  static void SetLogFileName(const std::string& log_file_path);

  // Set log level.
  // Any message logged less than this level will not be written to file.
  static void SetLogLevel(int log_level) { log_level_ = log_level; }

  // Log message in "printf" format.
  static void Log(int level, const char* message, ...);

  // Gzip "src" file to "dest" file.
  // The "dest" file would be over written if existing.
  static bool GZip(const char* src, const char* dest);

  // Get directory containing executable binary.
  // Note, if this method is called by webserver filter, the directory for
  // webserver main program will be returned.
  static std::string GetApplicationDir();

  // Encrypt text with MD5 algorithm.
  static bool MD5Encrypt(const char* text, std::string* encrypted);

  // Initialize flags for the application,
  // like log-level, log-file-path, and etc. 
  static bool InitFlags();

  static bool Match(const std::string& str, int pos, const std::string& substr);

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

#elif defined(__linux__)
  // Change current process' effective group to apache group.
  static bool RunWithApacheGroup();
#endif

 private:
  // Private constructor to prevent instantiation
  Util() {}

  // Create log file with full access to everyone.
  // Filter also needs read/write this file.
  static bool CreateLogFile();

 private:
  // Cache application directory value.
  static std::string application_dir_;

  // Name of log file.
  static std::string log_file_name_;

  // Log level.
  static int log_level_;
};

#endif  // COMMON_UTIL_H__
