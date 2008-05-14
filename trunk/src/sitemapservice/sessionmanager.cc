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

#include "sitemapservice/sessionmanager.h"

#include "sitemapservice/pagecontroller.h"
#include "sitemapservice/httpproto.h"
#include "sitemapservice/securitymanager.h"

const unsigned int SessionManager::kMaxSessionNum = 16;
const unsigned int SessionManager::kSessionExpireTime = 30*60; // secs, 30 mins

bool SessionManager::CreateSession(const std::string& seed,
                                   std::string* session_id) {
  // Max times of trying to create new session id.
  static const int kMaxTryTimes = 100;

  if (IsFull()) {
    return false;
  }

  for (int i = kMaxTryTimes; i >= 0; --i) {
    std::string temp_id = SecurityManager::GenerateRandomId(seed);

    if (sessions_.find(temp_id) == sessions_.end()) {
      SessionInfo* session = new SessionInfo;
      session->session_id = temp_id;
      time(&session->last_access);

      session_id->assign(temp_id);
      sessions_[temp_id] = session;
      return true;
    }
  }

  return false;
}

SessionManager::SessionInfo* SessionManager::GetSession(HttpProto *r) {
  std::string session_id = r->GetParam(HttpProto::kSessionIdParamName);
  RemoveExpireSession();
  HashTable::const_iterator it = sessions_.find(session_id);
  if (it != sessions_.end()) {
    DLog(EVENT_IMPORTANT, "access session %s", session_id.c_str()); 
    it->second->last_access = time(NULL);
    return it->second;
  }
  Util::Log(EVENT_ERROR, "!!invalid session %s", 
            Util::EscapeLogMessage(session_id.c_str()).c_str()); 
  return NULL;
}

void SessionManager::RemoveExpireSession() {
  HashTable::iterator it = sessions_.begin();
  while (it != sessions_.end()) {
    if (time(NULL) - it->second->last_access > kSessionExpireTime) {
      delete it->second;
      sessions_.erase(it);
      it = sessions_.begin();
    } else {
      ++it;
    }
  }
}

bool SessionManager::CheckSessionLogin(HttpProto* r) {
  SessionInfo* sess = GetSession(r);
  return sess != NULL;
}

bool SessionManager::IsFull() {
  RemoveExpireSession();
  return sessions_.size() >= kMaxSessionNum;
}

void SessionManager::RemoveSession(const std::string& id) {
  HashTable::iterator it = sessions_.find(id);
  if (it != sessions_.end()) {
    delete it->second;
    sessions_.erase(it);
  }
}

