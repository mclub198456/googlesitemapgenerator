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
Util.event.add(window, 'unload', function() {
  Util.event.flush();
  Component.releaseAll();
});
