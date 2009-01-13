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

// SharedMemory provides a unified object oriented interface for shared
// memory resource across different platforms.

#ifndef COMMON_SHAREDMEMORY_H__
#define COMMON_SHAREDMEMORY_H__

#include <string>

class SharedMemory {
 public:
  SharedMemory();
  ~SharedMemory();

  // Create a shared memory.
  // "name" is the unique name of this shared memory.
  // "size" is the memory size.
  // "server" indicates whether this user is a server or not.
  // "exist" returns whether shared memory with given "name" already exists.
  bool Create(const std::string& name, size_t size, bool server, bool* exist);

  // Destroy this shared memory.
  // Related system resources are released.
  void Destroy();

  // Returns the pointer to shared memory.
  void* data() const {
    return data_;
  }

  // Get the name of this SharedMemory.
  const std::string& name() const {
    return name_;
  }

 private:
  void* data_;
  std::string name_;
  bool server_;

#ifdef WIN32
  HANDLE file_mapping_;
#else
  int shm_id_;
  std::string lock_file_;
#endif
};

#endif // COMMON_SHAREDMEMORY_H__
