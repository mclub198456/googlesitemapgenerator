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

#include "sitemapservice/webserver.h"

#include <ctime>
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <fstream>

#include "common/util.h"
#include "common/port.h"
#include "sitemapservice/httpproto.h"

webserver* webserver::instance_ = NULL;
webserver* webserver::getInstance() { 
  if (instance_ == NULL) 
    instance_ = new webserver();
  return instance_; 
}
/////////////////////// thread function /////////////////////////////

void* ProcessRequestThread(void* param) {
  ActiveSocket* sock = reinterpret_cast<ActiveSocket*>(param);
  webserver::getInstance()->ProcessRequest(*sock);
  return 0;
}

void webserver::ProcessRequest(const ActiveSocket& s) {

  HttpProto http;
  http.ResetRequest();
  http.ProcessRequest(s);

  http.ResetResponse();
  request_func_(&http);  
  http.ProcessResponse(s);
}

webserver::~webserver() {
#ifdef WIN32
  WSACleanup();
#endif
}

bool webserver::Start(unsigned int port_to_listen, request_func r,
                      bool singleThread) {
#ifdef WIN32
  WSADATA info;  
  if (WSAStartup(MAKEWORD(2,0), &info)) {
    Util::Log(EVENT_ERROR, "fail to startup WSA (%d).", MyGetLastError());
    return false;
  }
#endif

  if (!StartListen(port_to_listen,5)) {
    Util::Log(EVENT_ERROR, "fail to listen (%d).", MyGetLastError());
    return false;
  }

  request_func_ = r;

  while (1) {
    ActiveSocket* sock = Accept();
    if (sock == NULL) {
      Util::Log(EVENT_ERROR, "accept failed (%d).", MyGetLastError());
      return false;
    }

    if (singleThread) {
      ProcessRequest(*sock);
    } else {
      bool failed;
#ifdef WIN32
      DWORD dwThreadId;
      failed = CreateThread(NULL, 0, 
                           (LPTHREAD_START_ROUTINE )ProcessRequestThread,
                           (void*)sock, 0, &dwThreadId) == INVALID_HANDLE_VALUE;
#else
      pthread_t th_head;
      failed = pthread_create(&th_head, NULL, 
                              &ProcessRequestThread,(void*)sock) != 0;
#endif
      if (failed) {
        Util::Log(EVENT_ERROR, 
                  "fail to create thread to process webserver request (%d).", 
                  MyGetLastError());
        return false;
      }
    }
  }
  return true;
}



bool webserver::StartListen(int port, int connections, bool isBlocking) {
  sockaddr_in sa;

  memset(&sa, 0, sizeof(sa));

  sa.sin_family = PF_INET;             
  sa.sin_port = htons(port);          
  listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket_ == INVALID_SOCKET) {
    return false;
  }

  if(!isBlocking) {
    int arg = 1;
    SetIoctl(listen_socket_, FIONBIO, &arg);
  }

#ifndef WIN32
  ReuseSocket(listen_socket_);
  if (fcntl(listen_socket_, F_SETFD, FD_CLOEXEC)) {
    // log error
  }
#endif

  if (bind(listen_socket_, (sockaddr *)&sa, sizeof(sockaddr_in)) 
      == SOCKET_ERROR) {
    closesocket(listen_socket_);
    return false;
  }

  int ret = listen(listen_socket_, connections);                    
  return ret == -1 ? false : true;
}

ActiveSocket* webserver::Accept() {
  sockaddr_in* addr = new sockaddr_in();
  socklen_t sockaddr_len = sizeof(struct sockaddr_in);
  SOCKET new_sock = accept(listen_socket_, (struct sockaddr*)addr, 
                           &sockaddr_len);
  if (new_sock == INVALID_SOCKET) {
    int rc = MyGetLastError();
    if(rc==EWOULDBLOCK) {
      return 0;
    } else {
      return NULL;
    }
  }

  ActiveSocket* r = new ActiveSocket(new_sock);
  r->SetSockAddr(addr);
  return r;
}
