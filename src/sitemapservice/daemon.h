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


#ifndef SITEMAPSERVICE_DAEMON_H__
#define SITEMAPSERVICE_DAEMON_H__

#include "sitemapservice/servicecontroller.h"

class Daemon {
public:

  // start service as a daemon
  static int Start();

  // stop the service daemon
  static int Stop();

  // restart service daemon
  static int Restart();

  // Reload service setting.
  static int ReloadSetting();

  // run service as a stand alone app
  static int RunService();

  // check whether the daemon is running
  static bool IsRunning();

private:
  static pid_t GetPid();

  static std::string GetPidFile();

  static void HandleSIGTERM(int signo);

  static void HandleSIGUSR1(int signo);

  static ServiceController* controller_;
};

#endif // SITEMAPSERVICE_DAEMON_H__
