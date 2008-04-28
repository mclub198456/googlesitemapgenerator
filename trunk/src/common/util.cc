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


#include "common/util.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#include <Wincrypt.h>
#elif defined(__linux__)
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <grp.h>
#include <unistd.h>
#include "common/apacheconfig.h"
#include "third_party/md5/md5.h"
#endif

#include <string>

#include "third_party/zlib/zlib.h"
#include "common/version.h"
#include "common/timesupport.h"

// Definition of static variables.
std::string Util::log_file_name_ = "";
int Util::log_level_ = EVENT_IMPORTANT;

std::string Util::application_dir_ = "";


int Util::StrSplit(const std::string& str, char split, 
                   std::vector<std::string>* res) {
	res->clear();
	std::string oriStr = str;

	std::string::size_type pos = oriStr.find_first_of(split);
	while(pos != std::string::npos) {
	  res->push_back(oriStr.substr(0, pos));
	  oriStr = oriStr.substr(pos + 1);
	  pos = oriStr.find_first_of(split);
	}
	res->push_back(oriStr);

	return (int)res->size();
}

// pos == -1 means match from end
bool Util::Match(const std::string& str, int pos, const std::string& substr) {
  size_t len = str.length();
  size_t sublen = substr.length();
  if (len < sublen)
    return false;

  if (pos < 0)
    pos = (int) (len - sublen);

  if ((len - pos >= sublen && str.substr(pos, sublen) == substr))
    return true;
  else
    return false;
}

