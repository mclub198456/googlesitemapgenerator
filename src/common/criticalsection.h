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


// This file contains definition of portable critical section class, and the
// other two tight related classes, AutoLeave and ThreadSafeVar.
// CriticalSection class provides two most fundamental methods, Enter and Leave.
// AutoLeavel class is a wrapper of CriticalSection object. The caller can leave
// the wrapped critical section automatically when AutoLeave object is
// dectroyed. ThreadSafeVar is a template class. It could be used to define
// thread safe variables.

#ifndef COMMON_CRITICALSECTION_H__
#define COMMON_CRITICALSECTION_H__

#ifdef WIN32

#include <windows.h>

#elif defined(__linux__) || defined(__unix__)

#include <pthread.h>

#endif

// Note, for portability across platform, CriticalSection neither can be entered
// twice, nor can be left without entering first.
class CriticalSection {
 public:
  CriticalSection();
  ~CriticalSection();

  // Enter the critical section.
  // This method provides two mode, blocking mode and non-blocking mode. The
  // mode is determined by "wait" argument.
  bool Enter(bool wait);

  // Leave critical section.
  // This method shouldn't be called without successful entering first.
  void Leave();

 private:

#ifdef WIN32
  CRITICAL_SECTION criticalsection_;
#elif defined(__linux__) || defined(__unix__)
  pthread_mutex_t mutex_;
#endif
};


class AutoLeave {
 public:
  // Constructor.
  // The given CriticalSection should be already entered.
  AutoLeave(CriticalSection* cs);

  // Destructor.
  // CriticalSection is left automatically here.
  ~AutoLeave();

  // Leave the critical section ahead.
  void LeaveAhead();

 private:
  CriticalSection* critical_section_;
  bool left_;  // Inicates whether the critical_section_ is already left.
};


// Template for thread-safe variable.
template <typename T>
class ThreadSafeVar {
public:
  ThreadSafeVar() {
  }
  ThreadSafeVar(const T& value) {
    value_ = value;
  }

  // Get the value of internal variable.
  operator T() {
    T temp;
    cs_.Enter(true);
    temp = value_;
    cs_.Leave();
    return temp;
  }

  // Set new value of internal variable.
  const T& operator=(const T& value) {
    cs_.Enter(true);
    value_ = value;
    cs_.Leave();
    return value;
  }

private:
  // Disable copy constructor for internal critical section can't be copied.
  ThreadSafeVar(const ThreadSafeVar& another) {}

  // Wrapped variable.
  T value_;

  // CriticalSection used to ensure thread safety.
  CriticalSection cs_;
};

#endif  // COMMON_CRITICALSECTION_H__
