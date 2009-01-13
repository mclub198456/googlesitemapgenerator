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

#include "sitemapservice/securitymanager.h"

#ifdef WIN32
#include <Wincrypt.h>
#include <Winsock2.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <sstream>

#include "common/logger.h"
#include "common/util.h"
#include "common/httpconst.h"
#include "sitemapservice/httpsettingmanager.h"
#include "sitemapservice/sessionmanager.h"

SecurityManager::~SecurityManager() {
  // does nothing
}

bool SecurityManager::Initialize(HttpSettingManager* setting_manager,
                                 SessionManager* session_manager) {
  setting_manager_ = setting_manager;
  session_manager_ = session_manager;

  // Get local ip address.
#ifdef WIN32
  WSAData wsadata;
  if (WSAStartup(MAKEWORD(1, 1), &wsadata) == 0) {
#endif
    // get host name
    char buffer[1024];
    if (gethostname(buffer, 1024) != 0) {
      Logger::Log(EVENT_ERROR, "Failed to get host name in secure manager.");
      buffer[0] = '\0';
    }

    // get ip address
   if (buffer[0] != '\0') {
     hostent* addr = gethostbyname(buffer);
     if (addr != NULL) {
       for (int i = 0; addr->h_addr_list[i] != NULL; ++i) {
         char* ip = inet_ntoa(*reinterpret_cast<in_addr*>(addr->h_addr_list[i]));
         if (ip != NULL) {
           local_addrs_.insert(ip);
         }
       }
     } else {
       Logger::Log(EVENT_ERROR, "Failed to get host by name in secure manager.");
     }
   }
#ifdef WIN32
   WSACleanup();
  }
#endif

  local_addrs_.insert("127.0.0.1");
  local_addrs_.insert("::1"); // Very limit support for IPv6.
  return true;
}

bool SecurityManager::CheckRemoteClient(const HttpRequest& request) {
  if (local_addrs_.find(request.remote_addr()) == local_addrs_.end()) {
    // Check if it is local.
    // Note, attacker can pass this check if local machine is used as proxy.
    if (!setting_manager_->AllowRemoteAccess()){
      Logger::Log(EVENT_ERROR, "Remove client [%s] not allowd.",
                request.remote_addr().c_str());
      return false;
    }

#ifndef GSG_LOW_SECURITY
    // Check if SSL is used.
    if (request.https() != HttpRequest::kHttpsOn) {
      Logger::Log(EVENT_ERROR, "Remote connection is unsecured.");
      return false;
    }
#endif // GSG_LOW_SECURITY
  }
  return true;
}

void SecurityManager::RegisterPrivatePage(const std::string& path) {
  private_pages_.insert(path);
}

bool SecurityManager::Check(HttpContext* context) {
  HttpRequest* request = context->request();
  HttpResponse* response = context->response();

  // Exclude malicious path.
  if (!CheckPath(context->action())) {
    response->Reset(HttpConst::kStatus404, "Path not allowed."); 
    return false;
  }

  // Exclude remote client if not allowed.
  if (!CheckRemoteClient(*request)) {
    response->set_status(HttpConst::kStatus200);
    response->SetHeader(HttpConst::kHeaderContentType, "text/html; charset=utf-8");
    response->set_message_body(
      "<html><head>"
      "<meta http-equiv=\"Refresh\" content=\"0; url=/access_error.html\" />"
      "</head></body>"
      "Please follow <a href=\"/access_error.html\">link</a>."
      "</body></html>"
      );
    return false;
  }

  // Save the state whether this logged in or not.
  bool authenticated = session_manager_->CheckSessionLogin(context->session_id());
  context->set_need_login(!authenticated); 

  // Access control for private pages.
  if (private_pages_.find(context->action()) != private_pages_.end()) {
    // User should be logged in.
    if (!authenticated) {
      response->Reset(HttpConst::kStatus401, "Not logged in.");
      return false;
    }

    // Avoid XSRF attack.
    if (context->session_id() != context->GetParam(HttpContext::kSIDParamName)) {
      response->Reset(HttpConst::kStatus401, "Unexpected URL.");
      return false;
    }
  }

  return true;
}

bool SecurityManager::CheckPath(const std::string& path) {
  std::string::size_type pos = path.find("..");
  if (pos != std::string::npos) {
    // find invalid path
    return false;
  }
  return true;
}

std::string SecurityManager::GenerateSimpleRandomId(const std::string& seed) {
  srand((unsigned)time(NULL));

  int id = rand();
  std::ostringstream ostr;
  ostr << id;
  ostr << seed;
  std::string sid = ostr.str();

  // session id is MD5(randomValue + MD5(password))
  if(!Util::MD5Encrypt(sid.c_str(), &sid)) {
    Logger::Log(EVENT_ERROR, "do MD5Encrypt failed");
  }

  return sid;
}

std::string SecurityManager::GenerateRandomId(const std::string& seed) {
#ifdef WIN32
  HCRYPTPROV   h_crypt_prov;
  BYTE         pb_data[16];
  std::string  sid = "";

  do {
    //  Acquire a cryptographic provider context handle.
    if(CryptAcquireContext(&h_crypt_prov, NULL, NULL, PROV_RSA_FULL, 0)) {    
      DLog(EVENT_IMPORTANT, "CryptAcquireContext succeeded. \n");    
    } else {
      Logger::Log(EVENT_ERROR, "Error during CryptAcquireContext!\n");
      break;
    }

    // Generate a random initialization vector.
    if(CryptGenRandom(h_crypt_prov, 8, pb_data)) {
      DLog(EVENT_IMPORTANT, "Random sequence generated. \n");   
    } else {
      Logger::Log(EVENT_ERROR, "Error during CryptGenRandom.\n");
      break;
    }

    // Encrypt the vector
    pb_data[8] = 0;
    if(!Util::MD5Encrypt((const char*)pb_data, &sid)) {
      Logger::Log(EVENT_ERROR, "do MD5Encrypt failed");
      break;
    }
  } while(false);

  // release resource
  if(h_crypt_prov) {
    if(!(CryptReleaseContext(h_crypt_prov,0)))
      Logger::Log(EVENT_ERROR, "Error during CryptReleaseContext");
  }

  // return result
  return sid.length() > 0 ? sid : GenerateSimpleRandomId(seed);


#else
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    Logger::Log(EVENT_ERROR, "open /dev/urandom failed");
    return GenerateSimpleRandomId(seed);
  }

  char buf[16];
  int nbytes = 8;
  int ret = read(fd, buf, nbytes);
  if (ret < nbytes) { // fail to read
    Logger::Log(EVENT_ERROR, "read /dev/urandom failed");
    return GenerateSimpleRandomId(seed);
  }

  buf[8] = 0;
  close(fd);

  std::string sid;
  if(!Util::MD5Encrypt(buf, &sid)) {
    Logger::Log(EVENT_ERROR, "do MD5Encrypt failed");
    return GenerateSimpleRandomId(seed);
  }
  return sid;

#endif
}
