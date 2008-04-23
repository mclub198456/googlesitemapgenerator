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


#include "sitemapservice/activesocket.h"

#include <iostream>
#include <string>

#include "common/util.h"
#include "common/port.h"


int MyGetLastError() {
#ifdef WIN32
  return (int)GetLastError();
#else
  return errno;
#endif
}

int SetIoctl(SOCKET s, int cmd, int* arg) {
#ifdef WIN32
  return ioctlsocket(s, cmd, (u_long FAR*) arg);
#else
  return ioctl(s, cmd, (char*) arg);
#endif
}

void ReuseSocket(SOCKET sk) {
#ifdef WIN32
  char on = 1;
  if (SOCKET_ERROR == 
    setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
      Util::Log(EVENT_ERROR, "reuse socket error (%d)", MyGetLastError());
  }
#else
  int on = 1;
  if (SOCKET_ERROR == 
    setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on))) {
      Util::Log(EVENT_ERROR, "reuse socket error (%d)", MyGetLastError());
  }
#endif
}

///////////////////////////////////////////////////////

// constructor 
ActiveSocket::ActiveSocket(SOCKET s) : 
socket_(s), 
remote_addr_(NULL), 
ref_counter_(new int(1)) {

};

// destructor
ActiveSocket::~ActiveSocket() {
  if (! --(*ref_counter_)) {
    closesocket(socket_);
    delete ref_counter_;
    if (remote_addr_!=NULL) 
      delete remote_addr_;
  }
}

void ActiveSocket::DeepCopy(const ActiveSocket& o) {
  ref_counter_ = o.ref_counter_;
  (*ref_counter_)++;

  socket_ = o.socket_;
  remote_addr_ = o.remote_addr_;
}

ActiveSocket::ActiveSocket(const ActiveSocket& o) {
  DeepCopy(o);
}

ActiveSocket& ActiveSocket::operator=(const ActiveSocket& o) {
  (*ref_counter_)--;

  DeepCopy(o);

  return *this;
}


char* ActiveSocket::GetLocalIp() const {
  sockaddr_in local_addr;
#ifdef WIN32
  int local_addr_length = sizeof(local_addr);
#else
  socklen_t local_addr_length = sizeof(local_addr);
#endif

  if (getsockname(socket_, reinterpret_cast<sockaddr *>(&local_addr),
    &local_addr_length) != 0)
    return NULL;

  return inet_ntoa(local_addr.sin_addr);
}

char* ActiveSocket::GetRemoteIp() const{
  return remote_addr_ == NULL ? NULL : inet_ntoa(remote_addr_->sin_addr);
}


std::string ActiveSocket::ReceiveBytes(int reqLen) const {
  std::string ret;
  char buf[1024];

  while (true) {
    if (reqLen == -1) {
      int arg = 0;
      if (SetIoctl(socket_, FIONREAD, &arg) != 0)
        break;    

      if (arg == 0)
        break;

      if (arg > 1024) 
        arg = 1024;

      int rv = recv (socket_, buf, arg, 0);
      if (rv <= 0) {
        break;
      }

      ret.append(buf, rv);
    } else {
      while (reqLen > 0) {
        int rv = recv (socket_, buf, reqLen > 1024 ? 1024 : reqLen, 0);
        if (rv <= 0) {
          break;
        }

        ret.append(buf, rv);
        reqLen -= rv;
      }
      break;
    }
  }

  return ret;
}

// the string return without EOL
std::string ActiveSocket::ReceiveLine() const {
  std::string ret;
  ret.reserve(100);

  while (true) {
    char r;

    int res = recv(socket_, &r, 1, 0);
    if (res == 0) {
      return ret;
    } else if (res == -1) {
      return "";
    }

    if (r != '\x0d' && r != '\x0a')
      ret.push_back(r);

    // in Windows, it's 0d0a, in linux, it's 0a (\n)
    if (r == '\x0a') {
      return ret;
    }
  }
}

int ActiveSocket::SendLine(const std::string& s) const {
  std::string line = s + '\n';
  return send(socket_, line.c_str(), (int)line.length(), 0);
}

int ActiveSocket::SendBytes(const char * buffer, int length) const {
  return send(socket_, buffer, length, 0);
}
