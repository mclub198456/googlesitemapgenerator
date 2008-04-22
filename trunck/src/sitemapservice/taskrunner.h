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


#ifndef SITEMAPSERVICE_TASKRUNNER_H__
#define SITEMAPSERVICE_TASKRUNNER_H__

#include "common/basictypes.h"
#include "common/thread.h"
#include <queue>

// The abstract class defines an interface which every Task must follow.
class Task {
public:
  virtual ~Task() {}
  virtual void Run() = 0;
protected:
  Task() {}
};


// Class used to run tasks periodically.
// This class is not thread-safe.
class TaskRunner : public Thread {
public:
  TaskRunner();
  virtual ~TaskRunner();

  // period is in seconds
  void RegisterTask(Task* task, time_t start, int period);

  int CountTasks() { return static_cast<int>(tasks_.size()); }

  virtual void Run();

private:
  struct TaskEntry {
    Task* task;
    time_t next_run;
    int period;

    bool operator<(const TaskEntry& entry) const {
      return next_run > entry.next_run;
    }
  };

  std::priority_queue<TaskEntry> tasks_;

  DISALLOW_EVIL_CONSTRUCTORS(TaskRunner);
};

#endif // SITEMAPSERVICE_TASKRUNNER_H__

