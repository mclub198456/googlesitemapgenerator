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

#include "common/accesscontroller.h"

#ifdef WIN32
#include <windows.h>
#include <Wincrypt.h>
#include <Aclapi.h>
#include "common/iisconfig.h"
#elif defined(__linux__) || defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <errno.h>
#endif

#include "common/logger.h"
#include "common/settingmanager.h"
#include "common/fileutil.h"

#ifdef WIN32

bool AccessController::AddAccountToACL(PACL acl, const std::string& name,
                                       SID_NAME_USE sid_type, DWORD mask) {
  const int kDomainBufferSize = 1024;
  char domain_buffer[kDomainBufferSize], psid[SECURITY_MAX_SID_SIZE + 1];
  DWORD domain_size = kDomainBufferSize, sid_size = SECURITY_MAX_SID_SIZE + 1;
  SID_NAME_USE snu;
  if (!LookupAccountNameA(NULL, name.c_str(),
                          psid, &sid_size,
                          domain_buffer, &domain_size,
                          &snu)) {
    if (GetLastError() == ERROR_NONE_MAPPED) {
      Logger::Log(EVENT_IMPORTANT, "Account [%s] can't be found.",
                  name.c_str());
      return true;
    } else {
      Logger::Log(EVENT_ERROR, "Faild to lookup [%s]. (%d)",
                  name.c_str(), GetLastError());
      return false;
    }
  }

  // Check account type.
  if (snu != sid_type) {
    Logger::Log(EVENT_ERROR, "Account [%s] type [%d] is not expected [%d].",
                name.c_str(), snu, sid_type);
    return false;
  }

  // Add account to acl.
  if (!AddAccessAllowedAceEx(acl, ACL_REVISION,
                             CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                             mask, psid)) {
    Logger::Log(EVENT_ERROR, "Failed to add [%s] to ACL. (%d)",
                name.c_str(), GetLastError());
    return false;
  }
  return true;
}

