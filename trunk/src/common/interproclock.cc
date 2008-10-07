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

#include "common/interproclock.h"

#include "common/logger.h"
#include "common/util.h"
#include "common/accesscontroller.h"

InterProcLock::InterProcLock(const std::string& name) {
  Initialize(name, false, false);
}

InterProcLock::InterProcLock(const std::string& name,
                             bool with_webserver, bool is_server) {
  Initialize(name, with_webserver, is_server);
}

void InterProcLock::Initialize(const std::string& name,
                               bool with_webserver, bool is_server) {
  with_webserver_ = with_webserver;
  is_server_ = is_server;
  name_ = name;

  // Construct the lock file path.
#ifdef WIN32
  file_.append(Util::GetApplicationDir());
  file_.append("\\run\\").append(name).append(".lck");
  handle_ = INVALID_HANDLE_VALUE;
#else
  file_.append(Util::GetApplicationDir());
  file_.append("/run/").append(name).append(".lck");
  descriptor_ = -1;
#endif
}

InterProcLock::~InterProcLock() {
  Unlock();
}

#ifdef WIN32
bool InterProcLock::Lock(int wait_ms) {
  if (handle_ != INVALID_HANDLE_VALUE) {
    return true;
  }

  while (wait_ms >= 0) {
    // Try to create the lock file to own the lock.
    handle_ = CreateFileA(file_.c_str(), GENERIC_WRITE,
      0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle_ != INVALID_HANDLE_VALUE) {
      return true;
    }

    wait_ms -= kPollingPeriod;
    if (wait_ms < 0) break;
    Sleep(kPollingPeriod);
  }

  Logger::Log(EVENT_ERROR, "Failed to lock [%s]. (%d)",
            name_.c_str(), GetLastError());
  return false;
}

bool InterProcLock::Unlock() {
  if (handle_ == INVALID_HANDLE_VALUE) {
    return true;
  }

  // Close lock file (release the lock).
  if (::CloseHandle(handle_) == FALSE) {
    Logger::Log(EVENT_ERROR, "Failed to close handle in InterProcLock. (%d)",
              GetLastError());
    return false;
  }
  handle_ = INVALID_HANDLE_VALUE;

  if (with_webserver_) {
    if (is_server_) {
      if (!AccessController::AllowIISAccessFile(file_,
                                                GENERIC_READ | GENERIC_WRITE)) {
        Logger::Log(EVENT_ERROR, "Failed to change file permission in Unlock.");
        return false;
      }
    }
  } else {
    DeleteFileA(file_.c_str());
  }

  return true;
}

#else  // __linux__ || __unix__

#include "common/port.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

bool InterProcLock::Lock(int wait_ms) {
  if (descriptor_ != -1) return true;

  // Open lock file.
  descriptor_ = open(file_.c_str(),
                     O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (descriptor_ == -1) {
    Logger::Log(EVENT_ERROR, "Failed to open lock file [%s] (%d).",
              file_.c_str(), errno);
    return false;
  }

  while (wait_ms >= 0) {
    // Try to obtain the flock.
    if (flock(descriptor_, LOCK_EX | LOCK_NB) == 0) {
      return true;
    }

    wait_ms -= kPollingPeriod;
    if (wait_ms < 0) break;
    Sleep(kPollingPeriod);
  }

  Logger::Log(EVENT_ERROR, "Failed to lock [%s]. (%d)",
            name_.c_str(), errno);
  return false;
}

bool InterProcLock::Unlock() {
  if (descriptor_ == -1) return true;

  // Release the lock.
  if (flock(descriptor_, LOCK_UN) != 0) {
    Logger::Log(EVENT_ERROR, "Failed to unlock (%d)", errno);
    return false;
  } else {
    if (close(descriptor_) != 0) {
      Logger::Log(EVENT_ERROR, "Failed to remove lock file [%s]. (%d)",
                file_.c_str(), errno);
    }
    descriptor_ = -1;

    // Save file and change its permission if it is shared with Apache.
    if (with_webserver_) {
      if (is_server_) {
        if (!AccessController::AllowApacheAccessFile(file_,
                                                     S_IRGRP | S_IWGRP)) {
          Logger::Log(EVENT_ERROR, "Failed to change file mode in Unlock.");
          return false;
        }
      }
    } else {
      remove(file_.c_str());
    }

    return true;
  }
}
#endif
