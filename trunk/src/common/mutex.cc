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

#include "common/mutex.h"

#include "common/logger.h"

#ifdef WIN32

Mutex::Mutex() {
  event_ = NULL;
}

Mutex::~Mutex() {
  // does nothing.
}

void Mutex::Initialize(bool allow_multi_post, HANDLE event) {
  allow_multi_post_ = allow_multi_post;
  event_ = event;
}

void Mutex::Destroy() {
  event_ = NULL;
}

// Use WaitForSingleObject to implement this method.
Mutex::MutexResult Mutex::Wait(int wait_ms) {
  if (event_ == NULL) return MUTEX_INVALID;

  DWORD result = WaitForSingleObject(event_, wait_ms < 0 ? INFINITE : wait_ms);
  if (result == WAIT_OBJECT_0 ||
    (allow_multi_post_ && result == WAIT_ABANDONED)) {
    return MUTEX_OK;
  }

  if (result == WAIT_TIMEOUT) {
    return MUTEX_TIMEOUT;
  } else {
    return GetLastError() == ERROR_INVALID_HANDLE ? MUTEX_INVALID : MUTEX_ERROR;
  }
}

// Use SetEvent to implement this method.
Mutex::MutexResult Mutex::Post() {
  if (event_ == NULL) return MUTEX_INVALID;

  if (SetEvent(event_)) {
    return MUTEX_OK;
  } else {
    return GetLastError() == ERROR_INVALID_HANDLE ? MUTEX_INVALID : MUTEX_ERROR;
  }
}

int Mutex::LastError() {
  return GetLastError();
}

#else  // __linux__ || __unix__
#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

Mutex::Mutex() {
  sem_id_ = -1;
  sem_num_ = -1;
}

Mutex::~Mutex() {
  // does nothing.
}

void Mutex::Initialize(bool allow_multi_post, int sem_id, int sem_num) {
  allow_multi_post_ = allow_multi_post;
  sem_id_ = sem_id;
  sem_num_ = sem_num;
}

void Mutex::Destroy() {
  sem_id_ = -1;
  sem_num_ = -1;
}

// Use semop to implement this method.
Mutex::MutexResult Mutex::Wait(int wait_ms) {
  if (sem_id_ == -1) return MUTEX_INVALID;

  int result = RunSemop(true, !allow_multi_post_, wait_ms);
  return ErrnoToMutexResult(result);
}

// Use semop or semctl to implement this method.
Mutex::MutexResult Mutex::Post() {
  if (sem_id_ == -1) return MUTEX_INVALID;

  if (allow_multi_post_) {
    int arg = 1;
    int result = semctl(sem_id_, sem_num_, SETVAL, arg);
    return ErrnoToMutexResult(result == -1 ? errno : 0);
  } else {
    int result = RunSemop(false, true, 0);
    return ErrnoToMutexResult(result);
  }
}

int Mutex::LastError() {
  return errno;
}

Mutex::MutexResult Mutex::ErrnoToMutexResult(int err) {
  switch (err) {
    case 0:
      return MUTEX_OK;
    case EAGAIN:
      return MUTEX_TIMEOUT;
    case EIDRM:
    case EINVAL:
      return MUTEX_INVALID;
    default:
      return MUTEX_ERROR;
  }
}

// A convenient wrapper for semop call.
int Mutex::RunSemop(bool wait, bool undo, int wait_time) {
  sembuf operation;
  operation.sem_num = static_cast<short>(sem_num_);
  operation.sem_flg = undo ? SEM_UNDO : 0;
  operation.sem_op = wait ? -1 : 1;

  int result = -1;
  if (wait_time < 0) {
    result = semop(sem_id_, &operation, 1);
  } else if (wait_time == 0) {
    operation.sem_flg |= IPC_NOWAIT;
    result = semop(sem_id_, &operation, 1);
  } else {
#if defined(__linux__)
    timespec timeout;
    timeout.tv_sec = wait_time / 1000;
    timeout.tv_nsec = (wait_time % 1000) * 1000000;
    result = semtimedop(sem_id_, &operation, 1, &timeout);
#elif defined(__unix__)
    result = semop(sem_id_, &operation, 1);
#endif
  }

  return result == 0 ? 0 : errno;
}
#endif

bool Mutex::Reset(bool value) {
  if (allow_multi_post_ == false) {
    Logger::Log(EVENT_ERROR, "Invalid operation of reset mutex.");
    return false;
  }

  if (value) {
    return Post() == MUTEX_OK;
  } else {
    MutexResult res = Wait(0);
    return res == MUTEX_OK || res == MUTEX_TIMEOUT;
  }
}
