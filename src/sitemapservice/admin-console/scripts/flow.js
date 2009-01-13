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
// This is the top level setting class. It contains all the setting values for
// this application. Besides site specific settings, this class also includes
// application level configuration, like back-up duration, remote admin port,
// admin account, and etc. Especially, there is global setting field, which
// contains default values for site settings. Please see the member fields
// doc for details.
// Besides the xml setting load/save/validate functions, it provides functions
// to load values from file, as well as save value to a file.
// This class is not thread-safe.


/**
 * @fileoverview This file includes Controller class, which deal with the
 * transaction from user command.
 */

var Flow = {};

////////////// Utility functions to communicate to server //////////////////////
/**
 * Gets the XML data from server and updates it to UI.
 */
Flow.fetchSetting = function() {
  var http = {};
  var dom = ServerManager.getXml(
      CONFIG_XML_GET_ACTION + '&sid=' + _getSession().id());
  if (!dom) {
    // if not pass the authorization, login again
    if (ServerManager.statusCode == 401) {
      Flow.gotoLogin();
    }
    alert(SETTING_FETCH_ERROR);
    return;
  }

  _getSetting().setData(dom);
  ValidateManager.clear();
};

/**
 * Updates the runtimeinfo XML data to UI.
 * Gets the XML data from server, and fill it to all the runtime info page.
 */
Flow.fetchRuntime = function() {
  var dom = ServerManager.getXml(
      RUNTIME_XML_GET_ACTION + '&sid=' + _getSession().id());
  if (!dom) {
    // if not pass the authorization, login again
    if (ServerManager.statusCode == 401) {
      Flow.gotoLogin();
    }
    alert(RUNTIME_FETCH_ERROR);
    return;
  }

  // load
  _getRuntime().setData(dom);
};

/**
 * Handle the 'save' response. It may call itself.
 * @param {Object} xmlStr
 * @param {Object} ts
 * @param {Object} opt_force
 */
Flow.submitSetting = function(xmlStr, timestamp, opt_force) {
  var tsObj = {value: timestamp};
  switch (ServerManager.requestSave(xmlStr, tsObj, opt_force)) {
    case ServerManager.requestSave.rets.FAILED:
      // if not pass the authorization, login again
      if (ServerManager.statusCode == 401) {
        Flow.gotoLogin();
      } else {
        alert(SAVE_FAIL_MSG);
      }
      break;
    case ServerManager.requestSave.rets.OUTOFDATE:
      if (confirm(SAVE_WARNING_MSG)) {
        // post again
        Flow.submitSetting(xmlStr, tsObj, true);
      } else {
        if (confirm(REFRESH_MSG)) {
          // refresh the page, first setting, then runtime
          Flow.fetchSetting();
          Flow.fetchRuntime();
          alert(REFRESH_DONE_MSG);
          _getSetting().clearDirty();
          _getPager().refreshActionButtons();
        }
      }
      break;
    case ServerManager.requestSave.rets.SUCCESS:
      _getSetting().SetTimestamp(tsObj.value);
      _getSetting().clearDirty();
      _getPager().refreshActionButtons();
      break;
  }
};

/////////////////////////// Handlers //////////////////////////////////
/**
 * Handler for save.
 */
Flow.save = function() {
  // check if current sitemap or site setting is valid
  var data = _getSetting();
  if (data.validate()) {
    // save the setting to xml tree.
    data.save();

    // save to server
    Flow.submitSetting(data.getXmlString(), data.timestamp(), false);
    return true;
  } else {
    alert(VALIDATING_ONSAVE_FAIL_MSG);
    return false;
  }
};

/**
 * Handler for page load complete event.
 */
Flow.onLoad = function() {
  if (DO_LOGIN) {
    // clear the cookie for remembering the site and tab when user login.
    _getSession().clearCookie();
    _getPager().gotoPage('loginPage');
  } else {
    Flow.gotoConsole();
  }

  // remember the language and session id
  _getSession().saveCookie(Session.cookieItems.LANG, languageFlag);
  if (SESSION_ID != null) {
    _getSession().setId(SESSION_ID);
  }
};

