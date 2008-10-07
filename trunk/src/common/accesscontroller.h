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

// AccessController provides code to control access to system resources, like
// files and IPC objects.

#ifndef COMMON_ACCESSCONTROLLER_H__
#define COMMON_ACCESSCONTROLLER_H__

#include <string>

class AccessController {
 public:

  // Permission flags.
  static const int kAllowRead = 1;
  static const int kAllowWrite = 2;

  // Allow web server to access a file.
  // "permission" value should be combination of kAllowRead and kAllowWrite.
  static bool AllowWebserverAccess(const std::string& file, int permission);

#ifdef WIN32
  // Allow IIS to access a file.
  // "masks" could be common Windows access control mask value.
  static bool AllowIISAccessFile(const std::string& file, DWORD masks);

  // Build security descriptor for IPC objects.
  // The security descriptor will allow both owner and IIS to have full control
  // of IPC objects.
  // Caller should LocalFree pacl if operation succeeds.
  static bool BuildIPCSecurityDescriptor(SECURITY_ATTRIBUTES* sa,
                                         SECURITY_DESCRIPTOR* sd,
                                         PACL* pacl);

#else
  // Allow Apache access a file.
  // "mode" should be combination of S_IRGRP, S_IWGRP and S_IXGRP.
  static bool AllowApacheAccessFile(const std::string& file, mode_t mode);

  // Change current process' effective group to apache group.
  static bool RunWithApacheGroup();

#endif

 private:
#ifdef WIN32
  // Add accout information to given ACL.
  // Permission for the account is specified by "mask" value.
  static bool AddAccountToACL(PACL acl, const std::string& name,
                              SID_NAME_USE sid_type, DWORD mask);

  // Add IIS acounts information for ACL.
  static bool AddIISAccountsToACL(PACL acl, DWORD mask);

  // Create security descriptor.
  // As in the result security descriptor, Owner should have full control,
  // while IIS has control of "iis_mask".
  static bool CreateSecurityDescriptor(SECURITY_ATTRIBUTES* sa,
                                       SECURITY_DESCRIPTOR* sd,
                                       PACL* pacl, DWORD iis_mask);
#else
  // Apache group id.
  static gid_t apache_gid_;
#endif

  AccessController() {}
};

#endif // COMMON_ACCESSCONTROLLER_H__
