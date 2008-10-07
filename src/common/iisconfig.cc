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


#include "common/iisconfig.h"
#include "common/logger.h"
#include "common/util.h"
#include "common/iis6config.h"
#include "common/iis7config.h"

// Initialize static variables.
int IisConfig::iis_version_ = -1;
CriticalSection IisConfig::critical_section_;
const wchar_t* IisConfig::kAdminConsoleBinding = L":8181:";

bool IisConfig::Load() {
  int version_copy = GetIisVersion();

  // Load IIS configuration according to iis_version.
  IisConfig* iisconfig = NULL;
  if (version_copy == 6) {
    iisconfig = new Iis6Config();
  } else if (version_copy == 7) {
    iisconfig = new Iis7Config();
  } else {
    Logger::Log(EVENT_ERROR, "Failed to recognize IIS version.");
  }

  // Save configurations to this class's variable.
  bool result = false;
  if (iisconfig != NULL) {
    if (iisconfig->Load()) {
      host_urls_ = iisconfig->host_urls();
      site_ids_ = iisconfig->site_ids();
      names_ = iisconfig->names();
      physical_paths_ = iisconfig->physical_paths();
      log_paths_ = iisconfig->log_paths();
      result = true;
    }
    delete iisconfig;
  }

  return result;
}

bool IisConfig::InstallFilter() {
  int iis_version = GetIisVersion();

  // Construct plugin DLL name according to iis version and app running mode.
  std::string plugin_name(Util::GetApplicationDir());
  plugin_name.append("\\");
  if (iis_version == 7) {
    plugin_name.append("IIS7_Module");
  } else if (iis_version == 6) {
    plugin_name.append("IIS6_Filter");
  } else {
    Logger::Log(EVENT_ERROR, "IIS Version can't be determined.(%d)", iis_version);
    return false;
  }

  AppMode appmode = GetAppMode();
  if (appmode == IisConfig::MODE_X86) {
    plugin_name.append("_Win32");
  } else if (appmode == IisConfig::MODE_X64) {
    plugin_name.append("_x64");
  } else {
    Logger::Log(EVENT_ERROR, "Invalid IIS running mode. (%d)", appmode);
    return false;
  }

  // Convert the name to wide string.
  plugin_name.append(".dll");
  std::wstring wide_path;
  if (!Util::MultiByteToWideChar(plugin_name.c_str(), &wide_path)) {
    Logger::Log(EVENT_ERROR, "Failed to convert [%s] to wide string.",
      plugin_name.c_str());
    return false;
  }

  // Install filter according to iis version.
  if (iis_version == 6) {
    return Iis6Config::InstallFilter(wide_path.c_str());
  } else if (iis_version == 7) {
    return Iis7Config::InstallFilter(wide_path.c_str());
  } else {
    return false;
  }
}

bool IisConfig::UninstallFilter() {
  int version_copy = GetIisVersion();

  if (version_copy == 6) {
    return Iis6Config::UninstallFilter();
  } else if (version_copy == 7) {
    return Iis7Config::UninstallFilter();
  } else {
    Logger::Log(EVENT_ERROR, "Failed to InstallFilter because of IIS version.");
    return false;
  }
}

