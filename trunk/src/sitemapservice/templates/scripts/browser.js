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
 * @fileoverview
 *
 * @author chaiying@google.com
 */

var Browser = {};
/**
 * Browsers' type.
 * @enum {number}
 */
Browser.types = {IE: 1, FIREFOX: 2};

// check browser
var sUserAgent = navigator.userAgent;
var fAppVersion = parseFloat(navigator.appVersion);

function compareVersions(sVersion1, sVersion2){

  var aVersion1 = sVersion1.split(".");
  var aVersion2 = sVersion2.split(".");

  if (aVersion1.length > aVersion2.length) {
    for (var i = 0; i < aVersion1.length - aVersion2.length; i++) {
      aVersion2.push("0");
    }
  } else if (aVersion1.length < aVersion2.length) {
    for (var i = 0; i < aVersion2.length - aVersion1.length; i++) {
      aVersion1.push("0");
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
}

var isOpera = sUserAgent.indexOf("Opera") > -1;

// for IE
var isIE = sUserAgent.indexOf("compatible") > -1 &&
           sUserAgent.indexOf("MSIE") > -1 &&
           !isOpera;

var isMinIE6 = false;
var isMinIE7 = false;

if (isIE) {
  var reIE = new RegExp("MSIE (\\d+\\.\\d+);");
  reIE.test(sUserAgent);
  var fIEVersion = parseFloat(RegExp["$1"]);

  isMinIE6 = fIEVersion >= 6.0;
  isMinIE7 = fIEVersion >= 7.0;
}

// for Firefox
var isFF = sUserAgent.indexOf("Firefox") > -1;

var isMinFF1p5 = false;
var isMinFF2 = false;

if (isFF) {
  var reFF = new RegExp("Firefox/(\\d+\\.\\d+(?:\\.\\d+)?)");
  reFF.test(sUserAgent);
  isMinFF1p5 = compareVersions(RegExp["$1"], "1.5") >= 0;
  isMinFF2 = compareVersions(RegExp["$1"], "2.0") >= 0;
}

// alert user
var BROWSER_TYPE = null;
if (isMinIE6) {
  BROWSER_TYPE = Browser.types.IE;
} else if (isMinFF1p5) {
  BROWSER_TYPE = Browser.types.FIREFOX;
} else {
  var msg = BROWSER_REQUIRE_MSG + '\nfirefox1.5\nfirefox2.0\nIE6\nIE7\n';
  alert(msg);
  throw new Error('invalid browser');
}
