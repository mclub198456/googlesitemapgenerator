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


#include "sitemapservice/daemon.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common/logger.h"
#include "sitemapservice/servicecontroller.h"

ServiceController* Daemon::controller_ = NULL;

int Daemon::Start() {
  if (IsRunning()) {
    printf("Service is already running.\n");
    return 0;
  }

  pid_t pid = fork();
  if (pid < 0) {
    Logger::Log(EVENT_ERROR, "Daemon can't fork a child thread. (%d)", errno);
    return errno;
  }

  // mainthread: save pid and write it to pid file
  if (pid > 0) {
    FILE* fp = fopen(GetPidFile().c_str(), "w");
    if (fp == NULL) {
      Logger::Log(EVENT_ERROR, "Daemon can't open pid file. (%d)", errno);
      return errno;
    }
    fprintf(fp, "%d", pid);
    fclose(fp);

    return 0;
  }

  // following code is for child process

  // start new session for child process
  pid_t sid = setsid();
  if (sid < 0) {
    Logger::Log(EVENT_ERROR, "Daemon can't start new seesion. (%d)", errno);
    return errno;
  }

  // change child process' working directory to root
  if (chdir("/") < 0) {
    Logger::Log(EVENT_ERROR, "Deamon can't change CWD to root. (%d)", errno);
    return errno;
  }

  // register signal handler
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = &HandleSIGTERM;
  action.sa_flags |= SA_RESTART;
  if (sigaction(SIGTERM, &action, NULL) != 0) {
    Logger::Log(EVENT_ERROR, "Deamon can't register TERM handler. (%d)",
              errno);
    return errno;
  }

  action.sa_handler = &HandleSIGUSR1;
  if (sigaction(SIGUSR1, &action, NULL) != 0) {
    Logger::Log(EVENT_ERROR, "Deamon can't register USR1 handler. (%d)",
              errno);
    return errno;
  }

  // close std io when in production
  int devnull = open("/dev/null", O_RDWR, 0);
  if (devnull == -1) {
    Logger::Log(EVENT_ERROR, "Daemon can't open /dev/null. (%d)", errno);
    return errno;
  }

  dup2(devnull, STDIN_FILENO);
  dup2(devnull, STDOUT_FILENO);
  dup2(devnull, STDERR_FILENO);

  if (devnull > 2) close(devnull);

  return RunService();
}

int Daemon::Stop() {
  pid_t pid = GetPid();
  if (pid == -1) {
    printf("No service is running.\n");
    return 0;
  }

  if (kill(pid, SIGTERM) != -1) {
    // Wait it to die.
    while (kill(pid, 0) != -1) {
      sleep(1);
    }
    if (errno != ESRCH) {
      Logger::Log(EVENT_ERROR, "Failed to terminate sitemap-daemon.");
      return -1;
    }
    remove(GetPidFile().c_str());
    return 0;
  } else {
    Logger::Log(EVENT_ERROR, "Daemon can't be terminated. (%d)", errno);
    return errno;
  }
}

int Daemon::ReloadSetting() {
  pid_t pid = GetPid();
  if (pid == -1) {
    fprintf(stderr, "No service is running.\n");
    return 1;
  }

  if (kill(pid, SIGUSR1) != -1) {
    printf("Reload setting command is sent.\n");
    return 0;
  } else {
    fprintf(stderr, "Reload setting command can't be sent.\n");
    Logger::Log(EVENT_ERROR, "Failed to send reload command. (%d)", errno);
    return 2;
  }
}

int Daemon::Restart() {
  // try to stop it first
  if (Daemon::GetPid() != -1) {
    int result = Daemon::Stop();
    if (result != 0) {
      return result;
    }
  }

  return Daemon::Start();
}

// run the service
int Daemon::RunService() {
  controller_ = new ServiceController();

  if (!controller_->Initialize()) {
    Logger::Log(EVENT_ERROR, "Service can't be initialized.");
    return 1234567;  // not a system error
  }

  // Schedule service every one second.
  while (true) {
    controller_->ScheduleService();
    sleep(1);
  }

  if (controller_ != NULL) delete controller_;

  return 0;
}

// check whether the service process is running
bool Daemon::IsRunning() {
  return GetPid() != -1;
}

// handle the TERM signal: Stop service
void Daemon::HandleSIGTERM(int signo) {
  if (signo != SIGTERM) return;

  if (controller_ == NULL) {
    return;
  }

  controller_->StopService();
  delete controller_;
  controller_ = NULL;

  exit(0);
}

void Daemon::HandleSIGUSR1(int signo) {
  if (signo != SIGUSR1) return;

  if (controller_ != NULL) {
    controller_->ReloadSetting();
  }
}

// get service process id from pid file
pid_t Daemon::GetPid() {
  FILE* fp = fopen(GetPidFile().c_str(), "r");
  if (fp == NULL) {
    return -1;
  }

  pid_t pid;
  if (fscanf(fp, "%d", &pid) == EOF) {
    fclose(fp);
    return -1;
  }
  fclose(fp);

  if (kill(pid, 0) == -1) {
    return -1;
  }

  return pid;
}

std::string Daemon::GetPidFile() {
  std::string file = Util::GetApplicationDir();
  file.append("/run/google-sitemap-generator.pid");
  return file;
}