bool IisConfig::InstallAdminConsole() {
  Logger::Log(EVENT_CRITICAL, "Install Admin Console...");

  // Contruct site root path.
  std::string site_root(Util::GetApplicationDir());
  site_root.append("admin-console");

  std::string cgi_path(site_root);
  int version_copy = GetIisVersion();
  if (version_copy == 6) {
    cgi_path.append("\\cgi-bin\\admin-console.cgi");
  } else if (version_copy == 7) {
    cgi_path.append("\\cgi-bin\\admin-console.exe");
  } else {
    Logger::Log(EVENT_ERROR, "Unknown iis version [%d].", version_copy);
  }

  // Rename the CGI exe file.
  std::string cgi_exe(site_root);
  cgi_exe.append("\\cgi-bin\\AdminConsoleCGI.exe");
  if (!MoveFileExA(cgi_exe.c_str(), cgi_path.c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    Logger::Log(EVENT_ERROR, "Failed to rename cgi file. (%d)", GetLastError());
    return false;
  }

  // Special handling for IIS 7.
  if (version_copy == 7) {
    std::string dummy_file(site_root);
    dummy_file.append("\\cgi-bin\\admin-console.cgi");
    if (!CopyFileA(cgi_path.c_str(), dummy_file.c_str(), false)) {
      Logger::Log(EVENT_ERROR, "Failed to create a dummy file for cgi.");
      return false;
    }
  }

  // Install AdminConsole according to IIS version.
  int iis_version = GetIisVersion();
  if (iis_version == 7) {
    return Iis7Config::InstallAdminConsole(site_root.c_str(), cgi_path.c_str());
  } else if (iis_version == 6) {
    return Iis6Config::InstallAdminConsole(site_root.c_str(), cgi_path.c_str());
  } else {
    Logger::Log(EVENT_ERROR, "IIS Version can't be determined.(%d)", iis_version);
    return false;
  }
}

bool IisConfig::UninstallAdminConsole() {
  Logger::Log(EVENT_CRITICAL, "Install Admin Console...");

  // Contruct site root path.
  std::string site_root(Util::GetApplicationDir());
  site_root.append("admin-console");

  std::string cgi_path(site_root);
  cgi_path.append("\\cgi-bin\\admin-console.cgi");

  int iis_version = GetIisVersion();
  if (iis_version == 7) {
    return Iis7Config::UninstallAdminConsole(site_root.c_str(), cgi_path.c_str());
  } else if (iis_version == 6) {
    return Iis6Config::UninstallAdminConsole(site_root.c_str(), cgi_path.c_str());
  } else {
    Logger::Log(EVENT_ERROR, "IIS Version can't be determined.(%d)", iis_version);
    return false;
  }
}

int IisConfig::GetIisVersion() {
  critical_section_.Enter(true);

  if (iis_version_ == -1) {
    if (Iis7Config::IsSupported()) {
      iis_version_ = 7;
    } else if (Iis6Config::IsSupported()) {
      iis_version_ = 6;
    } else {
      iis_version_ = 0;
    }
  }

  critical_section_.Leave();

  return iis_version_;
}

int IisConfig::GetMajorVersinFromReg() {
  HKEY parameters;
  LONG result = RegOpenKeyA(HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters", &parameters);
  if (result != ERROR_SUCCESS) {
    Logger::Log(EVENT_ERROR, "Open iis reg key failed. (0x%x)", result);
    return -1;
  }

  DWORD version;
  DWORD size = sizeof(DWORD);
  VALENTA val_ent;
  val_ent.ve_valuename = "MajorVersion";
  result = RegQueryMultipleValuesA(parameters, &val_ent, 1,
                                   reinterpret_cast<LPSTR>(&version), &size);
  if (result != ERROR_SUCCESS) {
    Logger::Log(EVENT_ERROR, "Read iis reg value failed. (0x%x)", result);
    return -1;
  }

  return static_cast<int>(version);
}

IisConfig::AppMode IisConfig::GetAppMode() {
  int version = GetIisVersion();
  AppMode mode = MODE_FAIL;
  if (version == 7) {
    mode = Iis7Config::GetAppMode();
  } else if (version == 6) {
    mode = Iis6Config::GetAppMode();
  }

  Logger::Log(EVENT_CRITICAL, "Iis is running under [%d] mode.", mode);
  return mode;
}

int IisConfig::GetNativeSystemInfo(SYSTEM_INFO* system_info) {
  HMODULE module = GetModuleHandle(L"Kernel32.dll");
  if (module == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to GetModuleHandle of Kernel32.dll. (%d)",
              GetLastError());
    return -1;
  }

  typedef void (WINAPI *Procedure)(LPSYSTEM_INFO);
  Procedure proc = (Procedure) GetProcAddress(module, "GetNativeSystemInfo");
  if (proc == NULL) {
    if (GetLastError() == ERROR_PROC_NOT_FOUND) {
      return 0;
    } else {
      Logger::Log(EVENT_ERROR, "Failed to GetProcAddress (%d).", GetLastError());
      return -1;
    }
    return 0;
  } else {
    proc(system_info);
    return 1;
  }
}

