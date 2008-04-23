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

#include "sitemapservice/securitymanager.h"

#include <Wincrypt.h>
#include <sstream>

#include "common/util.h"
#include "sitemapservice/httpproto.h"
#include "sitemapservice/sessionmanager.h"
#include "sitemapservice/settingmanager.h"

bool SecurityManager::SecurityCheck(HttpProto *r, SessionManager* sess,
                                    bool allowRemote) {
  // security check
  if (!CheckIp(r, allowRemote)){
    return false;
  }

  if (!sess->CheckSessionLogin(r)) {
    r->answer_status_ = "401 Unauthorized"; 
    r->answer_ = "Invalid session, you may do login first";
    return false;
  }
  return true;
}


bool SecurityManager::CheckIp(HttpProto *r, bool allowRemote) {
  // check if it is local
  if (r->remote_ip_ != "127.0.0.1") {
    // not local
    if (!allowRemote){
      Util::Log(EVENT_ERROR, "Remote access is not allowed!");
      r->answer_status_ = "401 Unauthorized"; 
      r->answer_ = "Remote access is not allowed!";
      return false;
    }
  }
  return true;
}

bool SecurityManager::VerifyPasswd(HttpProto *r, SettingManager* setting) {
  // get username and password
  std::string username = r->GetParam(HttpProto::kUsernameParamName);
  std::string password = r->GetParam(HttpProto::kPasswordParamName);


  // encrypt the input password  
  std::string encPassword;
  if (!Util::MD5Encrypt(password.c_str(), &encPassword)) {
    Util::Log(EVENT_ERROR, "can not encrypt password for login"); 
    return false;
  }

  return setting->Login(username, encPassword);
}


bool SecurityManager::CheckPath(HttpProto* r) {
  std::string::size_type pos = r->path_.find("..");
  if (pos != std::string::npos) {
    // find invalid path
    return false;
  }
  return true;
}

std::string SecurityManager::GenerateSimpleRandomId(SettingManager* setting) {
  srand((unsigned)time(NULL));

  int id = rand();
  std::ostringstream ostr;
  ostr << id;
  ostr << setting->GetPassword();
  std::string sid = ostr.str();

  // session id is MD5(randomValue + MD5(password))
  if(!Util::MD5Encrypt(sid.c_str(), &sid)) {
    Util::Log(EVENT_ERROR, "do MD5Encrypt failed");
  }

  return sid;
}

std::string SecurityManager::GenerateRandomId(SettingManager* setting) {
#ifdef WIN32
  HCRYPTPROV   hCryptProv;
  BYTE         pbData[16];
  std::string  sid = "";

  do {
    //  Acquire a cryptographic provider context handle.
    if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) {    
      DLog(EVENT_IMPORTANT, "CryptAcquireContext succeeded. \n");    
    } else {
      Util::Log(EVENT_ERROR, "Error during CryptAcquireContext!\n");
      break;
    }

    // Generate a random initialization vector.
    if(CryptGenRandom(hCryptProv, 8, pbData)) {
      DLog(EVENT_IMPORTANT, "Random sequence generated. \n");   
    } else {
      Util::Log(EVENT_ERROR, "Error during CryptGenRandom.\n");
      break;
    }

    // Encrypt the vector
    pbData[8] = 0;
    if(!Util::MD5Encrypt((const char*)pbData, &sid)) {
      Util::Log(EVENT_ERROR, "do MD5Encrypt failed");
      break;
    }
  } while(false);

  // release resource
  if(hCryptProv) {
    if(!(CryptReleaseContext(hCryptProv,0)))
      Util::Log(EVENT_ERROR, "Error during CryptReleaseContext");
  }

  // return result
  return sid.length() > 0 ? sid : GenerateSimpleRandomId(setting);


#else
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    Util::Log(EVENT_ERROR, "open /dev/urandom failed");
    return GenerateSimpleRandomId(setting);
  }

  char buf[16];
  int nbytes = 8;
  int ret = read(fd, buf, nbytes);
  if (ret < nbytes) { // fail to read
    Util::Log(EVENT_ERROR, "read /dev/urandom failed");
    return GenerateSimpleRandomId(setting);
  }

  buf[8] = 0;
  close(fd);

  std::string sid;
  if(!Util::MD5Encrypt(buf, &sid)) {
    Util::Log(EVENT_ERROR, "do MD5Encrypt failed");
    return GenerateSimpleRandomId(setting);
  }
  return sid;

#endif
}