bool AccessController::AddIISAccountsToACL(PACL acl, DWORD mask) {
  char name_buffer[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD name_size = MAX_COMPUTERNAME_LENGTH + 1;
  if (!GetComputerNameA(name_buffer, &name_size)) {
    Logger::Log(EVENT_ERROR, "Failed to get computer name. (%x)",
                GetLastError());
    return false;
  }

  std::string sid, account;
  int iis_version = IisConfig::GetIisVersion();
  if (iis_version == 6) {
    account.assign(name_buffer);
    account.append("\\IUSR_").append(name_buffer);
    if (!AddAccountToACL(acl, account, SidTypeUser, mask)) {
      return false;
    }

    account.assign(name_buffer);
    account.append("\\IIS_WPG");
    if (!AddAccountToACL(acl, account, SidTypeAlias, mask)) {
      return false;
    }

    // For backward compatibility.
    account.assign(name_buffer);
    account.append("\\ASPNET");
    if (!AddAccountToACL(acl, account, SidTypeUser, mask)) {
      return false;
    }
  } else if (iis_version == 7) {
    account.assign("BUILTIN\\IIS_IUSRS");
    if (!AddAccountToACL(acl, account, SidTypeAlias, mask)) {
      return false;
    }

    account.assign("NT AUTHORITY\\IUSR");
    if (!AddAccountToACL(acl, account, SidTypeWellKnownGroup, mask)) {
      return false;
    }
  } else {
    Logger::Log(EVENT_ERROR, "Invalid IIS version.");
    return false;
  }

  return true;
}

bool AccessController::AllowIISAccessFile(const std::string& file,
                                          DWORD access_mask) {
  return AllowIISAccessFile(file, access_mask, false);
}
bool AccessController::AllowIISAccessFile(const std::string& file,
                                          DWORD access_mask, bool inherited) {
  SECURITY_DESCRIPTOR sd;
  SECURITY_ATTRIBUTES sa;
  PACL pacl;
  if (!CreateSecurityDescriptor(&sa, &sd, &pacl, access_mask, inherited)) {
    Logger::Log(EVENT_ERROR, "Failed to build security descriptor.");
    return false;
  }

  // Set new SD to file.
  if (!SetFileSecurityA(file.c_str(), DACL_SECURITY_INFORMATION, &sd)) {
    Logger::Log(EVENT_ERROR, "Failed to SetFileSecurityA. (%d)",
                GetLastError());
    LocalFree(pacl);
    return false;
  }

  LocalFree(pacl);

  FileAttribute fileAttr;
  if (!FileUtil::GetFileAttribute(file.c_str(), &fileAttr)) {
    Logger::Log(EVENT_ERROR, "Failed to get attribute of [%s].", file.c_str());
    return false;
  }

  if (fileAttr.is_dir) {
    std::vector<std::string> childDirs, childFiles;
    if (!FileUtil::ListDir(file.c_str(), false, &childDirs, &childFiles)) {
      Logger::Log(EVENT_ERROR, "Failed to scan dir [%s].", file.c_str());
      return false;
    }

    for (int i = 0; i < static_cast<int>(childDirs.size()); ++i) {
      if (!AllowIISAccessFile(childDirs[i], access_mask, true)) {
        Logger::Log(EVENT_ERROR, "Failed to change permission of child dir [%s].",
                    childDirs[i].c_str());
        return false;
      }
    }

    for (int i = 0; i < static_cast<int>(childFiles.size()); ++i) {
      if (!AllowIISAccessFile(childFiles[i], access_mask, true)) {
        Logger::Log(EVENT_ERROR, "Failed to change permission of child file [%s].",
                    childFiles[i].c_str());
        return false;
      }
    }
  }
  
  return true;
}

bool AccessController::AllowWebserverAccess(const std::string& file,
                                            int permission) {
  DWORD mask = 0;
  if ((permission & kAllowRead) == kAllowRead) {
    mask |= GENERIC_READ;
  }
  if ((permission & kAllowWrite) == kAllowWrite) {
    mask |= GENERIC_WRITE;
  }

  return AllowIISAccessFile(file, mask);
}

bool AccessController::BuildIPCSecurityDescriptor(SECURITY_ATTRIBUTES* sa,
                                                  SECURITY_DESCRIPTOR* sd,
                                                  PACL* ppacl) {
  return CreateSecurityDescriptor(sa, sd, ppacl, GENERIC_ALL, false);
}

bool AccessController::CreateSecurityDescriptor(SECURITY_ATTRIBUTES* sa,
                                                SECURITY_DESCRIPTOR* sd,
                                                PACL* ppacl,
                                                DWORD iis_mask,
                                                bool inherited) {
  // Initialize security descriptor.
  if (!InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION)) {
    Logger::Log(EVENT_ERROR, "Failed to initialize SD.");
    return false;
  }

  // Retrieve security identifier for OWNER.
  char owner_sid_buffer[SECURITY_MAX_SID_SIZE];
  DWORD owner_sid_size = SECURITY_MAX_SID_SIZE;
  PSID owner_sid = owner_sid_buffer;
  if (!CreateWellKnownSid(WinCreatorOwnerSid, NULL, owner_sid, &owner_sid_size)) {
    Logger::Log(EVENT_ERROR, "Failed to get wellknown sid for owner. (%d)",
                GetLastError());
    return false;
  }

  // Retrieve seucrity identifier for Adminsitrators group.
  char everyone_sid_buffer[SECURITY_MAX_SID_SIZE];
  DWORD everyone_sid_size = SECURITY_MAX_SID_SIZE;
  PSID everyone_sid = everyone_sid_buffer;
  if (!CreateWellKnownSid(WinWorldSid, NULL, everyone_sid, &everyone_sid_size)) {
    Logger::Log(EVENT_ERROR, "Failed to get wellknown sid for everyone. (%d)",
                GetLastError());
    return false;
  }

  // Build an ACL and allow OWNER, Local System, Administrators and IIS' access.
  const int kACLSize = 2048;
  *ppacl = reinterpret_cast<PACL>(LocalAlloc(0, kACLSize));
  if (*ppacl == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to alloc memory for ACL. (%d).",
              GetLastError());
    return false;
  }

  DWORD ace_flags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
  if (inherited) {
    ace_flags |= INHERITED_ACE;
  }

  bool result = false;
  do {
    if (!InitializeAcl(*ppacl, kACLSize, ACL_REVISION)) {
      Logger::Log(EVENT_ERROR, "Failed to initialize ACL. (%d).", GetLastError());
      break;
    }

    // Allow owner to full control.
    if (!AddAccessAllowedAceEx(*ppacl, ACL_REVISION,
                               ace_flags,
                               GENERIC_ALL, owner_sid)) {
      Logger::Log(EVENT_ERROR, "Failed to add permission for owner. (%d)",
                  GetLastError());
      break;
    }

    // Allow Everyone to full control.
    if (!AddAccessAllowedAceEx(*ppacl, ACL_REVISION,
                               ace_flags,
                               GENERIC_ALL, everyone_sid)) {
      Logger::Log(EVENT_ERROR, "Failed to add permission for everyone. (%d)",
                  GetLastError());
      break;
    }

    // Set ACL to the Security Descriptor.
    if (SetSecurityDescriptorDacl(sd, TRUE, *ppacl, FALSE) == FALSE) {
      Logger::Log(EVENT_ERROR, "Failed to set Dacl to Security descriptor. (%d)",
                  GetLastError());
      break;
    }

    result = true;
  } while (false);

  if (result == false) {
    Logger::Log(EVENT_ERROR, "Failed to init security descriptor.");
    LocalFree(*ppacl);
  }

  sa->nLength = sizeof(SECURITY_DESCRIPTOR);
  sa->lpSecurityDescriptor = sd;
  sa->bInheritHandle = FALSE;
  return result;
}


