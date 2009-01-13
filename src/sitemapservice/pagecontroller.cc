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

#include "sitemapservice/pagecontroller.h"

#include "common/port.h"
#include "common/logger.h"
#include "common/httpconst.h"
#include "common/timesupport.h"
#include "sitemapservice/httpsettingmanager.h"
#include "sitemapservice/sessionmanager.h"
#include "sitemapservice/securitymanager.h"
#include "sitemapservice/pagehandler.h"

const std::string PageController::kXmlGetAction = "/getxml";
const std::string PageController::kRuntimeInfoAction = "/getruntimeinfo";
const std::string PageController::kXmlSetAction = "/postxml";
const std::string PageController::kReloadAction = "/reload";
const std::string PageController::kLogoutAction = "/logout";
const std::string PageController::kLoginAction = "/login";
const std::string PageController::kMessageBundleAction = "/language.js";
const std::string PageController::kMainAction = "/main";
const std::string PageController::kChangePasswordAction = "/chpswd";

PageController PageController::instance_;

PageController::PageController() {
  session_manager_ = NULL;
  setting_manager_ = NULL; 
  security_manager_ = NULL;
}

PageController::~PageController() {
  if (session_manager_ != NULL) delete session_manager_;
  if (setting_manager_ != NULL) delete setting_manager_;
  if (security_manager_ != NULL) delete security_manager_;

  while (handlers_.begin() != handlers_.end()) {
    delete handlers_.begin()->second;
    handlers_.erase(handlers_.begin());
  }
}

void PageController::RegisterHandler(const std::string& path,
                                     PageHandler* handler,
                                     bool is_private) {
  handlers_[path] = handler;
  if (is_private) {
    security_manager_->RegisterPrivatePage(path);
  }
}

bool PageController::Initialize() {
  session_manager_ = new SessionManager();
  setting_manager_ = new HttpSettingManager();

  security_manager_ = new SecurityManager();
  if (!security_manager_->Initialize(setting_manager_, session_manager_)) {
    Logger::Log(EVENT_ERROR, "Failed to initialize security manager.");
    return false;
  }
 
  MainPageHandler* mainpage_handler = new MainPageHandler();
  mainpage_handler->set_setting_manager(setting_manager_);
  mainpage_handler->set_session_manager(session_manager_);
  RegisterHandler("/", mainpage_handler, false);

  mainpage_handler = new MainPageHandler();
  mainpage_handler->set_setting_manager(setting_manager_);
  mainpage_handler->set_session_manager(session_manager_);
  RegisterHandler(kMainAction, mainpage_handler, false);

  LoginHandler* login_handler = new LoginHandler();
  login_handler->set_setting_manager(setting_manager_);
  login_handler->set_session_manager(session_manager_);
  RegisterHandler(kLoginAction, login_handler, false);

  LogoutHandler* logout_handler = new LogoutHandler();
  logout_handler->set_session_manager(session_manager_);
  RegisterHandler(kLogoutAction, logout_handler, true);

  RegisterHandler(kReloadAction, new ReloadSettingHandler(), true);

  GetSettingHandler* getsetting_handler = new GetSettingHandler();
  getsetting_handler->set_setting_manager(setting_manager_);
  RegisterHandler(kXmlGetAction, getsetting_handler, true);

  SetSettingHandler* setsetting_handler = new SetSettingHandler();
  setsetting_handler->set_setting_manager(setting_manager_);
  RegisterHandler(kXmlSetAction, setsetting_handler, true);

  RegisterHandler(kRuntimeInfoAction, new RuntimeInfoHandler(), true);

  RegisterHandler(kMessageBundleAction, new MessageBundleHandler(), false);

  RegisterHandler(kChangePasswordAction, new ChangePasswordHandler(), true);

  return true;
}

void PageController::Process(HttpContext* context) {
  // Remove expire sessions for every request.
  session_manager_->RemoveExpireSessions();

  if (!security_manager_->Check(context)) {
    Logger::Log(EVENT_NORMAL, "Security check failed.");

  } else {
    std::map<std::string, PageHandler*>::iterator itr = handlers_.find(
        context->action());
    if (itr != handlers_.end()) {
      itr->second->Execute(context);
    } else {
      context->response()->Reset(HttpConst::kStatus404, "");
      Logger::Log(EVENT_ERROR, "Page can't be found [%s].", context->action().c_str());
    }
  }

  // Add server date.
  HttpResponse* response = context->response();
  response->SetHeader(HttpConst::kHeaderDate,
                      FormatHttpDate(time(NULL)));
}


