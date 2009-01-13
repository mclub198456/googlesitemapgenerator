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


// This interface defines the contract that should be followed by all kinds
// of services. A service must contain a Run method to do the job. And it
// should be provide the running period value to let service scheduler to
// schedule services.

#ifndef SITEMAPSERVICE_SERVICEINTERFACE_H__
#define SITEMAPSERVICE_SERVICEINTERFACE_H__

class ServiceInterface {
 public:
  ServiceInterface() {}
  virtual ~ServiceInterface() {}

  // Get waiting time to run service next time.
  // A non-positive value indicates the service should be runned immediately.
  virtual int GetWaitTime() = 0;

  // Get running period of service.
  virtual int GetRunningPeriod() = 0;

  // Run the service.
  virtual void Run() = 0;
};

#endif // SITEMAPSERVICE_SERVICEINTERFACE_H__

