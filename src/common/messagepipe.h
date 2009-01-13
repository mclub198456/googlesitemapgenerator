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

// MessagePipe is used to transport messages between CGI exe and sitemap
// generator daemon. The sitemap generator daemon should use this pipe in
// server mode while the CGI exe should use it in client mode.
// Only one client can connect this pipe at one time.
// Please note that this message pipe is only designed for very restricted
// use. The implementation is very simple. A shared memory is used to transport
// message. Mutexes (binary semaphore) are used to ensure read/write sequences.
// There are two mutexes, server mutex and client mutex.

#ifndef COMMON_MESSAGEPIPE_H__
#define COMMON_MESSAGEPIPE_H__

#include "common/urlloopbuffer.h"
#include "common/sitesettings.h"
#include "common/mutexset.h"
#include "common/interproclock.h"
#include "common/sharedmemory.h"

class MessagePipe {
 public:
  // Message block transported in this pipe.
  // Long message should be splitted into several blocks, which are transported
  // by this pipe several times.
  struct MessageBlock {
    static const int kBlockSize = 1024 * 64;

    // Base 0 index of all blocks.
    int block_index;

    // Number of blocks will be used for a long message.
    int block_count;

    // Current data length. It shouldn't exceed kBlockSize.
    int length;

    // Message data.
    char data[kBlockSize];
  };

  MessagePipe(bool is_server);
  virtual ~MessagePipe();

  // Initialize this pipe.
  // Server should take the responsibility to create all necessary resources.
  bool Initialize();

  // Send a message.
  bool Send(const std::string& message);

  // Receive a message.
  bool Receive(std::string* message);

  // Inter-process lock for this pipe.
  // Because only one client is allowed to use pipe at one time,
  // client should lock this pipe before using it.
  bool Lock(int wait_ms);

  // Unlock this message pipe.
  void Unlock();

  // Reset mutexes used by this pipe.
  // It should be called if some thing error occurs in Send or Receive methods.
  // This method should only be called by server.
  bool ResetMutex();

 private:
  // Acquire necessary resources.
  bool AcquireResource();

  // Release all resources used in this pipe.
  void ReleaseResource();

  // Names for system resources.
  static const char* kSharedMemName;
  static const char* kServerMutexName;
  static const char* kClientMutexName;

  // Default waiting time for blocking methods in Send/Recieve method.
  static const int kWaitTime = 5000;

  // Lock name for message pipe.
  static const char* kLockName;

  // Inidicates whether this pipe is used in server mode or not.
  bool is_server_;

  // Used to transport message.
  // It will be attached to a shared memory.
  MessageBlock* block_;

  // Mutex set used by this pipe.
  // This set contains two mutexes, owner mutex and peer mutex.
  MutexSet mutex_set_;
  Mutex owner_mutex_;
  Mutex peer_mutex_;

  // Shared memory used by this pipe.
  SharedMemory shared_mem_;

  InterProcLock* lock_;
};

#endif  // COMMON_MESSAGECONVEYER_H__
