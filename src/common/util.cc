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


#include "common/util.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#include <wincrypt.h>
#elif defined(__linux__) || defined(__unix__)
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "common/apacheconfig.h"
#include "third_party/md5/md5.h"
#endif

#include <string>

#include "third_party/zlib/zlib.h"
#include "common/version.h"
#include "common/timesupport.h"
#include "common/fileutil.h"
#include "common/accesscontroller.h"
#include "common/logger.h"

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
#if defined(DEBUG) || defined(_DEBUG)
  Logger::SetLogType(EVENT_APPLOG | EVENT_STDERR);
#endif

  // Set application dir.
  std::string app_dir; 
#ifdef WIN32
  if (!GetModuleDir(NULL, &app_dir)) {
    Logger::Log(EVENT_ERROR, "Failed to init application dir.");
    return false;
  }
#else
  app_dir = ReadApplicationDir();
  if (app_dir.length() == 0) {
    Logger::Log(EVENT_ERROR, "Failed to init application dir.");
    return false;
  }
#endif
  SetApplicationDir(app_dir);
 
  // Set log file path.
  std::string log_path = GetApplicationDir();
#ifdef WIN32
  log_path.append("\\google-sitemap-generator.log");
#else
  log_path.append("/log/google-sitemap-generator.log");
#endif
  Logger::SetLogFileName(log_path);

  // Initialize setting manager.
  std::string settingspath = SettingManager::GetDefaultFilePath();
  SettingManager* setting_manager = SettingManager::default_instance();
  setting_manager->Initialize(settingspath);
  Logger::Log(EVENT_IMPORTANT, "[%s] is used as application settings.",
                               settingspath.c_str());

  // Load and set logging level.
  SiteSettings settings;
  if (setting_manager->LoadApplicationSetting(&settings)) {
    Logger::SetLogLevel(settings.logging_level());
  } else {
    Logger::Log(EVENT_CRITICAL, "Skip to load application setting.");
  }

#if defined(__linux__) || defined(__unix__)
  // Set apache conf root.
  if (settings.apache_conf().length() == 0 &&
      ApacheConfig::GetConfFilePath().length() == 0) {
    Logger::Log(EVENT_ERROR, "Apache conf shouldn't be empty.");
    return false;
  }
  if (ApacheConfig::GetConfFilePath().length() == 0) {
    ApacheConfig::SetConfFilePath(settings.apache_conf().c_str());
  }
