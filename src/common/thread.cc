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


#include "common/thread.h"
#include "common/logger.h"

#if defined(__linux__) || defined(__unix__)
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

Thread::Thread() {  
  entry_ = NULL;
  arg_ = NULL;
  thread_ = 0;

  priority_ = PRIORITY_NORMAL;
}

Thread::Thread(ThreadPriority priority) {
  entry_ = NULL;
  arg_ = NULL;
  thread_ = 0;

  priority_ = priority;
}

Thread::~Thread() {
  Stop();
}

void Thread::Run() {
  if (entry_ != NULL) (*entry_)(arg_);
}

#ifdef WIN32
DWORD WINAPI Thread::EntryPoint(LPVOID threadinstance) {
#elif defined(__linux__) || defined(__unix__)
void* Thread::EntryPoint(void *threadinstance) {
#endif
  Thread* thread = reinterpret_cast<Thread*>(threadinstance);
  thread->ChangePriority();
  thread->Run();
  return NULL;
}


bool Thread::Start() {
#ifdef WIN32
  // Create stop event, which can be used to notify the thread to stop.
  stop_event_ = CreateEvent(0, FALSE, FALSE, NULL);
  if (stop_event_ == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to create stop event for thread. (%d).",
              GetLastError());
    return false;
  } else {
    DLog(EVENT_ERROR, "Event for thread [%X].", stop_event_);
  }

  // Create thread.
  thread_ = CreateThread(0, 0, &EntryPoint, this, 0, NULL);
  if (thread_ == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to create thread. (%d).", GetLastError());
    return false;
  }

#elif defined(__linux__) || defined(__unix__)
  // Create a pthread.
  int result = pthread_create(&thread_, NULL, &EntryPoint, this);
  if (result != 0) {
    Logger::Log(EVENT_ERROR, "Failed to create thread. (%d).", result);
    return false;
  }
#endif
  return true;
}

bool Thread::Start(ThreadEntry entry, void* arg) {
  entry_ = entry;
  arg_ = arg;

  return Start();
}

void Thread::Stop() {
  if (thread_ == 0) return;

#ifdef WIN32
  if (!SetEvent(stop_event_)) {
    Logger::Log(EVENT_ERROR, "Failed to set event to stop thread. (%d)",
              GetLastError());
  }
#elif defined(__linux__) || defined(__unix__)
    pthread_cancel(thread_);
#endif
}

void Thread::WaitOrDie(int seconds) {
#ifdef WIN32
  DWORD res = WaitForSingleObject(stop_event_, seconds * 1000);
  if (res == WAIT_FAILED) {
    Logger::Log(EVENT_ERROR, "Failed to wait thread stop object. (%d)",
              GetLastError());
  } else if (res == WAIT_OBJECT_0) {
    CloseHandle(stop_event_);
    CloseHandle(thread_);
    thread_ = 0;
    ExitThread(0);
  }
#elif defined(__linux__) || defined(__unix__)
  // sleep is a cancellation point.
  int left = 0;
  while ((left = sleep(seconds)) != 0) {
    if (errno == EINTR) {
      seconds = left;
    } else {
      Logger::Log(EVENT_ERROR, "Failed to wait thread to stop. (%d)", errno);
      thread_ = 0;
      pthread_detach(pthread_self());
      pthread_exit(NULL);
    }
  }
#endif
}

void Thread::ChangePriority() {
#ifdef WIN32
  BOOL result = TRUE;
  if (priority_ == PRIORITY_LOW) {
    result = ::SetThreadPriority(thread_, THREAD_PRIORITY_BELOW_NORMAL);
  } else if (priority_ == PRIORITY_HIGH) {
    result = ::SetThreadPriority(thread_, THREAD_PRIORITY_ABOVE_NORMAL);
  }

  if (result == FALSE) {
    Logger::Log(EVENT_ERROR, "Failed to change thread priority. (%d)",
              GetLastError());
  }
#elif defined(__linux__) || defined(__unix__)
  int result = 0;
  if (priority_ == PRIORITY_LOW) {
    result = setpriority(PRIO_PROCESS, 0, 5);
  } else if (priority_ == PRIORITY_HIGH) {
    result = setpriority(PRIO_PROCESS, 0, -5);
  }

  if (result != 0) {
    Logger::Log(EVENT_ERROR, "Failed to change thread priority. (%d)",
              errno);
  }
#endif
}

void Thread::Join() {
  if (thread_ == 0) return;

#ifdef WIN32
  if (WaitForSingleObject(thread_, INFINITE) == WAIT_FAILED) {
    Logger::Log(EVENT_ERROR, "Failed to wait thread exit, ignore.");
  }
#elif defined(__linux__) || defined(__unix__)
  // All kinds of errors could be ignored.
  void* thread_return;
  pthread_join(thread_, &thread_return);
#endif
}
