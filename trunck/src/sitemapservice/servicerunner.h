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


// This file defines two classes: ServiceRunner and ServicePool.
// A ServicePool contains many services, and divides the services into
// three categories according to their statuses. They are waiting status,
// sleeping status and running status.
// A ServiceRunner is a thread which can run services. It can run different
// services sequentially but not in parallel. When it is free, it checks the
// ServicePool periodically to see whether any service is in waiting status,
// and ready to run.

#ifndef SITEMAPSERVICE_SERVICERUNNER_H__
#define SITEMAPSERVICE_SERVICERUNNER_H__

#include <list>
#include <set>

#include "common/thread.h"
#include "common/criticalsection.h"
#include "sitemapservice/serviceinterface.h"


class ServicePool {
 public:
  ServicePool();
  ~ServicePool();

  // Get a service ready to run.
  // If there is no service is available, NULL is returned.
  ServiceInterface* GetService();

  // Return a service which is completed.
  // This service was retrieved via GetService method before.
  void ReturnService(ServiceInterface* service);

  // Scheduling the services.
  // Returns the next scheduling time.
  int ScheduleService();

 private:
  // Services in waiting status.
  std::list<ServiceInterface*> waiting_services_;
  // Services in sleeping status.
  std::list<ServiceInterface*> sleeping_services_;
  // Services in running status.
  std::set<ServiceInterface*> running_services_;

  // Critical section used to protect accessing to services.
  CriticalSection lock_;
};

class ServiceRunner : public Thread {
public:
  // By default, the service is runned in a low priority.
  ServiceRunner();

  ServiceRunner(Thread::ThreadPriority priority);

  virtual ~ServiceRunner() {}

  // Start this runner.
  // This runner will poll the given "pool" periodically to see if any service
  // is available, and runs the available services.
  virtual bool Start(ServicePool* pool);

protected:
  // Run services.
  virtual void Run();
  
private:
  // "pool" where the services are retrieved.
  ServicePool* pool_;
};

#endif // SITEMAPSERVICE_SERVICERUNNER_H__