#else
gid_t AccessController::apache_gid_ = -1U;
std::string AccessController::apache_group_ = "";

bool AccessController::AllowApacheAccessFile(const std::string& file,
                                             int permission) {
  if (apache_gid_ == -1U) {
    Logger::Log(EVENT_ERROR, "Apache group id is not determined.");
    return false;
  }

  if (chown(file.c_str(), getuid(), apache_gid_) != 0) {
    Logger::Log(EVENT_ERROR, "Failed to chown of [%s]. (%d)",
                file.c_str(), errno);
    return false;
  }

  mode_t mode = 0;
  if (permission & kAllowRead) {
    mode |= GSG_SHARE_READ;
  }
  if (permission & kAllowWrite) {
    mode |= GSG_SHARE_WRITE;
  }

  if (chmod(file.c_str(), mode)) {
    Logger::Log(EVENT_ERROR, "Failed to chmod of [%s]. (%d)",
                file.c_str(), errno);
    return false;
  }

  return true;
}

bool AccessController::RunWithApacheGroup() {
  // We will only allow access with owner or group.
  // umask(S_IROTH | S_IWOTH | S_IXOTH);
  umask(0);  // free for all

  if (apache_group_.length() == 0) {
    SettingManager* setting_manager = SettingManager::default_instance();
    SiteSettings settings;
    if (!setting_manager->LoadApplicationSetting(&settings)) {
      Logger::Log(EVENT_ERROR, "Failed to load setting.");
      return false;
    }
    apache_group_ = settings.apache_group();
  }

  std::string group_name = apache_group_;
  if (group_name.length() == 0) {
    Logger::Log(EVENT_ERROR, "Failed to retrieve apache group name.");
    return false;
  }

  if (group_name[0] == '#') {
    long long id = atoll(group_name.c_str() + 1);
    apache_gid_ = static_cast<gid_t>(id);
  } else {
    group* group_struct = getgrnam(group_name.c_str());
    if (group_struct == NULL) {
      Logger::Log(EVENT_ERROR, "Failed retrieve group id for [%s].",
                group_name.c_str());
      return false;
    } else {
      apache_gid_ = group_struct->gr_gid;
    }
  }

  // Change e-group-id to apache process.
  if (setgid(apache_gid_) != 0) {
    Logger::Log(EVENT_ERROR, "Failed to change group id to [%u] (%d).",
                apache_gid_, errno);
    return false;
  } else {
    Logger::Log(EVENT_CRITICAL, "Process group is changed to [%s].",
              group_name.c_str());
  }

  // Change owner of log file to let apache module write.
  std::string log_file = Logger::GetLogFileName();
  if (chown(log_file.c_str(), getuid(), apache_gid_) != 0) {
    Logger::Log(EVENT_ERROR, "Failed to chown of log file. (%d)",
                errno);
    return false;
  }

  return true;
}

bool AccessController::AllowWebserverAccess(const std::string& file,
                                            int permission) {
  /*
  mode_t mode = 0;
  if ((permission & kAllowRead) == kAllowRead) {
    mode |= S_IRGRP;
  }
  if ((permission & kAllowWrite) == kAllowWrite) {
    mode |= S_IWGRP;
  }
  */

  return AllowApacheAccessFile(file, permission);
}

#endif
