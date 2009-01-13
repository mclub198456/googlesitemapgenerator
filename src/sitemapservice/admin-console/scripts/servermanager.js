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
 * @fileoverview  The ajax communication logic module.
 */

/**
 * The namespace of this module.
 */
var ServerManager = {};

////////////////////////////// From server tunnel ////////////////////////////
/**
 * Gets XML content from server.
 * @param {String} cmd  The URI of the 'GET' request
 * @return {Document?} The XML document
 */
ServerManager.getXml = function(cmd) {
  var xmldom = null;

  var param = {};
  var lang = _getSession().getCookie(Session.cookieItems.LANG);
  if (lang) {
    param[LANGUAGE_PARAM_NAME] = lang;
  }

  var xmlURL = cmd + '&' + AjaxUtil.encodeFormData(param);

  // create requester
  var requester = {
    url: xmlURL,
    wait: true
  };

  //create responser
  var responser = new AjaxUtil_responser(requester);
  responser.onResponseFail = function(){
    ServerManager.statusCode = responser.xhr.status;
  };

  // do request
  AjaxUtil.makeRequest(requester, responser);

  // check result
  xmldom = AjaxUtil.getResponseContent(responser);
  if (xmldom == null) {
    _err('can not load xml file');
  }

  return xmldom;
};

////////////////////////////// To server tunnel ////////////////////////////
/**
 * Sends logout request.
 * @return {Boolean} If the request is successful
 */
ServerManager.requestLogout = function() {
  return ServerManager.post_(
      LOGOUT_ACTION + '&sid=' + _getSession().id());
};

/**
 * Send save request.
 * @param {String} xmlstring  The content to be saved
 * @param {Object} ts  The 'value' property of 'ts' object is a string that used
 * to pass in/out the timestamp of the configuration. When passing in, it
 * represents the timestamp when the configuration is got from server, which
 * will be used for server to judge if the configuration has been out-of-date.
 * If the save action is successful, server will response the new timestamp,
 * which will be updated to the 'value' property of 'ts' object when function
 * return.
 * @param {Boolean} opt_force  The 'force' param flag, if true, it tells server
 * to save the content anyway
 * @return {ServerManager.requestSave.rets} If the request is successful
 */
ServerManager.requestSave = function(xmlstring, ts, opt_force){
  var param = {};
  param[XML_PARAM_NAME] = xmlstring;
  param[TS_PARAM_NAME] = ts.value;
  if (opt_force) {
    // 'force' is a flag param, the value is not important
    param[FORCE_PARAM_NAME] = 1;
  }

  var ret;
  var action = new ResponseAction(
    function(responser) {
      var msgFromServer = AjaxUtil.getResponseContent(responser);
      if (msgFromServer == SAVE_WARN_MSG_FROM_SERVER) {
        ret = ServerManager.requestSave.rets.OUTOFDATE;
      } else {
        ret = ServerManager.requestSave.rets.SUCCESS;
        alert(SAVE_SUCCESS_MSG);
        ts.value = msgFromServer;
      }
    },
    function() {
      ret = ServerManager.requestSave.rets.FAILED;
    }
  );

  // the 'post_' function call will change the value of the 'ret' var
  ServerManager.post_(XML_SET_ACTION + '&sid=' + _getSession().id(),
                      action, param, 'text/xml');
  return ret;
};

/**
 * The return result of the 'requestSave' function.
 * @enum
 */
ServerManager.requestSave.rets = {SUCCESS: 0, FAILED: 1, OUTOFDATE: 2};

/**
 * Sends login request.
 * @param {String} username  The login username
 * @param {String} password  The login password
 * @return {Boolean} If the request is successful
 */
ServerManager.requestLogin = function(username, password) {
  var param = {};
  param[USERNAME_PARAM_NAME] = username;
  param[PASSWORD_PARAM_NAME] = password;

  var action = new ResponseAction(
    function() {},
    function() {
      alert(LOGIN_FAIL_MSG);
    }
  );

  return ServerManager.post_(LOGIN_ACTION, action, param);
};

 /**
 * Sends change password request.
 * @param {String} oldpswd  The old password
 * @param {String} newpswd  The new password
 */
