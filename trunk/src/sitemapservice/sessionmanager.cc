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

#include "common/logger.h"
#include "sitemapservice/securitymanager.h"

SessionTable::SessionTable(unsigned int max_num, unsigned int expire_time) : 
max_session_num_(max_num), 
session_expire_time_(expire_time) {  
}

SessionTable::~SessionTable() {
  HashTable::iterator it = sessions_.begin();
  while (it != sessions_.end()) {
    delete it->second;
    sessions_.erase(it);
    it = sessions_.begin();
  }
}

bool SessionTable::IsFull() {
  return sessions_.size() >= max_session_num_;
}

void SessionTable::RemoveExpireSession() {
  time_t now = time(NULL);
  HashTable::iterator it = sessions_.begin();
  while (it != sessions_.end()) {
    if (now - it->second->last_access > session_expire_time_) {
      delete it->second;
      sessions_.erase(it);
      it = sessions_.begin();
    } else {
      ++it;
    }
  }
}

void SessionTable::AddSession(const std::string& id) {
  SessionInfo* session = new SessionInfo;
  session->session_id = id;
  time(&session->last_access);
  sessions_[id] = session;
}

void SessionTable::AddSession(const std::string& id, 
                              SessionTable* another_table) {
  SessionInfo* session = another_table->RemoveSession(id);
  time(&session->last_access);
  sessions_[id] = session;
}

void SessionTable::DeleteSession(const std::string& id) {
  HashTable::iterator it = sessions_.find(id);
  if (it != sessions_.end()) {
    delete it->second;
    sessions_.erase(it);
  }
}

bool SessionTable::Exist(const std::string& id) {
  HashTable::const_iterator it = sessions_.find(id);
  if (it != sessions_.end()) {
    time(&it->second->last_access);
  }
  return it != sessions_.end();
}

SessionTable::SessionInfo* SessionTable::RemoveSession(const std::string& id) {
  HashTable::iterator it = sessions_.find(id);
  if (it != sessions_.end()) {
    SessionInfo* session = it->second;
    sessions_.erase(it);
    return session;
  }
  return NULL;
}

SessionManager::SessionManager() {
  login_sessions_ = new SessionTable(16, 30*60);
  fresh_sessions_ = new SessionTable(1000, 5*60);
}

SessionManager::~SessionManager() {
  delete login_sessions_;
  delete fresh_sessions_;
}

bool SessionManager::CreateSession(const std::string& seed,
                                   std::string* session_id) {
  // Max times of trying to create new session id that is not the same as
  // existed session ids.
  static const int kMaxTryTimes = 100;

  if (fresh_sessions_->IsFull()) {
    return false;
  }

  for (int i = kMaxTryTimes; i >= 0; --i) {
    std::string temp_id = SecurityManager::GenerateRandomId(seed);

    if (!Exist(temp_id)) {
      fresh_sessions_->AddSession(temp_id);
      session_id->assign(temp_id);
      return true;
    }
  }

  return false;
}

bool SessionManager::CheckSessionLogin(const std::string& id) {
  return login_sessions_->Exist(id);
}

bool SessionManager::Login(const std::string& id) {
  if (login_sessions_->IsFull()) {
    Logger::Log(EVENT_ERROR, "Login failed!! Reach the maximum sessions");
    return false;
  }

  if (!fresh_sessions_->Exist(id)) {
    if (login_sessions_->Exist(id)) {
      return true;      
    } else {
      Logger::Log(EVENT_ERROR, "Login failed!! Invalid session id");
      return false;
    }
  }

  login_sessions_->AddSession(id, fresh_sessions_);
  return true;
}

void SessionManager::RemoveSession(const std::string& id) {
  login_sessions_->DeleteSession(id);
  fresh_sessions_->DeleteSession(id);
}

bool SessionManager::Exist(const std::string& id) {
  return login_sessions_->Exist(id) || fresh_sessions_->Exist(id);
}

bool SessionManager::IsFull() {
  return login_sessions_->IsFull() || fresh_sessions_->IsFull();
}

void SessionManager::RemoveExpireSessions() {
  login_sessions_->RemoveExpireSession();
  fresh_sessions_->RemoveExpireSession();
}

