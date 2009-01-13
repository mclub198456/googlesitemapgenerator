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


#include "sitemapservice/httpgetter.h"

#ifdef WIN32
#include <windows.h>
#include <winhttp.h>
#elif defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#endif

#include "common/logger.h"
#include "common/util.h"

HttpGetter::HttpGetter() {
  status_ = -1;
}

#ifdef WIN32

bool HttpGetter::Get(const char* host, int port, const char *path) {

  BOOL  result = FALSE;
  HINTERNET  session_handle = NULL,
             connect_handle = NULL,
             request_handle = NULL;

  // Use WinHttpOpen to obtain a session handle.
  session_handle = WinHttpOpen(L"Simple Http Getter",
                               WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                               WINHTTP_NO_PROXY_NAME,
                               WINHTTP_NO_PROXY_BYPASS, 0);

  // Specify an HTTP server.
  if (session_handle != NULL) {
    std::wstring wide_host;
    if (Util::MultiByteToWideChar(host, &wide_host)) {
      connect_handle = WinHttpConnect(session_handle,
                                      wide_host.c_str(),
                                      port,
                                      0);
    }
  }

  // Create an HTTP request handle.
  if (connect_handle != NULL) {
    std::wstring wide_path;
    if (Util::MultiByteToWideChar(path, &wide_path)) {
      request_handle = WinHttpOpenRequest(connect_handle, L"GET",
                                          wide_path.c_str(),
                                          NULL, WINHTTP_NO_REFERER,
                                          WINHTTP_DEFAULT_ACCEPT_TYPES,
                                          0);
    }
  }

  // Send a request.
  if (request_handle != NULL) {
    result = WinHttpSendRequest(request_handle,
                                WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0,
                                0, 0 );
  }

  // End the request.
  if (result != FALSE) {
    result = WinHttpReceiveResponse(request_handle, NULL);
  }

  // Get response status code.
  if (result) {
    WCHAR status_buffer[16];
    DWORD status_size = 16;
    if (!WinHttpQueryHeaders(request_handle, WINHTTP_QUERY_STATUS_CODE,
     WINHTTP_HEADER_NAME_BY_INDEX, status_buffer, &status_size,
     WINHTTP_NO_HEADER_INDEX)) {
     result = FALSE;
    }

    std::string status_str;
    Util::WideCharToMultiByte(status_buffer, &status_str);
    status_ = atoi(status_str.c_str());
  }

  // Get response content.
  if (result) {
    content_.clear();
    char buffer[1024];
    DWORD read_size = 0;
    do {
      if (!WinHttpReadData(request_handle, buffer, 1024, &read_size)) {
        result = FALSE;
        break;
      }

      // Save buffer data to content.
      content_.append(buffer, buffer + read_size);
      if (content_.length() > kMaxContentLength) {
        break;
      }

    } while (read_size > 0);
  }

  // Close any open handles.
  if(request_handle != NULL) WinHttpCloseHandle(request_handle);
  if(connect_handle != NULL) WinHttpCloseHandle(connect_handle);
  if(session_handle != NULL) WinHttpCloseHandle(session_handle);

  if (result == FALSE) {
    Logger::Log(EVENT_IMPORTANT, "Fail to do http get.");
  }

  // Error code. See: Error Messages:
  // http://msdn2.microsoft.com/en-us/library/aa385465.aspx
  return result != FALSE;
}

#else

bool HttpGetter::Get(const char* host, int port, const char *path) {
  int result = 0;

  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    Logger::Log(EVENT_ERROR, "Can't create socket.");
    return errno;
  }

  while (true) {
    // Set the timeout values.
    struct timeval timeout;
    timeout.tv_sec = kTimeOut;
    timeout.tv_usec = 0;
    if (setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO,
                   &timeout, sizeof(timeout)) != 0) {
      result = errno;
      break;
    }
    if (setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) != 0) {
      result = errno;
      break;
    }

    hostent* server = gethostbyname(host);
    if (server == NULL) {
      result = h_errno;
      break;
    }

    // Build the server address.
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    memcpy(&address.sin_addr.s_addr, server->h_addr, server->h_length);

    // Connect to server.
    sockaddr* addr = reinterpret_cast<sockaddr*>(&address);
    if (connect(socketfd, addr, sizeof(address)) < 0) {
      result = errno;
      break;
    }

    // Send http request to server.
    if (strlen(path) + strlen(host) > 1024) {
      result = -1;
      break;
    }
    char buffer[2048];
    sprintf(buffer,
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Accept: text/html, text/plain, text/*\r\n"
            "\r\n", path, host);

    if (send(socketfd, buffer, strlen(buffer), 0) < 0) {
      result = errno;
      break;
    }

    // Receive response.
    bool header_found = false;
    status_ = -1;
    content_.clear();
    memset(buffer, 0, sizeof(buffer));
    int recv_result = -1, content_length = -1;
    while ((recv_result = recv(socketfd, buffer, 1024, 0)) > 0) {
      content_.append(buffer, buffer + recv_result);

      // Parse the header.
      std::string::size_type text_pos = std::string::npos;
      int split_length = 4;
      if (header_found == false) {
        text_pos = content_.find("\r\n\r\n");

        // Accommodate to non-standard behavior.
        if (text_pos == std::string::npos) {
          text_pos = content_.find("\n\n");
          split_length = 2;
        }
      }

      if (text_pos != std::string::npos) {
        header_found = true;
        std::string header = content_.substr(0, text_pos);
        content_.erase(0, text_pos + split_length);

        // Pasrse status code.
        if (header.length() < 12) {
          status_ = -1;
        } else {
          // "HTTP/1.0 200 ..."
          status_ = atoi(header.substr(9, 3).c_str());
        }

        // Parse Content-Length.
        std::string::size_type pos = header.find("Content-Length:");
        if (pos != std::string::npos) {
          content_length = 0;
          pos += strlen("Content-Length:");
          while (pos < header.length() && header[pos] == ' ') {
            ++pos;
          }
          while (pos < header.length() && isdigit(header[pos])) {
            content_length = content_length * 10 + header[pos] - '0';
            ++pos;
          }

          if (content_length > kMaxContentLength || content_length < 0) {
            content_length = kMaxContentLength;
          }
        }
      }

      if (content_length >= kMaxContentLength ||
          (content_length != -1 && content_.length() >= content_length)) {
        break;
      }
    }

    if (recv_result < 0) {
      result = errno;
      break;
    }

    break;
  }

  shutdown(socketfd, SHUT_RDWR);
  close(socketfd);

  if (result != 0) {
    Logger::Log(EVENT_ERROR, "Failed to do http Get. (%d)", result);
    return false;
  } else {
    return true;
  }
}

#endif
