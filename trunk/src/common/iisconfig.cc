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
#include "common/util.h"
#include "common/iis6config.h"
#include "common/iis7config.h"

// Initialize static variables.
int IisConfig::iis_version_ = -1;
CriticalSection IisConfig::critical_section_;

bool IisConfig::Load() {
  int version_copy = GetIisVersion();

  // Load IIS configuration according to iis_version.
  IisConfig* iisconfig = NULL;
  if (version_copy == 6) {
    iisconfig = new Iis6Config();
  } else if (version_copy == 7) {
    iisconfig = new Iis7Config();
  } else {
    Util::Log(EVENT_ERROR, "Failed to recognize IIS version.");
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
    Util::Log(EVENT_ERROR, "IIS Version can't be determined.(%d)", iis_version);
    return false;
  }

  AppMode appmode = GetAppMode();
  if (appmode == IisConfig::MODE_X86) {
    plugin_name.append("_Win32");
  } else if (appmode == IisConfig::MODE_X64) {
    plugin_name.append("_x64");
  } else {
    Util::Log(EVENT_ERROR, "Invalid IIS running mode. (%d)", appmode);
    return false;
  }

  // Convert the name to wide string.
  plugin_name.append(".dll");
  std::wstring wide_path;
  if (!Util::MultiByteToWideChar(plugin_name.c_str(), &wide_path)) {
    Util::Log(EVENT_ERROR, "Failed to convert [%s] to wide string.",
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
    Util::Log(EVENT_ERROR, "Failed to InstallFilter because of IIS version.");
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

IisConfig::AppMode IisConfig::GetAppMode() {
  int version = GetIisVersion();
  AppMode mode = MODE_FAIL;
  if (version == 7) {
    mode = Iis7Config::GetAppMode();
  } else if (version == 6) {
    mode = Iis6Config::GetAppMode();
  }

  Util::Log(EVENT_CRITICAL, "Iis is running under [%d] mode.", mode);
  return mode;
}

