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

#include "common/logger.h"

#ifdef WIN32
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#endif

#include "common/util.h"
#include "common/timesupport.h"
#include "common/accesscontroller.h"

// Definition of static variables.
std::string Logger::log_file_name_ = "";
int Logger::log_level_ = EVENT_IMPORTANT;
int Logger::log_type_ = EVENT_APPLOG;

void Logger::SetLogFileName(const std::string& log_file_path) {
  log_file_name_ = log_file_path;
}

void Logger::SetLogLevel(int log_level) {
  log_level_ = log_level;
}

void Logger::SetLogType(int log_type) {
  log_type_ = log_type;
}

std::string Logger::EscapeLogMessage(const char* message) {
  std::string escaped;
  while (*message != '\0') {
    int value = static_cast<unsigned char>(*message);
    if (value >= 32 && value <= 127) {
      escaped.push_back(*message);
    } else {
      escaped.push_back('{');
      escaped.push_back(Util::int_to_hex_digit_high(value >> 4));
      escaped.push_back(Util::int_to_hex_digit_high(value));
      escaped.push_back('}');
    }
    ++message;
  }
  return escaped;
}

void Logger::Log(int event_meta, const char *event_pattern, ...) {
  const int     kErrorLen = 2048;
  char          event_message[kErrorLen];
  va_list argptr;

  // If current event is less than log_level_, return directly.
  int event_level = event_meta & EVENT_LEVEL_MASK;
  if (event_level < log_level_) return;

  // Combile log type with default log type.
  int event_type = event_meta | log_type_;

  va_start(argptr, event_pattern);
  vsnprintf(event_message, kErrorLen, event_pattern, argptr);
  va_end(argptr);

#ifdef WIN32
  // Only write error and critical event to windows event log
  if ((event_type & EVENT_SYSLOG) != 0 && event_level >= EVENT_CRITICAL) {
    WORD windows_event_type;
    if (event_level == EVENT_ERROR) {
      windows_event_type = EVENTLOG_ERROR_TYPE;
    } else {  //  if (event_level == EVENT_CRITICAL)
      windows_event_type= EVENTLOG_INFORMATION_TYPE;
    }

    LPCSTR event_message_pointer = event_message;

    // Get a handle to the event log.
    HANDLE h = RegisterEventSource(NULL, L"Google Sitemap");
    if (h != NULL) {
      // Report the event.
      ReportEventA(h,                       // event log handle
                   windows_event_type,      // event type
                   0,                       // event category
                   1,                       // event identifier
                   NULL,                    // no user security identifier
                   1,                       // number of strings
                   0,                       // no data
                   &event_message_pointer,  // pointer to strings
                   NULL);                   // no data
      DeregisterEventSource(h);
    }
  }
#endif  // WIN32

  // Output log to file
  if ((event_type & EVENT_APPLOG) != 0) {
    if (CreateLogFile()) {
      FILE* file = fopen(log_file_name_.c_str(), "a+");
      if (file != NULL) {
        fputs(FormatW3CTime(time(NULL)).c_str(), file);
        fputs(": ", file);
        fputs(event_message, file);
        fputs("\n", file);
        fclose(file);
      } else if ((event_type & EVENT_STDERR) != 0) {
        fprintf(stderr, "%s: Failed to open log file (%d).\n",
                FormatW3CTime(time(NULL)).c_str(), errno);
      }
    } else if ((event_type & EVENT_STDERR) != 0) {
      fprintf(stderr, "%s: Failed to create log file (%d)\n",
              FormatW3CTime(time(NULL)).c_str(), errno);
    }
  }

  // Output to std err.
  if ((event_type & EVENT_STDERR) != 0) {
    fprintf(stderr, "%s: %s\n",
            FormatW3CTime(time(NULL)).c_str(), event_message);
  }
}

#ifdef WIN32
bool Logger::CreateLogFile() {
  // Check whether log file already exists.
  WIN32_FIND_DATAA finddata;
  HANDLE findhandle = FindFirstFileA(log_file_name_.c_str(), &finddata);
  if (findhandle != INVALID_HANDLE_VALUE) {
    FindClose(findhandle);
    return true;
  }

  // Create the file with SA.
  HANDLE file = CreateFileA(log_file_name_.c_str(), GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;

  CloseHandle(file);

  if (!AccessController::AllowIISAccessFile(log_file_name_.c_str(),
                                            GENERIC_READ | GENERIC_WRITE)) {
    Logger::Log(EVENT_ERROR, "Failed to change permission for log file.");
  }
  return true;
}

#else
bool Logger::CreateLogFile() {
  // File already exits.
  struct stat stat_struct;
  if (lstat(log_file_name_.c_str(), &stat_struct) == 0) return true;

  // Only root can create log file.
  if (getuid() != 0) {
    return false;
  }

  // Try to create the file.
  int filed = open(log_file_name_.c_str(), O_WRONLY | O_CREAT | O_APPEND,
                   GSG_SHARE_WRITE);
  if (filed == -1) {
    fprintf(stderr, "Can't open log file (%s).", log_file_name_.c_str());
    return false;
  }
  close(filed);

  return true;
}

#endif
