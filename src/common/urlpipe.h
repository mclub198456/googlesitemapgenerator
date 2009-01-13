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


// UrlPipe is a pipe used to send/receive URl information between main service
// and websever filter. It is designed as a one direction pipe. A pipe client
// couldn't read and write the pipe at same time. In addition to that, this
// pipe supports multiple writers from different process, but at the same time,
// only one reader is supported.

#ifndef COMMON_URLPIPE_H__
#define COMMON_URLPIPE_H__

#ifdef WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__unix__)
#include <pthread.h>
#endif

#include "common/criticalsection.h"
#include "common/urlloopbuffer.h"
#include "common/sitesettings.h"
#include "common/mutexset.h"
#include "common/sharedmemory.h"

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

  bool AcquireResource();

  // Release IPC objects.
  void ReleaseResource();

  static const char* kSharedMemName;
  static const char* kRwMutexName;
  static const char* kNotifyMutexName;

 private:
  // This value the retrieving period for IPC objects used by sender.
  // Under linux, the IPC objects are shared as a Client/Server architecture.
  // Receiver is the server, which creates IPC objects and stores IDs in a file.
  // Sender is the client, and it must polls the file to get correct ID.
  // See "Send" method for details.
  static const int kRetrievePeriod = 60;

  static const int kSendWaitTime = 100;

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

  time_t last_acquire_resource_;

  MutexSet mutex_set_;

  // IPC objects handles.
  // "rw_event_" is used to lock buffer_.
  Mutex rw_mutex_;

  // "nofity_event_" is used by sender to tell receiver new data is available.
  Mutex notify_mutex_;

  // "file_mapping_" is the handle for shared memory.
  SharedMemory shared_mem_;

};

#endif  // COMMON_URLPIPE_H__