#endif

  std::string cache_dir(GetApplicationDir());
  cache_dir.append("/cache");
  if (!FileUtil::CreateDir(cache_dir.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to create cache dir.");
    return false;
  }

  std::string run_dir(GetApplicationDir());
  run_dir.append("/run");
  if (!FileUtil::CreateDir(run_dir.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to create run dir.");
    return false;
  }

  // Print a starting message.
  Logger::Log(EVENT_CRITICAL, "== Google Sitemap Generator [%s] ==",
            SITEMAP_VERSION1);

  return true;
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
  return FileUtil::ParentDir(dir);
}

bool Util::GetModulePath(HMODULE module, std::string* path) {
  char buffer[1024];
  if (GetModuleFileNameA(module, buffer, 1024) == 0) {
    return false;
  }

  *path = buffer;
  return true;
}

bool Util::MD5Encrypt(const char* data, int size, std::string* encrypted) {
  if (data == NULL || encrypted == NULL || size <= 0) {
    return false;
  }

  HCRYPTPROV cryptprov = NULL;
  HCRYPTHASH hash = NULL;

  bool result = false;
  while (true) {
    if(!CryptAcquireContext(&cryptprov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
      Logger::Log(EVENT_ERROR, "Failed to acquire crypt context (0x%x).",
                  GetLastError());
      break;
    }

    if(!CryptCreateHash(cryptprov, CALG_MD5, 0, 0, &hash)) {
      Logger::Log(EVENT_ERROR, "Failed to create hash (0x%x).",
                  GetLastError());
      break;
    }

    if (!CryptHashData(hash, reinterpret_cast<BYTE*>(const_cast<char*>(data)),
                       static_cast<DWORD>(size), 0)) {
      Logger::Log(EVENT_ERROR, "Failed to hash data. (0x%x).",
                  GetLastError());
      break;
    }

    // MD5 always has a 128 bit hash value
    BYTE buffer[20];
    DWORD bufferlen = 20;
    if (!CryptGetHashParam(hash, HP_HASHVAL, buffer, &bufferlen, 0)) {
      Logger::Log(EVENT_ERROR, "Failed to get hash value (0x%x).",
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

bool Util::ReplaceEnvironmentVariable(const std::string& src,
                                      std::string *dest) {
  dest->clear();
  std::string::size_type pos = 0;
  while (pos < src.length()) {
    std::string::size_type pos2 = src.find('%', pos);
    if (pos2 == std::string::npos) {
      dest->append(src.c_str() + pos);
      break;
    }

    dest->append(src, pos, pos2 - pos);
    ++pos2;
    pos = src.find('%', pos2);
    if (pos == std::string::npos) {
      Logger::Log(EVENT_ERROR, "%% is not matched [%s].", src.c_str());
      return false;
    }

    std::string varname = src.substr(pos2, pos - pos2);
    char buffer[1024];
    DWORD size = ::GetEnvironmentVariableA(varname.c_str(), buffer, 1024);
    if (size >= 1024) {
      Logger::Log(EVENT_ERROR, "EnvVar [%s] exceeds 1024.", varname.c_str());
      return false;
    } else if (size <= 0) {
      Logger::Log(EVENT_ERROR, "Failed to get EnvVar[%s].", varname.c_str());
      return false;
    } else {
      dest->append(buffer, buffer + size);
      ++pos;
    }
  }
  
  return true;
}

bool Util::GenerateRandom(char* buffer, int size) {
  bool result = false;
  HCRYPTPROV   h_crypt_prov = NULL;
  do {
    //  Acquire a cryptographic provider context handle.
    if(CryptAcquireContext(&h_crypt_prov, NULL, NULL, PROV_RSA_FULL, 0)) {    
      DLog(EVENT_IMPORTANT, "CryptAcquireContext succeeded.");    
    } else {
      Logger::Log(EVENT_ERROR, "Error during CryptAcquireContext. (0x%x)",
                  GetLastError());
      break;
    }

    // Generate a random initialization vector.
    if(CryptGenRandom(h_crypt_prov, size, reinterpret_cast<BYTE*>(buffer))) {
      DLog(EVENT_IMPORTANT, "Random sequence generated.");
    } else {
      Logger::Log(EVENT_ERROR, "Error during CryptGenRandom. (0x%x)",
                  GetLastError());
      break;
    }

    result = true;
  } while(false);

  // Release resource.
  if(h_crypt_prov) {
    if(!(CryptReleaseContext(h_crypt_prov,0)))
      Logger::Log(EVENT_ERROR, "Error during CryptReleaseContext");
  }

  return result;
}

#elif defined(__linux__) || defined(__unix__)
bool Util::GenerateRandom(char* buffer, int size) {
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    Logger::Log(EVENT_ERROR, "open /dev/urandom failed. (%d)", errno);
    return false;
  }

  int ret = read(fd, buffer, size);
  if (ret < size) {
    Logger::Log(EVENT_ERROR, "Read /dev/urandom failed. (%d)", errno);
    close(fd);
    return false;
  }

  close(fd);
  return true;
}

std::string Util::ReadApplicationDir() {
  if (application_dir_.length() != 0) {
    return application_dir_;
  }

  char buffer[1024];
  FILE* file = fopen("/etc/google-sitemap-generator/home", "r");
  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open gsg home conf file. (%d)", errno);
    return "";
  }
  if (fgets(buffer, 1024, file) == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to read gsg home conf file. (%d)", errno);
    return "";
  }

  application_dir_.assign(buffer);
  std::string::size_type pos;
  while ((pos = application_dir_.find_first_of("\n\r")) != std::string::npos) {
    application_dir_.erase(application_dir_.begin() + pos,
                           application_dir_.end());
  }
  return application_dir_;
}

bool Util::MD5Encrypt(const char* data, int size, std::string* encrypted) {
  unsigned char digest[16];
  md5_state_t state;

  md5_init(&state);
  md5_append(&state, reinterpret_cast<const unsigned char*>(data), size);
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

bool Util::ConvertToSystemEncoding(std::vector<std::string> &strs) {
  for (int i = 0; i < static_cast<int>(strs.size()); ++i) {
    std::string result;
    if (!Util::UTF8ToSystem(strs[i].c_str(), &result)) {
      Logger::Log(EVENT_ERROR, "Failed to convert [%s] to system encoding.",
                  strs[i].c_str());
      return false;
    }
    strs[i] = result;
  }
  return true;
}

bool Util::ConvertToUTF8(std::vector<std::string> &strs) {
  for (int i = 0; i < static_cast<int>(strs.size()); ++i) {
    std::string result;
    if (!Util::SystemToUTF8(strs[i].c_str(), &result)) {
      Logger::Log(EVENT_ERROR, "Failed to convert [%s] to utf-8 encoding.",
                  strs[i].c_str());
      return false;
    }
    strs[i] = result;
  }
  return true;
}
