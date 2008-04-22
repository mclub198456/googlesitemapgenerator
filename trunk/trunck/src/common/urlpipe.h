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


// UrlPipe is a pipe used to send/receive URl information between main service
// and websever filter. It is designed as a one direction pipe. A pipe client
// couldn't read and write the pipe at same time. In addition to that, this
// pipe supports multiple writers from different process, but at the same time,
// only one reader is supported.

#ifndef COMMON_URLPIPE_H__
#define COMMON_URLPIPE_H__

#ifdef WIN32
#include <windows.h>
#elif defined(__linux__)
#include <pthread.h>
#endif

#include "common/urlloopbuffer.h"
#include "common/sitesettings.h"

// TODO(maoyq):
// refactor this class to define separated receiver and sender classes.
class UrlPipe {
 public:
  UrlPipe();
  virtual ~UrlPipe();

  // Initialize this UrlPipe.
  bool Initialize(bool is_receiver);

  // Send URL records to this UrlPipe.
  // "urlrecords" is pointer to an array, and "count" is the array size.
  // The constant "kSendWaitTime" determines its running mode. If the value is
  // negative, this method waits infinitely. If the value is positive, this
  // methods waits at most the "value" time. If the value is zero, this method
  // runs under non-blocking mode.
  // Returns the number of records sent, or -1 indicates error.
  int Send(UrlRecord *urlrecords, int count);

  // Receive URL records from this UrlPipe.
  // The records array is returned by "records" parameter. And the size of
  // array is indicated by return value. This class will take care of "records"
  // pointer, and caller shouldn't delete it after using.
  // This method is running in blocking mode, and waits infinitely.
  // Returning -1 means error occurs.
  int Receive(UrlRecord** records);

#ifdef WIN32
  // Initialize access control list for creating Event and Shared memory.
  // ppacl should be freed by caller if it is not null.
  // Default implementation allows access of Everyone.
  virtual bool InitACL(PACL* ppacl);

  // Release IPC objects.
  void ReleaseResource();

#else
  // Change the locking file directory.
  // This method is used for tests.
  void set_lock_dir(const std::string& lock_dir) {
    lock_dir_ = lock_dir;
  }

  // Create semaphore set and shared memory, and stores the IDs to a lock file.
  // This method is used by receiver.
  bool CreateResource();

  // Retrieve semaphore set id and shared memory id from a file.
  // This method is used sender.
  bool RetrieveResource();

  // Release resources.
  void ReleaseResource();
#endif

 private:
  // Max waiting time of sender in milliseconds.
  // If it is negative, sender should wait in blocking mode. If it is zero,
  // sender will return immediately.
  // Note, receiver is always in blocking mode in current implementation.
  const static int kSendWaitTime = 1000;

  // A flag indicating whether this UrlPipe is a receiver or a sender.
  bool is_receiver_;

  // UrlLoopBuffer used to send/receive URL records.
  // After this UrlPipe is initialized, the shared memory will be attached
  // to this UrlLoopBuffer. The buffer itself doesn't hold an internal memory.
  UrlLoopBuffer buffer_;

  // Memory used to hold UrlRecord read from buffer_.
  // This field is only used by reciever, and for a sender, it is always NULL.
  // Receiver uses this buffer in order to release IPC objects ASAP.
  UrlRecord* records_;

#ifdef WIN32
  // Initialize shared memory and attach it to buffer_.
  bool InitBuffer(SECURITY_ATTRIBUTES* sa);

  // Initialize read-write and notify events.
  bool InitEvents(SECURITY_ATTRIBUTES* sa);

  // IPC objects handles.
  // "rw_event_" is used to lock buffer_.
  HANDLE rw_event_;

  // "nofity_event_" is used by sender to tell receiver new data is available.
  HANDLE notify_event_;

  // "file_mapping_" is the handle for shared memory.
  HANDLE file_mapping_;

#elif defined(__linux__)
  // Resource includes a semaphore set id and a shared memory id.
  struct Resource {
    int sem_id;
    int shm_id;
  };

  // Semaphore index in the semaphore set.
  // As a result, RW (read-write) semaphore is the first semaphore in
  // this->resource_.sem_id semaphore set.
  // Notify semaphore is the second semaphore in this->resource_.sem_id.
  static const int kRwSemNum = 0;
  static const int kNotifySemNum = 1;

  // This value the retrieving period for IPC objects used by sender.
  // Under linux, the IPC objects are shared as a Client/Server architecture.
  // Receiver is the server, which creates IPC objects and stores IDs in a file.
  // Sender is the client, and it must polls the file to get correct ID.
  // See "Send" method for details.
  static const int kRetrievePeriod = 60;

  // Change the value of semnum semaphore in resource_.sem_id set.
  // It's a convenient wrapper of system "semctl" method.
  int SetSemValue(int semnum, int value);

  // Run semaphore operation for the semnum in resource_.sem_id.
  // It's a convenient wrapper of system "semop" method.
  int RunSemop(int num, int op, int flag, int wait_time);

  // Initialize semaphore.
  // For receiver it creates a semaphore set with two semaphores.
  // For sender, it just reads the semaphore from file.
  bool InitSem();

  // Initialize shared memory and attached it to UrlLoopBuffer.
  // For receiver it creates a shared memory and saves id to a file.
  // For sender, it just reads the shared memory id from file, and attaches it.
  bool InitBuffer();

  // Semaphore set id and shared memory id.
  Resource resource_;

  // The last polling time to retrieve resource ids.
  time_t last_retrieve_;

  // Where the lock files reside.
  std::string lock_dir_;
#endif

};

#endif  // COMMON_URLPIPE_H__
