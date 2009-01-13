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

// This class is a thread used to interact with Admin Console CGI exe.
// It receives request message from CGI exe, and sends reponse message back
// to CGI exe after processing.

#ifndef SITEMAPSERVICE_ADMINCONSOLETHREAD_H__
#define SITEMAPSERVICE_ADMINCONSOLETHREAD_H__

#include "common/thread.h"
#include "common/messagepipe.h"
#include "sitemapservice/pagecontroller.h"

class AdminConsoleThread : public Thread {
 public:
  AdminConsoleThread() : pipe_(true) {}
  virtual ~AdminConsoleThread() {}

  // Initialize the thread.
  bool Initialize();

  // Run this thread.
  virtual void Run();

 private:
  bool RunOnce();

  // Message pipe used to transport message between this thread and CGI exe.
  MessagePipe pipe_;

  // Page controller used to handling request.
  PageController* pagecontroller_;
};

#endif // SITEMAPSERVICE_ADMINCONSOLETHREAD_H__
