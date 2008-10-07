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

// CGI handler main file.

#include "common/util.h"
#include "common/logger.h"
#include "common/fileutil.h"
#include "common/httprequest.h"
#include "common/httpresponse.h"
#include "common/interproclock.h"
#include "common/messagepipe.h"
#include "common/httpconst.h"

#include "admin_console_cgi/cgihandler.h"

// Utility method.
// Log given error and send it to CGI.
void Error(const std::string& message) {
  Logger::Log(EVENT_ERROR, "%s", message.c_str());
  HttpResponse response;
  response.Reset(HttpConst::kStatus503, message);
  if (!CgiHandler::WriteResponse(response)) {
    Logger::Log(EVENT_ERROR, "CGI failed to write error response.");
  }
}

int main(int argc, char* argv[]) {
#ifdef WIN32
  // Get application directory.
  std::string app_dir;
  if (!Util::GetModuleDir(NULL, &app_dir)) {
    Error("Failed to get CGI dir.");
    return 1;
  }
  if (strncmp(app_dir.c_str(), "\\\\?\\", 4) == 0) {
    app_dir.erase(0, 4);
  }

  if (!FileUtil::ParentDir(&app_dir) || !FileUtil::ParentDir(&app_dir)) {
    Error("Malformed CGI dir 2.");
    return 1;
  }
  Util::SetApplicationDir(app_dir);

  // Set log file path.
  std::string log_path = app_dir;
  log_path.append("\\google-sitemap-generator.log");
  Logger::SetLogFileName(log_path);
#else
  // Get application directory.
  std::string path = Util::ReadApplicationDir();
  if (path.length() == 0) {
    Error("Failed to read app dir.");
    return 0;
  }
  Util::SetApplicationDir(path);

  // Set log file path.
  path.append("/log/google-sitemap-generator.log");
  Logger::SetLogFileName(path);
#endif

  // Only one CGI client is allowed at one time to communicate with server.
  MessagePipe pipe(false);
  if (!pipe.Lock(3000)) {
    Error("Failed to lock admin console cgi.");
    return 1;
  }

  int result = 1;
  do {
    HttpRequest request;
    if (!CgiHandler::ReadRequest(&request)) {
      Error("Failed to read request from cgi.");
      break;
    }
    std::string message;
    request.ToString(&message);

    if (!pipe.Initialize()) {
      Error("Failed to init message pipe in cgi.");
      break;
    }

    if (!pipe.Send(message)) {
      Error("CGI failed to send message.");
      break;
    }

    message.assign("");
    if (!pipe.Receive(&message)) {
      Error("CGI failed to receive message.");
      break;
    }

    HttpResponse response;
    if (!response.FromString(message)) {
      Error("CGI failed to parse response message.");
      break;
    } 

    if (!CgiHandler::WriteResponse(response)) {
      Error("CGI failed to write response.");
      break;
    }

    result = 0;
  } while (false);

  pipe.Unlock();

  return result;
}
