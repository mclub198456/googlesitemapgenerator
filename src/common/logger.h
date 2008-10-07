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

#ifndef COMMON_LOGGER_H__
#define COMMON_LOGGER_H__

#include <string>

// Log method which is only compiled into debug version.
#ifdef _DEBUG
#define DLog(level, format, ...) \
  Logger::Log(level, "%s %d: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DLog(leve, format, ...)
#endif

// Logging level definition.
#define EVENT_ERROR       100
#define EVENT_CRITICAL    75
#define EVENT_IMPORTANT   50
#define EVENT_NORMAL      25
#define EVENT_LEVEL_MASK  0xFFFF

#define EVENT_APPLOG      (1 << 17)
#define EVENT_SYSLOG      (1 << 18)
#define EVENT_STDOUT      (1 << 19)
#define EVENT_STDERR      (1 << 20)


class Logger {
 public:
  // Set log file name.
  // It should be full file path.
  static void SetLogFileName(const std::string& log_file_path);
  static const std::string& GetLogFileName() {
    return log_file_name_;
  }

  // Set log param.
  // Any message logged less than this level will not be written to file.
  // Message will be written to device included in log_type.
  static void SetLogLevel(int log_level);
  static void SetLogType(int log_type);

  // Log message in "printf" format.
  static void Log(int meta, const char* message, ...);

  // All chars except which in the range of [0x32, 0x127] are escaped.
  // This message should be escaped before passed into Log method to avoid
  // malicious corrupting, if the logging message comes from user.
  // The char will be escaped like "{FF}".
  static std::string EscapeLogMessage(const char* message);


 private: 
  // Create log file with full access to everyone.
  // Filter also needs read/write this file.
  static bool CreateLogFile();

  // Name of log file.
  static std::string log_file_name_;

  // Log level.
  static int log_level_;

  // Log type. Defines output destination of log message.
  static int log_type_;
};

#endif // COMMON_LOGGER_H__
