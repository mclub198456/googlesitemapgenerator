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


#include "sitemapservice/servicerunner.h"

#include "common/logger.h"

///////////////////////////////////////////////////////////
// Implementation for ServiceQueue class.
ServiceQueue::ServiceQueue() {
  // do nothing.
}

ServiceQueue::~ServiceQueue() {
  // do nothing.
}

ServiceInterface* ServiceQueue::PopService() {
  ServiceInterface* service = NULL;
  lock_.Enter(true);
  if (waiting_services_.size() > 0) {
    service = waiting_services_.front();
    waiting_services_.pop_front();
  }
  lock_.Leave();
  return service;
}

void ServiceQueue::PushService(ServiceInterface* service) {
  lock_.Enter(true);
  waiting_services_.push_back(service);
  all_services_.insert(service);
  lock_.Leave();
}

void ServiceQueue::CompleteService(ServiceInterface* service) {
  lock_.Enter(true);
  all_services_.erase(service);
  lock_.Leave();
}

bool ServiceQueue::Contains(ServiceInterface* service) {
  lock_.Enter(true);
  AutoLeave leave_lock(&lock_);

  return all_services_.find(service) != all_services_.end();
}


void ServiceQueue::RemoveWaitingServices(
  const std::list<ServiceInterface*>& services) {
  lock_.Enter(true);
  AutoLeave leave_lock(&lock_);

  std::set<ServiceInterface*> services_set(services.begin(), services.end());

  std::list<ServiceInterface*>::iterator itr = waiting_services_.begin();
  while (itr != waiting_services_.end()) {
    if (services_set.find(*itr) != services_set.end()) {
      all_services_.erase(*itr);
      itr = waiting_services_.erase(itr);
    } else {
      ++itr;
    }
  }
}

void ServiceQueue::RemoveAllWaitingServices() {
  lock_.Enter(true);
  AutoLeave leave_lock(&lock_);

  std::list<ServiceInterface*>::iterator itr = waiting_services_.begin();
  while (itr != waiting_services_.end()) {
    all_services_.erase(*itr);
    itr = waiting_services_.erase(itr);
  }
}


///////////////////////////////////////////////////////////
// Implementation for ServiceRunner class.

ServiceRunner::ServiceRunner()
: Thread(Thread::PRIORITY_LOW) {
  queue_ = NULL;
}

ServiceRunner::ServiceRunner(Thread::ThreadPriority priority)
: Thread(priority) {
  queue_ = NULL;
}

bool ServiceRunner::Start(ServiceQueue* queue) {
  queue_ = queue;
  return Thread::Start();
}

void ServiceRunner::Run() {
  Logger::Log(EVENT_NORMAL, "Thread [%X] is started to run.", this);

  while (true) {
    if (queue_ != NULL) {
      ServiceInterface* service = queue_->PopService();
      if (service != NULL) {
        Logger::Log(EVENT_NORMAL, "Thread [%X] is used to run service [%X].",
                  this, service);
        service->Run();
        queue_->CompleteService(service);
      }
    }

    WaitOrDie(1);
  }
}

