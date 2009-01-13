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


#include "common/mutexset.h"

#include "common/util.h"
#include "common/logger.h"
#include "common/accesscontroller.h"
#include "common/urlpipe.h"

int MutexSet::RegisterMutex(const std::string& name) {
  if (mutexes_.find(name) != mutexes_.end()) {
    return mutexes_[name];
  } else {
    int num = static_cast<int>(mutexes_.size());
    mutexes_[name] = num;
    return num;
  }
}

#ifdef WIN32

#include <windows.h>
#include <Aclapi.h>

MutexSet::MutexSet() {
  // does nothing.
}

MutexSet::~MutexSet() {
  Destroy();
}

bool MutexSet::Initialize(const std::string& name, bool server) {
  server_ = server;
  return true;
}

bool MutexSet::Load() {
  // does nothing.
  return true;
}

// Close all event handles.
void MutexSet::Destroy() {
  std::map<std::string, HANDLE>::iterator itr = events_.begin();
  for (; itr != events_.end(); ++itr) {
    CloseHandle(itr->second);
  }
  events_.clear();
}

// Use CreateEvent to implement this method.
bool MutexSet::GetMutex(const std::string& name, bool init_value,
                        bool allow_multi_post, Mutex* mutex) {
  if (mutexes_.find(name) == mutexes_.end()) {
    Logger::Log(EVENT_ERROR, "unregistered mutex name: [%s].", name.c_str());
    return false;
  }

  // try to retrieve events.
  if (events_.find(name) != events_.end()) {
    mutex->Initialize(allow_multi_post, events_[name]);
    return true;
  }

  // Try to open or create it.
  HANDLE event = NULL;
  if (!server_) {
    event = OpenEventA(EVENT_ALL_ACCESS, false, name.c_str());
  } else {
    // Initialize security attribute.
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
    PACL pacl;
    if (!AccessController::BuildIPCSecurityDescriptor(&sa, &sd, &pacl)) {
      Logger::Log(EVENT_ERROR, "Failed to build acl for events set.");
      return false;
    }

    event = CreateEventA(&sa, false, init_value, name.c_str());

    LocalFree(pacl);
  }

  if (event == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open/create [%s] event. (%d)",
              name.c_str(), GetLastError());
    return false;
  }

  events_[name] = event;
  mutex->Initialize(allow_multi_post, event);
  return true;
}

#else
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __linux__
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
#endif

MutexSet::MutexSet() {
  sem_id_ = -1;
}

MutexSet::~MutexSet() {
  Destroy();
}

bool MutexSet::Initialize(const std::string& name, bool server) {
  // Contruct lock file name for the semaphore.
  lock_file_.assign(Util::GetApplicationDir());
  lock_file_.append("/run/sem_");
  for (int i = 0; i < static_cast<int>(name.length()); ++i) {
    if (isalnum(name[i])) {
      lock_file_.push_back(static_cast<char>(name[i]));
    } else {
      lock_file_.push_back('_');
    }
  }
  lock_file_.append(".lck");

  server_ = server;
  return true;
}

bool MutexSet::Load() {
  // Open and lock the file.
  int lockfd = -1;
  if (server_) {
    lockfd = open(lock_file_.c_str(), O_CREAT | O_RDWR,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
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

  bool result = false;
  do {
    if (server_) {
      // Create a mutex.
      sem_id_ = semget(IPC_PRIVATE, static_cast<int>(mutexes_.size()),
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | IPC_CREAT);
      if (sem_id_ < 0) {
        Logger::Log(EVENT_ERROR, "Failed to create sem. (%d)", errno);
        break;
      }

      // Save mutex number to lock file.
      if (write(lockfd, &sem_id_, sizeof(int)) < 0) {
        Logger::Log(EVENT_ERROR, "Failed to save sem id. (%d)", errno);
        break;
      }
    } else {
      // Read mutex number from lock file.
      if (read(lockfd, &sem_id_, sizeof(int)) < 0) {
        Logger::Log(EVENT_ERROR, "Failed to read sem id. (%d)", errno);
        break;
      }
    }

    result = true;
  } while (false);

  if (result == false) {
    sem_id_ = -1;
  }

  flock(lockfd, LOCK_UN);
  close(lockfd);

  // Change file permission to allow Apache access.
  if (result && server_) {
    if (!AccessController::AllowApacheAccessFile(lock_file_, S_IRGRP)) {
      Logger::Log(EVENT_ERROR, "Failed to allow Apache allow lock file.");
      return false;
    }
  }
  return result;
}

void MutexSet::Destroy() {
  if (sem_id_ == -1) return;

  if (server_) {
    // Remove the semaphore.
    if (semctl(sem_id_, 0, IPC_RMID, 0) == -1) {
      Logger::Log(EVENT_ERROR, "Failed to remove sem. (%d)", errno);
    }
    remove(lock_file_.c_str());
  }

  sem_id_ = -1;
}

bool MutexSet::GetMutex(const std::string& name, bool init_value,
                        bool allow_multi_lock, Mutex* mutex) {
  if (mutexes_.find(name) == mutexes_.end()) {
    Logger::Log(EVENT_ERROR, "Unregistered mutex [%s].", name.c_str());
    return false;
  }

  int sem_num = mutexes_[name];
  if (server_) {
    semun arg;
    arg.val = init_value ? 1 : 0;
    if (semctl(sem_id_, sem_num, SETVAL, arg) == -1) {
      Logger::Log(EVENT_ERROR, "Failed to set sem value. (%d)", errno);
      return false;
    }
  }

  mutex->Initialize(allow_multi_lock, sem_id_, sem_num);

  return true;
}

#endif
