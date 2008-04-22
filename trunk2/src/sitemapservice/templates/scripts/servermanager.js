// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview
 *
 * @author chaiying@google.com (Ying Chai)
 */

var ServerManager = {};

/**
 * Informs user that the request to server has been success.
 * @param {String} msg  The message to user
 */
ServerManager.informUserServerReturnSuccess = function(msg) {
  if (msg)
    alert(msg);
};

/**
 * Informs user that the request to server has been failed.
 * @param {String} msg  The message to user
 */
ServerManager.informUserServerReturnFail = function(msg) {
  if (msg)
    alert(msg);
};
////////////////////////////// From server tunnel ////////////////////////////
/**
 * Get XML content from server.
 * @param {String} cmd  The URI of the 'GET' request
 * @return {Document?} The XML document
 */
ServerManager.getXml = function(cmd) {
  var xmldom = null;

  var param = {};
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    if (cookie.hl) {
      param[LANGUAGE_PARAM_NAME] = cookie.hl;
    }
  }

  var xmlURL = cmd + '?' + AjaxUtil.encodeFormData(param);

  if (BROWSER_TYPE == Browser.types.FIREFOX) {
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

    // server may return empty string (text/html) if load xml file failed
    if (typeof xmldom != 'object' || !(xmldom instanceof Document)) {
      xmldom = null;
      Util.console.error('can not load xml file');
    } else {
      // TODO: an error condition need to check:
      // If the server return response content with type: 'text/xml', the Ajax
      // API will do the parsing automatically, but if the content is not valid
      // xml content, it will return an XmlDocument object that can't access
      // correctly.
    }
  } else if (BROWSER_TYPE == Browser.types.IE) {
    xmldom = AjaxUtil.loadXml(xmlURL);
  }

  return xmldom;
};

////////////////////////////// To server tunnel ////////////////////////////
/**
 * Send logout request.
 */
ServerManager.requestLogout = function() {
  return ServerManager.post_(LOGOUT_ACTION);
};

/**
 * Send save request.
 * @param {String} xmlstring  The content to be saved
 */
ServerManager.requestSave = function(xmlstring){
  var param = {};
  param[XML_PARAM_NAME] = xmlstring;

  return ServerManager.postWithDefaultAction_(
      XML_SET_ACTION, new ResponseMessage(SAVE_SUCCESS_MSG, SAVE_FAIL_MSG),
      param, 'text/xml');
};

/**
 * Send save and restart request.
 * @param {String} xmlstring  The content to be saved
 */
ServerManager.requestSaveRestart = function(xmlstring) {
  var param = {};
  param[XML_PARAM_NAME] = xmlstring;
  
  var action = new ResponseAction(
    function() {
      ServerManager.informUserServerReturnSuccess(SAVE_RESTART_SUCCESS_MSG);
    },
    function() {
      var msgFromServer = AjaxUtil.getResponseContent(this);
      var msg = SAVE_RESTART_FAIL_MSG;
      if (msgFromServer == SAVE_FAIL_MSG_FROM_SERVER) {
        msg = SAVE_FAIL_MSG2;
      } else if (msgFromServer == RESTART_FAIL_MSG_FROM_SERVER) {
        msg = RESTART_FAIL_MSG;
      } else {
        debugger;
      }
      ServerManager.informUserServerReturnFail(msg);
    }
  );
  return ServerManager.post_(SAVE_RESTART_ACTION, action, param, 'text/xml');
};

/**
 * Send login request.
 * @param {String} username  The login username
 * @param {String} password  The login password
 */
ServerManager.requestLogin = function(username, password) {
  var param = {};
  param[USERNAME_PARAM_NAME] = username;
  param[PASSWORD_PARAM_NAME] = password;

  var action = new ResponseAction(
    function(responser) {
      var xmldom = AjaxUtil.getResponseContent(responser);
      var xml = new XmlManager(xmldom);
      var sid = xml.getAttributeByXpath('/Session@id');
      if (Cookie) {
        var cookie = new Cookie(TAB_COOKIE);
        cookie.sid = sid;
        cookie.store(ONE_DAY);
      }
    },
    function() {ServerManager.informUserServerReturnFail(LOGIN_FAIL_MSG);}
  );
  
  return ServerManager.post_(LOGIN_ACTION, action, param);
};

/**
 * Send change password request.
 * @param {String} oldpswd  The old password
 * @param {String} newpswd  The new password
 */
ServerManager.requestChangePassword = function(oldpswd, newpswd) {
  var param = {};
  param[OLD_PASSWORD_PARAM_NAME] = oldpswd;
  param[NEW_PASSWORD_PARAM_NAME] = newpswd;

  var action = new ResponseAction(
    function() {
      ServerManager.informUserServerReturnSuccess(CHPSWD_SUCCESS_MSG);
    },
    function() {
      if (ServerManager.statusCode == 401) {
        ServerManager.errorMsg = CHPSWD_FAIL_MSG1;
      } else if (ServerManager.statusCode == 500) {
        ServerManager.errorMsg = CHPSWD_FAIL_MSG2;
      } else {
        ServerManager.errorMsg = CHPSWD_FAIL_MSG;
      }
      //ServerManager.informUserServerReturnFail(ServerManager.errorMsg);
    }
  );
  
  return ServerManager.post_(CHANGE_PSWD_ACTION, action, param);
};

////////////////////////////////////// POST methods ////////////////////////////
/**
 * Post with default action that inform user the success/fail message.
 * @param {String} cmd  The command to server
 * @param {ResponseMessage?} opt_msg  The message cooresponding to the server
 *   response
 * @param {Object?} opt_param  The parameter sent to server
 * @param {String?} opt_typeval  The HTTP content-type value
 * @private
 */
ServerManager.postWithDefaultAction_ = function(cmd, opt_msg, opt_param, 
                                                opt_typeval) {
  var action = opt_msg ? new ResponseAction(
    function() {
      ServerManager.informUserServerReturnSuccess(opt_msg.success());
    },
    function() {
      ServerManager.informUserServerReturnFail(opt_msg.failed());
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

  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    if (cookie.hl) {
      opt_param[LANGUAGE_PARAM_NAME] = cookie.hl;
    }
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

//////////////////////////////////////////////////////////////////////
/**
 * @constructor
 * @param {String} success  The server response success message to user
 * @param {String} failed  The server response failed message to user
 */
function ResponseMessage (success, failed) {
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

//////////////////////////////////////////////////////////////////////
/**
 * @constructor
 * @param {Function} success  The action on server response success 
 * @param {Function} failed  The action on server response failed
 */
function ResponseAction (success, failed) {
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
