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

// This file defines PageHandler interface and all its implementations.
// A PageHandler can be used to handle a specified kind of action.
// Action is retrieved from HttpContext::action() method.

#ifndef SITEMAPSERVICE_PAGEHANDLER_H__
#define SITEMAPSERVICE_PAGEHANDLER_H__

#include <time.h>
#include <string>
#include "sitemapservice/httpcontext.h"

class HttpSettingManager;
class SessionManager;

// PageHandler interface.
class PageHandler {
public:
  PageHandler() {}
  virtual ~PageHandler() {}
   
  virtual void Execute(HttpContext* context) = 0;
};

// Handler to get run time information.
class RuntimeInfoHandler : public PageHandler {
public:
  RuntimeInfoHandler() {}
  virtual ~RuntimeInfoHandler() {}

  virtual void Execute(HttpContext* context);
};

// Handler to get application settings.
class GetSettingHandler : public PageHandler {
public:
  GetSettingHandler() { }
  virtual ~GetSettingHandler() {}

  virtual void Execute(HttpContext* context);

  void set_setting_manager(HttpSettingManager* setting_manager) {
    setting_manager_ = setting_manager;
  }

private:
  HttpSettingManager* setting_manager_;
};

// Handler to change application settings.
class SetSettingHandler : public PageHandler {
public:
  SetSettingHandler() { }
  virtual ~SetSettingHandler() {}

  virtual void Execute(HttpContext* context);

  void set_setting_manager(HttpSettingManager* setting_manager) {
    setting_manager_ = setting_manager;
  }

private:
  static const std::string kWarnAnswer;
  static const std::string kSaveFailAnswer;

  HttpSettingManager* setting_manager_;

  bool ReloadSetting();
};

// Handler to process login request.
class LoginHandler : public PageHandler {
public:
  LoginHandler() {}
  virtual ~LoginHandler() {}

  virtual void Execute(HttpContext* context);

  void set_setting_manager(HttpSettingManager* setting_manager) {
    setting_manager_ = setting_manager;
  }
  void set_session_manager(SessionManager* session_manager) {
    session_manager_ = session_manager;
  }
private:
  SessionManager* session_manager_;
  HttpSettingManager* setting_manager_;
};

// Handler to process logout request.
class LogoutHandler : public PageHandler {
public:
  LogoutHandler() {}
  virtual ~LogoutHandler() {}

  virtual void Execute(HttpContext* context);

  void set_session_manager(SessionManager* session_manager) {
    session_manager_ = session_manager;
  }

private:
  SessionManager* session_manager_;
};

// Handler to get message bundle according language setting.
class MessageBundleHandler: public PageHandler {
public:
  MessageBundleHandler() {}
  virtual ~MessageBundleHandler() {}

  virtual void Execute(HttpContext* context);
};

// Handler to get main page of Admin Console.
class MainPageHandler : public PageHandler {
public:
  MainPageHandler() {}
  virtual ~MainPageHandler() {}

  virtual void Execute(HttpContext* context);

  void set_session_manager(SessionManager* session_manager) {
    session_manager_ = session_manager;
  }
  void set_setting_manager(HttpSettingManager* setting_manager) {
    setting_manager_ = setting_manager;
  }

 private:
  SessionManager* session_manager_;
  HttpSettingManager* setting_manager_;
};

// Handler to process reload setting request.
class ReloadSettingHandler : public PageHandler {
public:
  ReloadSettingHandler() {}
  virtual ~ReloadSettingHandler() {}

  virtual void Execute(HttpContext* context);

private:
  static const std::string kReloadFailAnswer;

  bool ReloadSetting();
};

class ChangePasswordHandler : public PageHandler {
public:
  ChangePasswordHandler() {}
  virtual ~ChangePasswordHandler() {}

  virtual void Execute(HttpContext* context);
};
#endif // SITEMAPSERVICE_PAGEHANDLER_H__

