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


#include "common/port.h"
#include "sitemapservice/taskrunner.h"

TaskRunner::TaskRunner()
: Thread(Thread::PRIORITY_LOW) {
}

TaskRunner::~TaskRunner(){
  while (!tasks_.empty()) {
    delete tasks_.top().task;
    tasks_.pop();
  }
}

void TaskRunner::RegisterTask(Task* task, time_t start, int period) {
  if (task == NULL) return;

  TaskEntry entry = {task, start, period};
  tasks_.push(entry);
}

void TaskRunner::Run() {
  while (true) {
    TaskEntry taskentry = tasks_.top();
    time_t now = time(NULL);
    int wait = 0;

    if (taskentry.next_run <= now) {
      taskentry.task->Run();
      // update the task entry
      tasks_.pop();
      taskentry.next_run = time(NULL) + taskentry.period;
      tasks_.push(taskentry);
    } else {
      wait = static_cast<int>(taskentry.next_run - now);
    }

    Thread::WaitOrDie(wait);
  }
}
