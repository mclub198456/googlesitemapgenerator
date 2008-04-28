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

const unsigned int SessionManager::kMaxSessionNum = 16;
const unsigned int SessionManager::kSessionExpireTime = 30*60; // secs, 30 mins

void SessionManager::CreateSession(const std::string& session_id) {
  SessionInfo* sess = new SessionInfo;
  sess->session_id = session_id;
  sess->last_access = time(NULL);

  sessions_[session_id] = sess;
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

bool SessionManager::isFull() {
  return sessions_.size() >= kMaxSessionNum;
}

void SessionManager::RemoveSession(const std::string& id) {
  HashTable::iterator it = sessions_.find(id);
  if (it != sessions_.end()) {
    delete it->second;
    sessions_.erase(it);
  }
}

