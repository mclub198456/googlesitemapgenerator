// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview This file contains no class, it does the page initialization.
 * @author chaiying@google.com (Ying Chai)
 */

//////////////////////// Initialization ////////////////////////

function initAll(){
  // setup language related texts
  var perf = Perf.getPerf('Loading page');
  PageManager.setupLanguage();
  PageManager.InitElementEventHandlerStatic();
  if (DO_LOGIN) {
    PageManager.getInstance().showLoginPage();
  } else {
    PageManager.getInstance().showSettingPage();
  }
  perf.check('done');
  perf.report();

  // remember the language
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    cookie.hl = languageFlag;
    cookie.store(1);
  }
}

Util.event.add(window, 'load', initAll);
