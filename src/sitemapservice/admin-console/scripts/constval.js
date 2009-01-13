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
 * @fileoverview This file contains const values. We also put some
 *     brower-dependant code here.
 */
// default vars
var DEFAULT_PAGE = 'dashboard';
var DEFAULT_SITETAB = 'Settings';
var DEFAULT_SITEMAPTAB = 'web';
var DEFAULT_SITEID = 0;

// const vars
var MAX_WORDS_ONE_LINE_FOR_URLEDITOR = 55;
var FLAG_LIST_AUTO_EXTEND = true;
var ONE_DAY = 1;
var GLOBAL_SETTING_ID = 'g';
var TAB_COOKIE = 'gsg'; // should keep consistent with server side.
var KILO_UNIT_NUM = 1024;

// Customized HTML attribute names
var TRANSMARK_ATTRNAME = 'tm';

// CSS class name. If the CSS class name is used more than once, define it here,
// else, directly use it in js file.
var ACTIVETAB_CSS = 'active';
var INACTIVETAB_CSS = 'inactive';
var INVALID_CSS = 'invalid';
var HIDDEN_CSS = 'hidden';
var READONLY_CSS = 'readonly';
var CLICKABLE_CSS = 'clickable';
var ERR_CSS = 'error';

// server actions, should keep consistent with server side.
var CONFIG_XML_GET_ACTION = '/cgi-bin/admin-console.cgi?action=getxml';
var RUNTIME_XML_GET_ACTION = '/cgi-bin/admin-console.cgi?action=getruntimeinfo';
var XML_SET_ACTION = '/cgi-bin/admin-console.cgi?action=postxml';
var LOGOUT_ACTION = '/cgi-bin/admin-console.cgi?action=logout';
var LOGIN_ACTION = '/cgi-bin/admin-console.cgi?action=login';
var CHPSWD_ACTION = '/cgi-bin/admin-console.cgi?action=chpswd';

// post param names, should keep consistent with server side.
var LANGUAGE_PARAM_NAME = 'hl';
var XML_PARAM_NAME = 'xmlcontent';
var USERNAME_PARAM_NAME = 'username';
var PASSWORD_PARAM_NAME = 'password';
var OLD_PASSWORD_PARAM_NAME = 'opswd';
var NEW_PASSWORD_PARAM_NAME = 'npswd';
var TS_PARAM_NAME = 'ts';
var FORCE_PARAM_NAME = 'force';

// server response messages
// TODO: use HTTP status code to replace it.
var SAVE_WARN_MSG_FROM_SERVER = 'Settings is out-of-date';

/**
 * Namespace for some const variable groups.
 */
var ConstVars = {}

// Three types of setting fields.
ConstVars.settingFieldType = {TEXT: 1, LIST: 2, BOOL: 3};
// Default search engines for web sitemap automatically submission.
ConstVars.defaultSearchEngines = [
  {name: 'Microsoft', url: 'http://webmaster.live.com/ping.aspx?siteMap='},
  {name: 'Moreover.com', url: 'http://api.moreover.com/ping?u='},
  {name: 'Ask.com', url: 'http://submissions.ask.com/ping?sitemap='},
  {name: 'Yahoo!',
   url: 'http://search.yahooapis.com/SiteExplorerService/V1/ping?sitemap='},
  {name: 'Google',
   url: 'http://www.google.com/webmasters/sitemaps/ping?sitemap='}
];

///////////////////for IE////////////////////////////
if (!window.Node) {
  window.Node = {
    ELEMENT_NODE: 1,                 // Element
    ATTRIBUTE_NODE: 2,               // Attr
    TEXT_NODE: 3,                    // Text
    CDATA_SECTION_NODE: 4,           // CDATASection
    PROCESSING_INSTRUCTION_NODE: 7,  // ProcessingInstruction
    COMMENT_NODE: 8,                 // Comment
    DOCUMENT_NODE: 9,                // Document
    DOCUMENT_TYPE_NODE: 10,          // DocumentType
    DOCUMENT_FRAGMENT_NODE: 11       // DocumentFragment
  };
}
