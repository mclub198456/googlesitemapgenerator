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

// MutexSet is used to manage a set of mutex (binary semaphore).
// To be clear, in *nix system, it represents a semaphore set,
// while in Windows system, it represents a collection of Event objects.
// It is designed to ensure MutexSet has same interface in different system.

#ifndef COMMON_MUTEXSET_H__
#define COMMON_MUTEXSET_H__

#include <string>
#include <map>

#include "common/mutex.h"

class MutexSet {
 public:
  MutexSet();
  ~MutexSet();

  // Name of mutex set, which should be unique among all instances of MutexSet.
  // "server" indicates whether this MutexSet is in server mode or not.
  // If same mutex set is used by different processes, only one user can
  // initialize it in server mode.
  bool Initialize(const std::string& name, bool server);

  // Register a mutex.
  // All mutexes should be registered before Load method.
  int RegisterMutex(const std::string& name);

  // Load all the registered mutexes.
  // GetMutex can be invoked after success execution of this method.
  bool Load();

  // Destroy all the loaded mutexes.
  // This MutexSet can't be used before Load is invoked again.
  void Destroy();

  // Get Mutex with given name.
  // "name" should be registered in RegisterMutex method.
  // "init_value" is the initial state of mutex.
  // "allow_multi_post" indicates that whether the Mutex can be Posted more
  // than once.
  // Note, mutex can only be gotten once.
  bool GetMutex(const std::string& name, bool init_value,
                bool allow_multi_post, Mutex* mutex);

 private:
  bool server_;

  // Mutex name to internal mutex id mapping.
  std::map<std::string, int> mutexes_;

#ifdef WIN32
  std::map<std::string, HANDLE> events_;
#else
  // Semaphore id.
  int sem_id_;
  
  // Lock file to save semaphore id.
  // Server writes sem_id_ to this file while client can read sem_id_ from this
  // file.
  std::string lock_file_;
#endif
};

#endif // COMMON_MUTEXMANAGER_H__
