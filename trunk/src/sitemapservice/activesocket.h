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


#ifndef SITEMAPSERVICE_SOCKET_H__
#define SITEMAPSERVICE_SOCKET_H__

#ifdef WIN32

#include <WinSock2.h>
#define EWOULDBLOCK WSAEWOULDBLOCK
#define socklen_t int

#else // Linux

#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/ioctl.h>//linux
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <fcntl.h>

#define SOCKET int

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

// follow windows function name.
inline void closesocket(int socket) {
  close(socket);
}

#endif 

#include <string>

// cross-platform functions

// Returns error number
int MyGetLastError();

// set socket ioctl
int SetIoctl(SOCKET s, int cmd, int* arg);

// Reuses the socket
void ReuseSocket(SOCKET sk);

class ActiveSocket{
public:
  // constructor
  ActiveSocket(SOCKET s);

  // destructor
  virtual ~ActiveSocket();

  // copy constructor
  ActiveSocket(const ActiveSocket&);

  // assign operator override
  ActiveSocket& operator=(const ActiveSocket&);

  // Receives a line from client
  std::string ReceiveLine() const;

  // Receives the number of bytes from client
  std::string ReceiveBytes(int len = -1) const;

  // Sends a line to client
  int SendLine(const std::string&) const;

  // Sends the number of bytes to client
  int SendBytes(const char * buffer, int length) const;

  // Returns the string of remote IP
  char* GetRemoteIp() const;

  // Returns the string of local IP
  char* GetLocalIp() const;

  // Sets remote client socket address
  void SetClientSockAddr(sockaddr_in* addr) {remote_addr_ = addr;}

private:
  // Copy the member of the object
  void DeepCopy(const ActiveSocket& o);

  ActiveSocket(); // default constructor

  SOCKET socket_; // The socket that connects to remote client
  sockaddr_in* remote_addr_; // The remote client's address
  int* ref_counter_;  // the reference count of the object
};
#endif
