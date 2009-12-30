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

#include "common/sharedmemory.h"

#include "common/logger.h"
#include "common/util.h"
#include "common/accesscontroller.h"

#ifdef WIN32
SharedMemory::SharedMemory() {
  data_ = NULL;
  name_.assign("");
}

SharedMemory::~SharedMemory() {
  Destroy();
}

void SharedMemory::Destroy() {
  if (data_ == NULL) return;

  if (data_ != NULL) {
    UnmapViewOfFile(data_);
    data_ = NULL;
  }

  if (file_mapping_ != NULL) {
    CloseHandle(file_mapping_);
    file_mapping_ = NULL;
  }

  name_.assign("");
}

bool SharedMemory::Create(const std::string& name, size_t size,
                          bool server, bool* exist) {
  // Try to get handle of shared memory.
  if (!server) {
    file_mapping_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, name.c_str());
  } else {
    // Initialize security attribute.
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    PACL pacl;
    if (!AccessController::BuildIPCSecurityDescriptor(&sa, &sd, &pacl)) {
      Logger::Log(EVENT_ERROR, "Failed to build acl for shared memory.");
      return false;
    }

    file_mapping_ = CreateFileMappingA(
      INVALID_HANDLE_VALUE,
      &sa, PAGE_READWRITE,           // protection
      0, (DWORD) size, name.c_str());

    if (file_mapping_ != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
      *exist = true;
    } else {
      *exist = false;
    }
  }

  if (file_mapping_ != NULL) {
    // Attach the shared memory.
    data_ = MapViewOfFile(file_mapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (data_ == NULL) {
      CloseHandle(file_mapping_);
      file_mapping_ = NULL;
      Logger::Log(EVENT_ERROR, "Failed to map view of file. (%d)",
                GetLastError());
      return false;
    }
  } else {
    Logger::Log(EVENT_ERROR, "Failed to create file mapping. (%d)",
              GetLastError());
    return false;
  }

  name_ = name;
  server_ = server;
  return true;
}

#else // __linux__ || __unix__
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

SharedMemory::SharedMemory() {
  data_ = NULL;
  name_.assign("");
}

SharedMemory::~SharedMemory() {
  Destroy();
}

void SharedMemory::Destroy() {
  if (data_ == NULL || shm_id_ == -1) return;

  if (shmdt(data_) == -1) {
    Logger::Log(EVENT_ERROR, "Failed to detach shm. (%d)", errno);
  }
  data_ = NULL;

  if (server_) {
    if (shmctl(shm_id_, IPC_RMID, 0) == -1) {
      Logger::Log(EVENT_ERROR, "Failed to remove shm. (%d)", errno);
    }
    shm_id_ = -1;

    remove(lock_file_.c_str());
    lock_file_.assign("");
  }
}

bool SharedMemory::Create(const std::string& name, size_t size,
                          bool server, bool* exist) {
  // Contruct lock file name for the shared memory.
  lock_file_.assign(Util::GetApplicationDir());
  lock_file_.append("/run/shm_");
  for (int i = 0; i < static_cast<int>(name.length()); ++i) {
    if (isalnum(name[i])) {
      lock_file_.push_back(static_cast<char>(name[i]));
    } else {
      lock_file_.push_back('_');
    }
  }
  lock_file_.append(".lck");

  // Open and lock the file.
  int lockfd = -1;
  if (server) {
    lockfd = open(lock_file_.c_str(), O_CREAT | O_RDWR, GSG_SHARE_WRITE);
  } else {
    lockfd = open(lock_file_.c_str(), O_RDONLY);
  }
  if (lockfd == -1) {
    Logger::Log(EVENT_ERROR, "Failed to open lock file [%s] (%d).",
              lock_file_.c_str(), errno);
    return false;
  }
  if (flock(lockfd, LOCK_EX) != 0) {
    Logger::Log(EVENT_ERROR, "Failed to lock file [%s] (%d).",
              lock_file_.c_str(), errno);
    return false;
  }

  server_ = server;

  bool result = false;
  do {
    if (server) {
      // Get a new shm if it is server.
      shm_id_ = shmget(IPC_PRIVATE, size,
                       GSG_SHARE_WRITE | IPC_CREAT);
      if (shm_id_ < 0) {
        Logger::Log(EVENT_ERROR, "Failed to get shm id. (%d).", errno);
        break;
      }

      if (write(lockfd, &shm_id_, sizeof(int)) < 0) {
        Logger::Log(EVENT_ERROR, "Failed to write shm id. (%d)", errno);
        break;
      }

      *exist = false;

    } else {
      // Read existing shm if it is not a server.
      if (read(lockfd, &shm_id_, sizeof(int)) < 0) {
        Logger::Log(EVENT_ERROR, "Failed to read shm id. (%d)", errno);
        break;
      }

      *exist = true;
    }

    data_ = shmat(shm_id_, 0, 0);
    if (data_ == reinterpret_cast<void*>(-1)) {
      Logger::Log(EVENT_ERROR, "Failed to attach shm (%d).", errno);
      break;
    }

    result = true;
  } while (false);

  if (result == false) {
    shm_id_ = -1;
    data_ = NULL;
  }

  flock(lockfd, LOCK_UN);
  close(lockfd);

  // Change file permission to allow Apache access.
  if (result && server_) {
    if (!AccessController::AllowApacheAccessFile(lock_file_,
                                                 AccessController::kAllowRead)) {
      Logger::Log(EVENT_ERROR, "Failed to allow Apache allow lock file.");
      return false;
    }
  }

  return result;
}

#endif