ServerManager.requestChangePassword = function(oldpswd, newpswd) {
  var param = {};
  param[OLD_PASSWORD_PARAM_NAME] = oldpswd;
  param[NEW_PASSWORD_PARAM_NAME] = newpswd;

  var action = new ResponseAction(
    function() {
      alert(CHPSWD_SUCCESS_MSG);
    },
    function() {
      if (ServerManager.statusCode == 401) {
        ServerManager.errorMsg = CHPSWD_FAIL_MSG1;
      } else if (ServerManager.statusCode == 500) {
        ServerManager.errorMsg = CHPSWD_FAIL_MSG2;
      } else {
        ServerManager.errorMsg = CHPSWD_FAIL_MSG;
      }
    }
  );

  return ServerManager.post_(
      CHPSWD_ACTION + '&sid=' + _getSession().id(), action, param);
};

////////////////////////////// POST methods ////////////////////////////
/**
 * Post with default action that inform user the success/fail message.
 * @param {String} cmd  The command to server
 * @param {ResponseMessage?} opt_msg  The message cooresponding to the server
 *   response
 * @param {Object?} opt_param  The parameter sent to server
 * @param {String?} opt_typeval  The HTTP content-type value
 * @private
 * @return {Boolean} If the request is successful
 */
ServerManager.postWithDefaultAction_ = function(cmd, opt_msg, opt_param,
                                                opt_typeval) {
  var action = opt_msg ? new ResponseAction(
    function() {
      alert(opt_msg.success());
    },
    function() {
      alert(opt_msg.failed());
    }
  ) : null;
  return ServerManager.post_(cmd, action, opt_param, opt_typeval);
};

/**
 * Directly access ajax API for POST method. If failed, will return false, and
 * the server response status code will save in ServerManager.statusCode.
 * So the method must be syncronizedly called.
 * @param {String} cmd  The command to server
 * @param {ResponseAction?} opt_action  The action cooresponding to the server
 *   response
 * @param {Object?} opt_param  The parameter sent to server
 * @param {String?} opt_typeval  The HTTP content-type value
 * @return {Boolean}  If the request is success.
 * @private
 */
ServerManager.post_ = function(cmd, opt_action, opt_param, opt_typeval) {
  // add session id
  if (!opt_param) {
    opt_param = {};
  }

  var lang = _getSession().getCookie(Session.cookieItems.LANG);
  if (lang) {
    opt_param[LANGUAGE_PARAM_NAME] = lang;
  }

  // create requester
  var requester = {
    url: cmd,
    type: opt_typeval,
    content: opt_param,
    timeout: 5000,// 5 sec
    wait: true
  };

  //create responser
  var responser = new AjaxUtil_responser(requester);
  var ret = true;  // if !waiting
  responser.onResponseSuccess = function(){
    if (opt_action)
      opt_action.success(responser);
    ret = true;
  };
  responser.onResponseFail = function(){
    ServerManager.statusCode = responser.xhr.status;
    if (opt_action)
      opt_action.failed(responser);
    ret = false;
  };

  // do request
  AjaxUtil.postContent(requester, responser);
  return ret;
};

//////////////////////////Some help classes /////////////////////////////
/**
 * @constructor
 * @param {String} success  The server response success message to user
 * @param {String} failed  The server response failed message to user
 */
function ResponseMessage(success, failed) {
  this.succMsg_ = success;
  this.failMsg_ = failed;
}
/**
 * Gets the message for server response success.
 * @return {String} The success message
 */
ResponseMessage.prototype.success = function() {
  return this.succMsg_;
};
/**
 * Gets the message for server response failed.
 * @return {String} The failed message
 */
ResponseMessage.prototype.failed = function() {
  return this.failMsg_;
};

/**
 * @constructor
 * @param {Function} success  The action on server response success
 * @param {Function} failed  The action on server response failed
 */
function ResponseAction(success, failed) {
  this.succAction_ = success;
  this.failAction_ = failed;
}
/**
 * Does the action for server response success.
 * @param {AjaxUtil_responser} responser  The Ajax responser
 */
ResponseAction.prototype.success = function(responser) {
  if (this.succAction_)
    this.succAction_(responser);
};
/**
 * Does the action for server response failed.
 * @param {AjaxUtil_responser} responser  The Ajax responser
 */
ResponseAction.prototype.failed = function(responser) {
  if (this.failAction_)
    this.failAction_(responser);
};
