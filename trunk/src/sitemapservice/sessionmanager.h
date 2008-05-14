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

// This file defines a class to manage the user session. The user session
// object currently only record the session id for authorization and access
// time to remove expired sessions. It also limits the max number of sessions.

#ifndef WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_SESSIONMANAGER_H__
#define WEBSITE_TOOLS_SITEMAP_SITEMAPSERVICE_SESSIONMANAGER_H__

#include "common/hashmap.h"

class HttpProto;

class SessionManager {
  // data structure for each session
  struct SessionInfo {
    std::string session_id;
    time_t last_access;
  };

  // Hash table for sessions
  typedef HashMap<std::string, SessionInfo*>::Type HashTable;
 public:
  // Creates a new session.
  bool CreateSession(const std::string& seed, std::string* session_id);

  // Fetches session id from the request and get the session info from server
  SessionInfo* GetSession(HttpProto *r);

  // Removes the expired sessions
  void RemoveExpireSession();

  // Checks if the session has login
  bool CheckSessionLogin(HttpProto* r);

  // Returns true if sessions reaches the max limit number.
  bool IsFull();

  // Removes the session with the id
  void RemoveSession(const std::string& id);

 private:
  static const unsigned int kMaxSessionNum;
  static const unsigned int kSessionExpireTime; // secs, 30 mins

  // The sessions table
  HashTable sessions_;
};

#endif