bool Util::InitFlags() {
  // Try to get application dir.
  application_dir_ = GetApplicationDir();
  if (application_dir_.length() == 0) {
    Util::Log(EVENT_ERROR, "Failed to get application dir.");
    return false;
  }

  // Set log file path.
#ifdef __linux__
  Util::SetLogFileName(std::string("/var/log/google-sitemap-generator.log"));
#else
  std::string exe_path;
  if (!GetModuleDir(NULL, &exe_path)) {
    Util::Log(EVENT_ERROR, "Failed to get application path.");
  }
  exe_path.append("\\google-sitemap-generator.log");
  SetLogFileName(exe_path);
#endif

  // Load application level settings.
  std::string settingspath = SiteSettings::GetDefaultFilePath();
  Util::Log(EVENT_IMPORTANT, "[%s] is used as init global flags.",
                             settingspath.c_str());

  SiteSettings settings;
  if (!settings.LoadFromFile(settingspath.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to load setting file.");
    return false;
  }

  // Set logging level.
  Util::SetLogLevel(settings.logging_level());

#ifdef __linux__
  // Set apache conf root.
  if (settings.apache_conf().length() == 0 &&
      ApacheConfig::GetConfFilePath().length() == 0) {
    Util::Log(EVENT_ERROR, "Apache conf shouldn't be empty.");
    return false;
  }
  if (ApacheConfig::GetConfFilePath().length() == 0) {
    ApacheConfig::SetConfFilePath(settings.apache_conf().c_str());
  }
#endif

  // Print a starting message.
  Util::Log(EVENT_CRITICAL, "== Google Sitemap Generator [%s] ==",
            SITEMAP_VERSION1);

  return true;
}


void Util::SetLogFileName(const std::string& log_file_path) {
  log_file_name_ = log_file_path;
}

std::string Util::EscapeLogMessage(const char* message) {
  std::string escaped;
  while (*message != '\0') {
    int value = static_cast<unsigned char>(*message);
    if (value >= 32 && value <= 127) {
      escaped.push_back(*message);
    } else {
      escaped.push_back('{');
      escaped.push_back(int_to_hex_digit_high(value >> 4));
      escaped.push_back(int_to_hex_digit_high(value));
      escaped.push_back('}');
    }
    ++message;
  }
  return escaped;
}

void Util::Log(int event_type, const char *event_pattern, ...) {
  const int     kErrorLen = 2048;
  char          event_message[kErrorLen];
  va_list argptr;

  // If current event is less than log_level_, return directly.
  if (event_type < log_level_) return;

  va_start(argptr, event_pattern);
  vsnprintf(event_message, kErrorLen, event_pattern, argptr);
  va_end(argptr);

#ifdef WIN32
  // Only write error and critical event to windows event log
  if (event_type >= EVENT_CRITICAL) {
    WORD windows_event_type;
    if (event_type == EVENT_ERROR)
      windows_event_type = EVENTLOG_ERROR_TYPE;
    else
      windows_event_type= EVENTLOG_INFORMATION_TYPE;

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

  // Output logo to file
  if (CreateLogFile()) {
    FILE* file = fopen(log_file_name_.c_str(), "a+");
    if (file != NULL) {
      fputs(::FormatW3CTime(time(NULL)).c_str(), file);
      fputs(": ", file);
      fputs(event_message, file);
      fputs("\n", file);
      fclose(file);
    } else {
#ifdef _DEBUG
      fprintf(stderr, "%s: Can not open log file (%d) to write.\n",
              FormatW3CTime(time(NULL)).c_str(), errno);
#endif
    }
  } else {
#ifdef _DEBUG
    fprintf(stderr, "%s: Can not create log file (%d)\n",
            FormatW3CTime(time(NULL)).c_str(), errno);
#endif
  }

  // Output to console if it's in debugging mode.
#ifdef _DEBUG
  fprintf(stderr, "%s: %s\n",
          FormatW3CTime(time(NULL)).c_str(), event_message);
#else
  // This is for unittest. In production code, this value is never negative.
  if (log_level_ < 0) {
    fprintf(stderr, "%s: %s\n",
            FormatW3CTime(time(NULL)).c_str(), event_message);
  }
#endif
}


bool Util::GZip(const char* src, const char* dest) {
  if (src == NULL || dest == NULL) return false;

  const int kBufferSize = 1024 * 32;

  FILE* srcfile = NULL;
  gzFile destfile = NULL;
  char* buffer = NULL;

  bool result = false;
  while (true) {
    srcfile = fopen(src, "rb");
    if (srcfile == NULL) break;

    destfile = gzopen(dest, "wb");
    if (destfile == NULL) break;

    buffer = new char[kBufferSize];
    if (buffer == NULL) break;

    result = true;
    while (!feof(srcfile)) {
      int count = static_cast<int>(
          fread(buffer, sizeof(buffer[0]), kBufferSize, srcfile));
      if (ferror(srcfile)) {
        result = false;
        break;
      }

      if (count == 0) break;

      if (gzwrite(destfile, buffer, count) == 0) {
        result = false;
        break;
      }
    }
    break;
  }

  if (buffer != NULL) delete[] buffer;
  if (srcfile != NULL) fclose(srcfile);
  if (destfile != NULL) gzclose(destfile);

  return result;
}

#ifdef WIN32

bool Util::GetModuleDir(HMODULE module, std::string* dir) {
  char buffer[1024];
  if (GetModuleFileNameA(module, buffer, 1024) == 0) {
    return false;
  }

  *dir = buffer;
  for (int i = static_cast<int>(dir->length()) - 1; i >= 0; --i) {
    if ((*dir)[i] == '\\' || (*dir)[i] == '/') {
      *dir = dir->substr(0, i);
      break;
    }
  }

  return true;
}

bool Util::GetModulePath(HMODULE module, std::string* path) {
  char buffer[1024];
  if (GetModuleFileNameA(module, buffer, 1024) == 0) {
    return false;
  }

  *path = buffer;
  return true;
}


bool Util::CreateLogFile() {
  // Check whether log file already exists.
  WIN32_FIND_DATAA finddata;
  HANDLE findhandle = FindFirstFileA(log_file_name_.c_str(), &finddata);
  if (findhandle != INVALID_HANDLE_VALUE) {
    FindClose(findhandle);
    return true;
  }

  // Retrieve security identifier for "Everyone".
  char sidbuffer[1024];
  PSID psid = reinterpret_cast<PSID>(sidbuffer);
  DWORD sidbuffer_size = 1024;

  char domainbuffer[1024];
  DWORD domainbuffer_size = 1024;

  SID_NAME_USE snu;
  if (!LookupAccountNameA(NULL, "Everyone",
                          psid, &sidbuffer_size,
                          domainbuffer, &domainbuffer_size,
                          &snu))
     return false;

   // Build a ACL and allow Everyone's full control.
  char aclbuffer[1024];
  PACL pacl = reinterpret_cast<PACL>(aclbuffer);

  if (!InitializeAcl(pacl, 1024, ACL_REVISION))
    return false;

  if (!AddAccessAllowedAce(pacl, ACL_REVISION, GENERIC_ALL, psid))
    return false;

  // Build an SD, and add ACL to it.
  SECURITY_DESCRIPTOR sd;

  if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    return false;

  if (!SetSecurityDescriptorDacl(&sd, TRUE, pacl, FALSE))
    return false;

  // Build an SA, and add SD to it.
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.bInheritHandle = FALSE;
  sa.lpSecurityDescriptor = &sd;

  // Create the file with SA.
  HANDLE file = CreateFileA(log_file_name_.c_str(), GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;

  CloseHandle(file);
  return true;
}

std::string Util::GetApplicationDir() {
  // Check the cached value.
  // Note no locking mechanism is put on application_dir_,
  // because we assume that this value is initialized on starting up stage,
  // and never changed later.
  if (application_dir_ != "") {
    return application_dir_;
  }

  if (!GetModuleDir(NULL, &application_dir_)) {
    application_dir_ = "";
  }

  return application_dir_;
}

bool Util::MD5Encrypt(const char* original, std::string* encrypted) {
  if (original == NULL || encrypted == NULL) {
    return false;
  }

  HCRYPTPROV cryptprov = NULL;
  HCRYPTHASH hash = NULL;

  bool result = false;
  while (true) {
    if(!CryptAcquireContext(&cryptprov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
      Util::Log(EVENT_ERROR, "Failed to acquire crypt context (0x%x).",
                GetLastError());
      break;
    }

    if(!CryptCreateHash(cryptprov, CALG_MD5, 0, 0, &hash)) {
      Util::Log(EVENT_ERROR, "Failed to create hash (0x%x).",
                GetLastError());
      break;
    }

    if (!CryptHashData(hash, reinterpret_cast<BYTE*>(const_cast<char*>(original)),
                       static_cast<DWORD>(strlen(original)), 0)) {
      Util::Log(EVENT_ERROR, "Failed to hash data. (0x%x).",
                GetLastError());
      break;
    }

    // MD5 always has a 128 bit hash value
    BYTE buffer[20];
    DWORD bufferlen = 20;
    if (!CryptGetHashParam(hash, HP_HASHVAL, buffer, &bufferlen, 0)) {
      Util::Log(EVENT_ERROR, "Failed to get hash value (0x%x).",
                GetLastError());
      break;
    }

    encrypted->clear();
    for (int i = 0; i < static_cast<int>(bufferlen); ++i) {
      int val = static_cast<int>(buffer[i]) / 16;
      encrypted->push_back(Util::int_to_hex_digit_low(val));
      val = static_cast<int>(buffer[i]) % 16;
      encrypted->push_back(Util::int_to_hex_digit_low(val));
    }

    result = true;
    break;
  }

  if(hash != NULL) {
    CryptDestroyHash(hash);
  }
  if(cryptprov != NULL) {
    CryptReleaseContext(cryptprov,0);
  }
  return result;
}

bool Util::MultiByteToWideChar(const char* input, std::wstring* output) {
  if (input[0] == '\0') {
    output->clear();
    return true;
  }

  // Covert sys to wide char.
  int input_len = static_cast<int>(strlen(input));
  int size1 = ::MultiByteToWideChar(GetACP(), MB_ERR_INVALID_CHARS,
    input, input_len, NULL, 0);
  if (size1 == 0) return false;

  output->resize(size1);
  int size2 = ::MultiByteToWideChar(GetACP(), MB_ERR_INVALID_CHARS,
    input, input_len,
    const_cast<wchar_t*>(output->c_str()), size1);

  return size1 == size2;
}

bool Util::WideCharToMultiByte(const wchar_t* input, std::string* output) {
  if (input[0] == '\0') {
    output->clear();
    return true;
  }

  // Covert wide char to system default encoding page.
  int input_len = static_cast<int>(wcslen(input));
  int size1 = ::WideCharToMultiByte(GetACP(), 0,
    input, input_len, NULL, 0, NULL, NULL);
  if (size1 == 0) return false;

  output->resize(size1);
  int size2 = ::WideCharToMultiByte(GetACP(), 0,
    input, input_len,
    const_cast<char*>(output->c_str()),
    size1, NULL, NULL);

  return size1 == size2;
}


bool Util::SystemToUTF8(const char* system, std::string* utf8) {
  if (system[0] == '\0') {
    utf8->clear();
    return true;
  }

  // Covert system to wide char first.
  std::wstring wide;
  if (!Util::MultiByteToWideChar(system, &wide)) {
    return false;
  }

  // Covert wide char to UTF8.
  int size1 = ::WideCharToMultiByte(CP_UTF8, 0,
    wide.c_str(), static_cast<int>(wide.length()), NULL, 0, NULL, NULL);
  if (size1 == 0) return false;

  utf8->resize(size1);
  int size2 = ::WideCharToMultiByte(CP_UTF8, 0,
    wide.c_str(), static_cast<int>(wide.length()),
    const_cast<char*>(utf8->c_str()),
    size1, NULL, NULL);
  return size1 == size2;
}

bool Util::UTF8ToSystem(const char* utf8, std::string* system) {
  if (utf8[0] == '\0') {
    system->clear();
    return true;
  }

  // Covert utf8 to wide char.
  std::wstring wide;
  int utf8_len = static_cast<int>(strlen(utf8));
  int size1 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
    utf8, utf8_len, NULL, 0);
  if (size1 == 0) return false;

  wide.resize(size1);
  int size2 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
    utf8, utf8_len,
    const_cast<wchar_t*>(wide.c_str()), size1);
  if (size1 != size2) return false;

  // Convert wide char to system default encoding char.
  return Util::WideCharToMultiByte(wide.c_str(), system);
}

#elif defined(__linux__)

bool Util::RunWithApacheGroup() {
  SiteSettings settings;
  settings.LoadWebserverConfig();
  std::string group_name = settings.webserver_config().group_name();
  if (group_name.length() == 0) {
    Util::Log(EVENT_ERROR, "Failed to retrieve apache group name.");
    return false;
  }

  gid_t group_id;
  group* group_struct = getgrnam(group_name.c_str());
  if (group_struct == NULL) {
    Util::Log(EVENT_ERROR, "Failed retrieve group id for [%s].",
              group_name.c_str());
    return false;
  } else {
    group_id = group_struct->gr_gid;
  }

  // Change e-group-id to apache process.
  if (setegid(group_id) != 0) {
    Util::Log(EVENT_ERROR, "Failed to change group id (%d).", errno);
    return false;
  }

  Util::Log(EVENT_CRITICAL, "Process group is changed to [%s].",
            group_name.c_str());
  return true;
}

bool Util::CreateLogFile() {
  // File already exits.
  struct stat stat_struct;
  if (lstat(log_file_name_.c_str(), &stat_struct) == 0) return true;

  // Get group-id and e-group-id
  if (getgid() == getegid()) {
    return false;
  }

  // Try to create the file.
  int filed = open(log_file_name_.c_str(), O_WRONLY | O_CREAT | O_APPEND,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (filed == -1) {
    fprintf(stderr, "Can't open log file (%s).", log_file_name_.c_str());
    return false;
  }
  close(filed);

  return true;
}

std::string Util::GetApplicationDir() {
  if (application_dir_.length() != 0) {
    return application_dir_;
  }

  char buffer[1024];
  int result = readlink("/proc/self/exe", buffer, 1024);
  if (result < 0 || result >= 1024) {
    return "";
  }
  while (result >= 0 && buffer[result] != '/') --result;
  if (result < 0) return "";

  application_dir_ = std::string(buffer, buffer + result + 1);
  return application_dir_;
}


bool Util::MD5Encrypt(const char* original, std::string* encrypted) {
  unsigned char digest[16];
  md5_state_t state;

  md5_init(&state);
  md5_append(&state, reinterpret_cast<const unsigned char*>(original),
             static_cast<int>(strlen(original)));
  md5_finish(&state, digest);

  encrypted->clear();
  for (int i = 0; i < 16; ++i) {
    int val = static_cast<int>(digest[i]) / 16;
    encrypted->push_back(Util::int_to_hex_digit_low(val));
    val = static_cast<int>(digest[i]) % 16;
    encrypted->push_back(Util::int_to_hex_digit_low(val));
  }
  return true;
}

// We assume linux system default encoding is utf8.
bool Util::SystemToUTF8(const char* system, std::string* utf8) {
  utf8->clear();
  utf8->append(system);
  return true;
}

bool Util::UTF8ToSystem(const char* utf8, std::string* system) {
  system->clear();
  system->append(utf8);
  return true;
}

#endif