/**
 * Handler for login.
 * It will send to server the login info, and get result from server.
 * If login succeeds, it will redirect to the main page of setting.
 */
Flow.login = function() {
  _getSession().login(_gel('loginPassword').value, function (success) {
    if (success) {
      Flow.gotoConsole();
    } else {
      // refresh the page to get new session id.
      window.location.reload();
    }
  });
};

/**
 * Handler for logout.
 */
Flow.logout = function() {
  ServerManager.requestLogout();
  Flow.gotoLogin();
};

///////////////////////// Page redirection ///////////////////////////////
/**
 * Goes to login page, use redirection to clear page status (not clear cookie).
 * @private
 */
Flow.gotoLogin = function() {
  // just refresh the page
  window.location.reload();
};

/**
 * Goes to administrator console page.
 * If user has login, it will be called after page load, otherwise, it will be
 * called after user pass login.
 */
Flow.gotoConsole = function() {
  // Notes: don't change the sequence
  // Gets XML from server
  Flow.fetchSetting();
  
  // update setting page with the data from server
  Flow.fetchRuntime();
  
  // hide login page and show setting page
  _getPager().showFirstPage();
};


//////////////////////// Url ////////////////////////

/**
 * Gets main page url.
 * @constructor
 * @param {Number?} opt_port  The port of the setting
 * @private
 */
function Url(url) {
  var results = url.match(/^http\:\/\/([^\:]+)\:(\d+)\/(.*)$/);
  if (!results || results.length != 4) {
    this.domain_ = null;
    this.port_ = null;
    return;
  }
  this.domain_ = results[1];
  this.port_ = results[2];
  this.uri_ = results[3];
}
Url.prototype.setPort = function(port) {
  this.port_ = port;
};
Url.prototype.get = function() {
  return 'http://' + this.domain_ + ':' + this.port_ + '/' + this.uri_;
};

//////////////////////// Session ////////////////////////
/**
 *
 * @constructor
 */
function Session() {
  this.pswd_ = '';
  this.cookie_ = new Cookie(TAB_COOKIE);
}

/**
 * Singleton access function.
 */
function _getSession() {
  if (_getSession.instance_ == null) {
    _getSession.instance_ = new Session();
  }
  return _getSession.instance_;
}

/**
 *
 */
Session.prototype.passwd = function() {
  return this.pswd_;
};

/**
 *
 * @param {Object} passwd
 */
Session.prototype.setPasswd = function(passwd) {
  this.pswd_ = passwd;
};

/**
 * Login to server to check password
 * @param {Object} passwd
 */
Session.prototype.login = function(opt_passwd, opt_callback) {
  if (opt_passwd) {
    this.setPasswd(opt_passwd);
  }
  // send to server
  var res = ServerManager.requestLogin(
      'admin', this.passwd(), false);
  if (opt_callback) {
    opt_callback(res);
  }
};

Session.cookieItems = {
  PAGE: 'page',
  SITE: 'site',
  SITETAB: 'siteTab',
  SITEMAP: 'sitemap',
  LANG: 'hl',
  ID: 'sid'
};

Session.prototype.clearCookie = function() {
  this.cookie_.remove();
};
Session.prototype.getCookie = function(name) {
  return this.cookie_[name];
};

Session.prototype.saveCookie = function(name, value) {
  var cookie = this.cookie_;
  cookie[name] = value;
  cookie.store(null, '', '', window.location.protocol == 'https:');
};

Session.prototype.id = function() {
  if (!this.id_) {
    this.id_ = this.getCookie(Session.cookieItems.ID);
  }
  return this.id_;
};

Session.prototype.setId = function(id) {
  this.id_ = id;
  this.saveCookie(Session.cookieItems.ID, id);
};
