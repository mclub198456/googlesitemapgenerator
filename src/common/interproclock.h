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

// A simple file based inter-process lock.
// The implementation is based on disk file locking. Please do not use it if
// you don't know how it works.

#ifndef COMMON_INTERPROCLOCK_H__
#define COMMON_INTERPROCLOCK_H__

#include <string>

class InterProcLock {
public:
  // Special wait time for lock.
  static const int kWaitInfinite = (2 << 31) - 1;
  static const int kWaitNoneBlock = 0;

  // Construct a named lock.
  // This name should be unique for among all its instances.
  InterProcLock(const std::string& name);
  InterProcLock(const std::string& name, bool with_webserver, bool is_server);
  ~InterProcLock();

  // Try to obtain the lock.
  bool Lock(int wait_ms);

  // Unlock.
  bool Unlock();

  // Get the file path representing this lock.
  // User can change the permissions of the locking file.
  const std::string& file() const {
    return file_;
  }

private:
  // Represents the polling period of Lock method in milli-seconds.
  static const int kPollingPeriod = 100;

  void Initialize(const std::string& name, bool with_webserver, bool is_server);

  // Lock name.
  std::string name_;

  // File path of this lock.
  std::string file_;

  // Whether this lock shared by webserver.
  bool with_webserver_;
  // Whether this is server if with_webserver_ is true.
  bool is_server_;

#ifdef WIN32
  // File handle opened by this lock.
  HANDLE handle_;
#else
  // File descriptor opened by this lock.
  int descriptor_;
#endif
};

#endif // COMMON_INTERPROCLOCK_H__
