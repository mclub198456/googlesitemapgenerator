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


// A portable thread class.
// This thread class provides many features, like thread priority, joinable
// and stoppable ability. But some features are not fully supported. Please
// see the method doc for details.

#ifndef COMMON_THREAD_H__
#define COMMON_THREAD_H__

#include "common/basictypes.h"
#include "common/criticalsection.h"

#if defined(__linux__) || defined(__unix__)
#include <pthread.h>
#include <unistd.h>
#endif

class Thread {
public:

  typedef void* (*ThreadEntry)(void*);

  enum ThreadPriority {
    PRIORITY_NORMAL,
    PRIORITY_LOW,
    PRIORITY_HIGH
  };

  Thread();

  Thread(ThreadPriority priority);

  // Stop and release the thread.
  virtual ~Thread();

  // Start the thread.
  // In this scenario, Run method must be overridden.
  bool Start();

  // Start the thread with specified entry point.
  // In this scenario, Run method shouldn't be overriden.
  // This method is provided for legacy code.
  bool Start(ThreadEntry entry, void* arg);

  // Try to stop the thread.
  // Actually, it only signals the thread to stop, but doens't stop it
  // immediately. WaitOrDie should be called in Run to make it stoppable.
  void Stop();

  // Wait the thread to die.
  void Join();

protected:

#ifdef WIN32
  static DWORD WINAPI EntryPoint(LPVOID thread);
#elif defined(__linux__) || defined(__unix__)
  static void* EntryPoint(void* thread);
#endif

  // By default, entry passed in Start is invoked in Run.
  // if no entry is given, this thread takes no effect.
  virtual void Run();

  // See Stop().
  void WaitOrDie(int seconds);

private:
  // Change thread priority to the value specified by this->priority_.
  void ChangePriority();

  // Thread entry point funtion.
  ThreadEntry entry_;

  // Argument passed to thread entry point.
  void* arg_;

  // Thread priority value.
  ThreadPriority priority_;

  // Thread handles.
#ifdef WIN32
  HANDLE thread_;
  HANDLE stop_event_;
#elif defined(__linux__) || defined(__unix__)
  pthread_t thread_;
#endif

  DISALLOW_EVIL_CONSTRUCTORS(Thread);
};


#endif // COMMON_THREAD_H__
