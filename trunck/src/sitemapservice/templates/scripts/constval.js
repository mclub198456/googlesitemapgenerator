// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview This file contains const values. We also put some
 *     brower-dependant code here.
 *
 * @author chaiying@google.com (Ying Chai)
 */
var MAX_SITELIST_WIDTH = 20;
var URL_RECORD_SIZE_INBYTES = 560;
var ONE_DAY = 1;
var GLOBAL_SETTING_ID = 'g';

// HTML attribute name
var BOOL_HTML_ATTRNAME = 'checked';
var STRING_HTML_ATTRNAME = 'value';
var BINDMARK_ATTRNAME = 'bindmark';
var TRANSMARK_ATTRNAME = 'transmark';

// server action
var CONFIG_XML_GET_ACTION = 'getxml';
var RUNTIME_XML_GET_ACTION = 'getruntimeinfo';
var XML_SET_ACTION = 'postxml';
var SAVE_RESTART_ACTION = 'saverestart';
var LOGOUT_ACTION = 'logout';
var LOGIN_ACTION = 'login';
var CHANGE_PSWD_ACTION = 'chpswd';

// server response message
var SAVE_FAIL_MSG_FROM_SERVER = 'Save Failed';
var RESTART_FAIL_MSG_FROM_SERVER = 'Restart Failed';

var TAB_CONTAINER_NAME = 'tabpage';
var TAB_COOKIE = 'sitemapsetting';

var SESSIONID_PARAM_NAME = 'sessionId';
var LANGUAGE_PARAM_NAME = 'hl';
var XML_PARAM_NAME = 'xmlcontent';
var USERNAME_PARAM_NAME = 'username';
var PASSWORD_PARAM_NAME = 'password';
var OLD_PASSWORD_PARAM_NAME = 'opswd';
var NEW_PASSWORD_PARAM_NAME = 'npswd';

var SERVER_RESTART_DELAY = 30000;

var SAVE_SUCCESS_RESULT_CSS_CLASS = 'saveSuccess';
var SAVE_FAILED_RESULT_CSS_CLASS = 'saveFailed';
var TEXT_LABEL_CSS_CLASS = 'textLabel';
var INPUT_URLREPLACE_CSS_CLASS = 'urlreplace';
var VMIDDLE_CSS_CLASS = 'vmiddle';

var ACTIVE_TAB = 'activeTab';
var INACTIVE_TAB = 'inactiveTab';

var ACTIVE_SITE = 'activeSite';
var INACTIVE_SITE = 'inactiveSite';

var INPUT_INVALID_CSS_CLASS = 'invalid';
var HIDDEN_CLASS = 'hidden';
var PSEUDO_HIDDEN_CLASS = 'pseudoHidden';
var DISPLAY_CLASS = 'display';
var READONLY_CLASS = 'readonly';
var POPUP_CLASS = 'popup';
var SITELIST_GLOBALSETTING_CSS_CLASS = 'global';
var DEBUG_WINDOW_NAME = 'debugOutputWindow';

var XML_FILE_INDENT = '    ';

/**
 * @constructor
 */
function ConstVars() {}

/**
 * The sitemap type definition.
 *
 * GENERAL means the site setting, treat it as a special sitemap called general.
 * WEB means the web sitemap setting.
 * NEWS means the news sitemap setting.
 *
 * @enum {number}
 */
ConstVars.settingFieldType = {TEXT: 1, LIST: 2, BOOL: 3};

//////////////////////////////////////////////////////
///////////////////for IE////////////////////////////
if (BROWSER_TYPE == Browser.types.IE) {
  var Node = {
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
