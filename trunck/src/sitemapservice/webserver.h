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


#ifndef SITEMAPSERVICE_WEBSERVER_H__
#define SITEMAPSERVICE_WEBSERVER_H__

#include <string>
#include <map>

#include "sitemapservice/activesocket.h"

class HttpProto;
class webserver {
public:
  ~webserver();
  // singleton factory
  static webserver* getInstance();

  typedef void (*request_func) (HttpProto*); 

  bool Start(unsigned int port_to_listen, request_func, bool singleThread);
  void ProcessRequest(const ActiveSocket& s);

private:
  // static member
  static webserver* instance_;

  // method
  bool StartListen(int port, int connections, bool isBlocking = true);
  ActiveSocket* Accept();

  // member
  SOCKET listen_socket_;
  request_func request_func_;
};
#endif
