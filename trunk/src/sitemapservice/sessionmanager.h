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

// This file defines two classes to manage the user session. The user session
// object currently only record the session id for authorization and access
// time to remove expired sessions. It also limits the max number of sessions.
// The SessionTable class is to manage maximum limitation for session table
// and expire time for each session in the table.
// The SessionManager class takes care of two session tables, one for sessions 
// that just created, one for sessions that have login. It provides interfaces
// for new session creation, session login, and session look-up by session id.

#ifndef SITEMAPSERVICE_SESSIONMANAGER_H__
#define SITEMAPSERVICE_SESSIONMANAGER_H__

#include "common/hashmap.h"

class SessionTable {
  // data structure for each session
  struct SessionInfo {
    std::string session_id;
    time_t last_access;
  };
 public:
  SessionTable(unsigned int max_num, unsigned int expire_time);
  ~SessionTable();

  // Returns true if sessions reaches the max limit number.
  bool IsFull();
  
  // Adds a new session to the session table.
  void AddSession(const std::string& id);

  // Adds an exist session in 'another_table' to the session table, 
  // and remove it from 'another_table'.
  void AddSession(const std::string& id, SessionTable* another_table);

  // Returns true if the session exist in the table.
  bool Exist(const std::string& id);

  // Deletes session from the session table.
  void DeleteSession(const std::string& id);

  // Removes the expired sessions
  void RemoveExpireSession();

 private:
  // Removes session from the session table.
  SessionInfo* RemoveSession(const std::string& id);

  // Hash table for sessions
  typedef HashMap<std::string, SessionInfo*>::Type HashTable;
  HashTable sessions_;
  unsigned int max_session_num_;
  unsigned int session_expire_time_;
};

class SessionManager {

 public:
  SessionManager();
  ~SessionManager();

  // Creates a new session.
  bool CreateSession(const std::string& seed, std::string* session_id);

  // Checks if the session has login
  bool CheckSessionLogin(const std::string& id);

  // Removes the session with the id
  void RemoveSession(const std::string& id);

  // Returns true if session id exists in the session tables
  bool Exist(const std::string& id);

  bool IsFull();

  bool Login(const std::string& id);

  void RemoveExpireSessions();

 private:
  // The sessions table
  SessionTable* login_sessions_; // sessions that have login
  SessionTable* fresh_sessions_; // sessions that haven't login
};

#endif  // SITEMAPSERVICE_SESSIONMANAGER_H__
