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
 * @fileoverview
 */

var Browser = {};
/**
 * Browsers' type.
 * @enum {number}
 */
Browser.types = {IE: 1, FIREFOX: 2, SAFARI: 3, CHROME: 4};

/**
 * Compares this two versions.
 * @param {Object} sVersion1
 * @param {Object} sVersion2
 * @supported All browsers
 */
Browser.compareVersions = function(sVersion1, sVersion2) {

  var aVersion1 = sVersion1.split('.');
  var aVersion2 = sVersion2.split('.');

  if (aVersion1.length > aVersion2.length) {
    for (var i = 0; i < aVersion1.length - aVersion2.length; i++) {
      aVersion2.push('0');
    }
  } else if (aVersion1.length < aVersion2.length) {
    for (var i = 0; i < aVersion2.length - aVersion1.length; i++) {
      aVersion1.push('0');
    }
  }

  for (var i = 0; i < aVersion1.length; i++) {

    if (aVersion1[i] < aVersion2[i]) {
      return -1;
    } else if (aVersion1[i] > aVersion2[i]) {
      return 1;
    }
  }

  return 0;
};

(function() {
  // check browser
  var sUserAgent = navigator.userAgent;
  var fAppVersion = parseFloat(navigator.appVersion);
  Browser.isOpera = sUserAgent.indexOf('Opera') > -1;

  // for Chrome
  Browser.isCRM = sUserAgent.indexOf('Chrome') > -1;

  Browser.isMinCRM0p2 = false;

  if (Browser.isCRM) {
    var reCRM = new RegExp('Chrome/(\\d+\\.\\d+(?:\\.\\d+)?)');
    reCRM.test(sUserAgent);
    var fCRMVersion = parseFloat(RegExp['$1']);

    Browser.isMinCRM0p2 = fCRMVersion >= 0.2;
  }

  // for IE
  Browser.isIE = sUserAgent.indexOf('compatible') > -1 &&
             sUserAgent.indexOf('MSIE') > -1 &&
             !Browser.isOpera;

  Browser.isMinIE6 = false;
  Browser.isMinIE7 = false;

  if (Browser.isIE) {
    var reIE = new RegExp('MSIE (\\d+\\.\\d+);');
    reIE.test(sUserAgent);
    var fIEVersion = parseFloat(RegExp['$1']);

    Browser.isMinIE6 = fIEVersion >= 6.0;
    Browser.isMinIE7 = fIEVersion >= 7.0;
  }

  // for Firefox
  Browser.isFF = sUserAgent.indexOf('Firefox') > -1;

  Browser.isMinFF1p5 = false;
  Browser.isMinFF2 = false;

  if (Browser.isFF) {
    var reFF = new RegExp('Firefox/(\\d+\\.\\d+(?:\\.\\d+)?)');
    reFF.test(sUserAgent);
    Browser.isMinFF1p5 = Browser.compareVersions(RegExp['$1'], '1.5') >= 0;
    Browser.isMinFF2 = Browser.compareVersions(RegExp['$1'], '2.0') >= 0;
  }

  // for Safari
  Browser.isSFR = Browser.isCRM ? false : sUserAgent.indexOf('Safari') > -1;

  Browser.isMinSFR3 = false;

  if (Browser.isSFR) {
    var reSFR = new RegExp('Version/(\\d+\\.\\d+(?:\\.\\d+)?)');
    reSFR.test(sUserAgent);
    Browser.isMinSFR3 = Browser.compareVersions(RegExp['$1'], '3.0') >= 0;
  }

  // alert user
  Browser.BROWSER_TYPE = null;
  if (Browser.isMinIE6) {
    Browser.BROWSER_TYPE = Browser.types.IE;
  } else if (Browser.isMinFF1p5) {
    Browser.BROWSER_TYPE = Browser.types.FIREFOX;
  } else if (Browser.isMinSFR3) {
    Browser.BROWSER_TYPE = Browser.types.SAFARI;
  } else if (Browser.isMinCRM0p2) {
    Browser.BROWSER_TYPE = Browser.types.CHROME;
  } else {
    var msg = BROWSER_REQUIRE_MSG +
        'Chrome0.2\nFirefox1.5\nFirefox2.0\nFirefox3\nIE6\nIE7\nIE8\nSafari3\n';
    alert(msg);
    Browser.BROWSER_TYPE = Browser.types.FIREFOX; // default way
  }
})();
