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

// Mutex is a traditinal binary semaphore, which can be used among processes.
// Usually, it should be retrieved from a MutexSet instead of being created
// directly.

#ifndef COMMON_MUTEX_H__
#define COMMON_MUTEX_H__

class Mutex {
 public:
  // Possbile results for mutex operations.
  enum MutexResult {
    MUTEX_OK = 0,
    MUTEX_ERROR = 1,
    MUTEX_TIMEOUT = 2,
    MUTEX_INVALID = 3
  };

  Mutex();
  ~Mutex();

  // Mutex should be intialized before anyother method.
#ifdef WIN32
  void Initialize(bool allow_multi_post, HANDLE event);
#else
  void Initialize(bool allow_multi_post, int sem_id, int sem_num);
#endif

  // Destroy this mutex.
  // It is called automatically in destructor.
  void Destroy();

  // Reset the status of this mutex.
  bool Reset(bool value);

  // Wait the mutex value to be true.
  // Use kWaitInfinite as wait_ms if you want to block this method.
  MutexResult Wait(int wait_ms);

  // Post the mutex. (Set the mutex value to be true.)
  MutexResult Post();

  // Get last error from mutex operation.
  int LastError();

  static const int kWaitInfinite = -1;

 private:
#ifdef WIN32
#else
  // Change errno to MutexResult.
  MutexResult ErrnoToMutexResult(int err);

  // A convenience wrapper of semop call.
  int RunSemop(bool wait, bool undo, int wait_ms);
#endif

  // Indicates whether this mutex allow multiple Post operation.
  bool allow_multi_post_;

#ifdef WIN32
  HANDLE event_;
#else
  int sem_id_;
  int sem_num_;
#endif
};

#endif // COMMON_MUTEX_H__
