// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview This file includes Controller class, which deal with the
 * transaction from user command.
 *
 * @author chaiying@google.com (Ying Chai)
 */


var Controller = {};


/**
 * Updates the setting XML data to UI.
 */
Controller.updateSettingData = function() {
  var dom = ServerManager.getXml(CONFIG_XML_GET_ACTION);
  if (!dom) {
    Controller.processErrorResponseFromServer_();
  }
  SiteSettings.getInstance().load(dom);
};

/**
 * Updates the runtimeinfo XML data to UI.
 * Gets the XML data from server, and fill it to all the runtime info page.
 */
Controller.updateRuntimeInfo = function() {
  var dom = ServerManager.getXml(RUNTIME_XML_GET_ACTION);
  if (!dom) {
    Controller.processErrorResponseFromServer_();
    Util.console.report('Warning: cannot get the runtime info from server!');
    return;
  }

  // load
  RuntimeManager.getInstance().load(dom);
};

/**
 * Deal with the error from server.
 * @private
 */
Controller.processErrorResponseFromServer_ = function() {
  // if not pass the authorization, login again
  if (ServerManager.statusCode == 401) {
    Controller.redirectToLoginPage_();
  }
};

/**
 * Gets the server listen port from the setting value.
 * @private
 */
Controller.getPortFromClientSetting_ = function() {
  // get the new port
  return SiteSettings.getInstance().getPort();
};

/**
 * Gets main page url.
 * @param {Number?} opt_port  The port of the setting
 * @private
 */
Controller.getMainPageURL_ = function(opt_port) {
  var url = window.location.href;
  var results = url.match(/^http\:\/\/([^\:]+)\:(\d+)/);
  if (!results || results.length < 3)
    return null;

  var domain = results[1];
  var port = opt_port ? opt_port : results[2];
  var param = null;
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    if (cookie.hl) {
      var date = {};
      date[LANGUAGE_PARAM_NAME] = cookie.hl;
      param = AjaxUtil.encodeFormData(date);
    }
  }
  
  return 'http://' + domain + ':' + port + '/main' + (param ? '?' + param : '');
};

/**
 * Goes to login page.
 * @param {Number?} opt_port  The port of the setting
 * @private
 */
Controller.redirectToLoginPage_ = function(opt_port) {
  window.location.href = Controller.getMainPageURL_(opt_port);
};

//////////////////////// Event handler ////////////////////////
/**
 * Submits the settings to server.
 */
Controller.onSubmitXml = function() {
  // Generally speaking, it should check all tabs in all sites.
  // But since program will check the current tab when user switch to another,
  // so we can assume that all the other tabs in the site have been checked,
  // and all tabs in other sites have been checked, too.
  // (we assume the values from server's xml are correct)
  if (TabManager.getInstance().currentTab().validate()) {
    SiteSettings.getInstance().save();
    var value = SiteSettings.getInstance().xml_.serialize();
    if (!ServerManager.requestSave(
        value)) {
      Controller.processErrorResponseFromServer_();
    }
  } else {
    alert(VALIDATING_ONSAVE_FAIL_MSG);
  }
  return false;
};

/**
 * Refresh the main page.
 */
Controller.onRefreshPage = function() {
  if (confirm(REFRESH_CONFIRM_MSG)) {
    Controller.updateSettingData();
    Controller.updateRuntimeInfo();
    alert(REFRESH_DONE_MSG);
  }
  return false;
};

/**
 * Submits the settings to server and restart the server.
 */
Controller.onSaveXmlAndRestart = function() {
  // Generally speaking, it should check all tabs in all sites.
  // But since program will check the current tab when user switch to another,
  // so we can assume that all the other tabs in the site have been checked,
  // and all tabs in other sites have been checked, too.
  // (we assume the values from server's xml are correct)
  if (TabManager.getInstance().currentTab().validate()) {
    if (confirm(SAVE_RESTART_MSG)) {
      SiteSettings.getInstance().save();
  
      // send save and restart command syncronized
      if (!ServerManager.requestSaveRestart(
          SiteSettings.getInstance().xml_.serialize())) {
        return;
      }
  
      // hide the content, show waiting page
      document.body.innerHTML = WAITING_FOR_SERVER_RESTART_MSG;
  
      // redirect to new server address
      var port = Controller.getPortFromClientSetting_();
      var waitTimer = setTimeout(function(){
        Controller.redirectToLoginPage_(port);
      }, SERVER_RESTART_DELAY);
    }
  } else {
    alert(VALIDATING_ONSAVE_FAIL_MSG);
  }
  return false;
};

/**
 * Logout from the setting main page.
 */
Controller.onLogout = function() {
  ServerManager.requestLogout();
  Controller.redirectToLoginPage_(Controller.getPortFromClientSetting_());
};

/**
 * Login to server.
 * It will send to server the login info, and get result from server.
 * If login succeeds, it will redirect to the main page of setting.
 */
Controller.onLogin = function() {
  // send to server
  var ret = ServerManager.requestLogin(
    PageManager.getInstance().getLoginUsername(),
    PageManager.getInstance().getLoginPassword());
  if (ret) { // success!!
    PageManager.getInstance().showSettingPage();
  }
};


