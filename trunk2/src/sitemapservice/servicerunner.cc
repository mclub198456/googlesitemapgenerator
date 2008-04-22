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


#include "sitemapservice/servicerunner.h"

#include "common/util.h"

///////////////////////////////////////////////////////////
// Implementation for ServicePool class.
ServicePool::ServicePool() {
  // do nothing.
}

ServicePool::~ServicePool() {
  lock_.Enter(true);

  std::list<ServiceInterface*>::iterator itr = sleeping_services_.begin();
  for (; itr != sleeping_services_.end(); ++itr) {
    delete *itr;
  }

  itr = waiting_services_.begin();
  for (; itr != waiting_services_.end(); ++itr) {
    delete *itr;
  }

  // Ignore running service...
  lock_.Leave();
}

ServiceInterface* ServicePool::GetService() {
  ServiceInterface* service = NULL;
  lock_.Enter(true);
  if (waiting_services_.size() > 0) {
    service = waiting_services_.front();
    waiting_services_.pop_front();
    running_services_.insert(service);
  }
  lock_.Leave();
  return service;
}

void ServicePool::ReturnService(ServiceInterface* service) {
  lock_.Enter(true);
  sleeping_services_.push_back(service);
  running_services_.erase(service);
  lock_.Leave();
}

int ServicePool::ScheduleService() {
  lock_.Enter(true);
  AutoLeave lock_autoleave(&lock_);

  // 1) Move services from sleeping list to waiting list.
  int next_waittime = 7 * 24 * 3600;  // at most one week.
  std::list<ServiceInterface*>::iterator itr = sleeping_services_.begin();
  while(itr != sleeping_services_.end()) {
    int wait_time = (*itr)->GetWaitTime();
    if (wait_time <= 0) {
      waiting_services_.push_back(*itr);
      itr = sleeping_services_.erase(itr);
    } else {
      next_waittime = std::min<int>(next_waittime, wait_time);
      ++itr;
    }
  }

  // 2) Check the waiting list to get wait time.
  itr = waiting_services_.begin();
  for (; itr != waiting_services_.end(); ++itr) {
    next_waittime = std::min<int>(next_waittime, (*itr)->GetRunningPeriod());
  }

  // 3) Check the running list to get wait time.
  std::set<ServiceInterface*>::iterator set_itr = running_services_.begin();
  for (; set_itr != running_services_.end(); ++set_itr) {
    next_waittime = std::min<int>(next_waittime, (*set_itr)->GetRunningPeriod());
  }

  // Check for stupid error.
  if (next_waittime <= 0) next_waittime = 1;

  return next_waittime;
}

///////////////////////////////////////////////////////////
// Implementation for ServiceRunner class.

ServiceRunner::ServiceRunner()
: Thread(Thread::PRIORITY_LOW) {
  pool_ = NULL;
}

ServiceRunner::ServiceRunner(Thread::ThreadPriority priority)
: Thread(priority) {
  pool_ = NULL;
}

bool ServiceRunner::Start(ServicePool* pool) {
  pool_ = pool;
  return Thread::Start();
}

void ServiceRunner::Run() {
  Util::Log(EVENT_NORMAL, "Thread [%X] is started to run.", this);

  while (true) {
    if (pool_ != NULL) {
      ServiceInterface* service = pool_->GetService();
      if (service != NULL) {
        Util::Log(EVENT_NORMAL, "Thread [%X] is used to run service [%X].",
                  this, service);
        service->Run();
        pool_->ReturnService(service);
      }
    }

    WaitOrDie(1);
  }
}

