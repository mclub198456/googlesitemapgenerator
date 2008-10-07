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


#include "sitemapservice/adminconsolethread.h"

#include "common/httpconst.h"
#include "common/logger.h"
#include "sitemapservice/runtimeinfomanager.h"

bool AdminConsoleThread::Initialize() {
  // Initialize Url Pipe.
  if (!pipe_.Initialize()) {
    Logger::Log(EVENT_ERROR, "Failed to create pipe for admin console thread.");
    return false;
  }

  if (!pipe_.ResetMutex()) {
    Logger::Log(EVENT_ERROR, "Failed to reset mutex.");
    return false;
  }

  pagecontroller_ = PageController::instance();

  return true;
}

void AdminConsoleThread::Run() {
  while (true) {
    if (!RunOnce()) {
      // The pipe has some problem, fix it.
      while (!pipe_.ResetMutex()) {
        Logger::Log(EVENT_ERROR, "Failed to reset message pipe.");
        WaitOrDie(1000 * 60);
      }
    }
    WaitOrDie(0);
  }
}

bool AdminConsoleThread::RunOnce() {
  // Receive http request message.
  std::string message;
  if (!pipe_.Receive(&message)) {
    Logger::Log(EVENT_ERROR, "Failed to receive message from admin console.");
    return false;
  }

  // Parse message to http request.
  HttpResponse response;
  HttpRequest request;
  if (!request.FromString(message)) {
    Logger::Log(EVENT_ERROR, "Failed to parse request from string.");
    response.Reset(HttpConst::kStatus503, "Failed to parse request string.");
  } else {
    // Construct http context.
    HttpContext context;
    context.Initialize(&request, &response);

    // Process http context.
    pagecontroller_->Process(&context);
  }

  // Convert http response to message.
  message.assign("");
  response.ToString(&message);

  // Send message.
  if (!pipe_.Send(message)) {
    Logger::Log(EVENT_ERROR, "Failed to send message to admin console.");
    return false;
  }

  return true;
}
