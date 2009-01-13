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
 * @fileoverview The message.js files contains const message values,
 *     which is related to language. This file is for en-us.
 *
 * @author chaiying@google.com (Ying Chai)
 */
var languageFlag = 'en-us';

// Save-setting messages
var CHANGE_UNSAVE_WARNING = 'You have not yet saved your changes.';
var SAVE_SUCCESS_MSG = 'Settings saved';
var SAVE_FAIL_MSG = 'Failed to save settings';
var SAVE_WARNING_MSG = 
    'Another user changed these settings and you have not ' +
    'yet seen the new values. Do you still want to save settings, overwriting' +
    ' the other user\'s work?';
var REFRESH_MSG = 'Do you want to refresh the page to get the latest settings?';
var REFRESH_DONE_MSG = 'Current settings have been refreshed.';

// Password-change messages
var NEW_PASSWORDS_CONFLICT = 'The new passwords do not match.';
var NEW_PASSWORD_INVALID = 'The new password must have 5 or more characters.';
var OLD_PASSWORD_INVALID = 'The old password is invalid.';
var CHPSWD_SUCCESS_MSG = 'Your password has been changed.';
var CHPSWD_FAIL_MSG =
    'Your password failed to changed due to some unknown reason.';
var CHPSWD_FAIL_MSG1 = 'The old password is not correct.';
var CHPSWD_FAIL_MSG2 =
    'A server problem occurred and your password was not changed.';

// Login messages
var LOGIN_FAIL_MSG = 'Sign in failed.';

// Data fetch messages
var SETTING_FETCH_ERROR = 'Error: Cannot get the setting data from server.';
var RUNTIME_FETCH_ERROR = 
    'Warning: Cannot get runtime information from server.';

// Status page messages
var SITE_NOT_RUNNING = 'Service components for this site are not running';

// Special setting related messages 
var NEWS_ENABLE_MSG = 
    'Your site must be authorized to submit content to Google News. ' +
    'If necessary, submit your request at ' +
    'http://www.google.com/support/news_pub/bin/request.py?';
var REMOTE_ENABLE_MSG =
    'Remote access is supported only under HTTPS';
var ROBOTS_INCLUDE_CONFIRM_MSG = 
    'This function modifies your robots.txt ' +
    ' file.\nAre you sure you want to continue?';
var PRIVACY_WARNING = 
    'You have changed the default settings which were ' +
    'designed to protect user privacy.  Your changes could result ' +
    'in sending user information to search engines. Make sure that any ' +
    'information you send complies with the commitments made to your ' +
    'users under your privacy policy.';
var ERROR_MSG_QUERY_FIELD_DUPLICATE = 
    'This query field has already been specified: ';
var REVERT_MSG = 'Changes to query fields have been reverted.';
var NO_URL_INCLUDE = 
    'Your Sitemap URL filter excludes all URLs from this Sitemap.';

// Revert-to-default messages
var CANCEL_CUSTOMIZE_SETTING_MSG =
    'Are you sure you want to abandon your customized settings?';

// Validating messages
var VALIDATING_ONSAVE_FAIL_MSG = 
    'Site settings are incomplete or incorrect\n' +
    'Correct the highlighted fields and try again.'
var VALIDATING_FAIL_MSG = 'Input validation failed. Please correct the error.';

// Other messages
var SERVER_NOT_REACHABLE = 'Server is unreachable!';
var BROWSER_REQUIRE_MSG = 
    'This browser is not supported. ' +
    'Please use one of the following browsers: ';
var INVALID_PAGE = 'Invalid page!';

// Here's some text strings in UI.
// setting labels
var GLOBAL_SETTING_NAME = 'Defaults for All Sites';
// The title of the browser window.
var WINDOW_TITLE =
    'Administration Console - Google Sitemap Generator Beta';
// For dashboard page table.
var ENABLED_SITES = 'Registered Sites';
var NO_ENABLED_SITES = 
    'No site is enabled. To add or remove sites, ' +
    'go to <a id=siteMngLink href=#>this page</a>';
// For status page
var STATE_SUCCESS = 'Success';
var STATE_FAILED = 'Failed';
var STATE_DISABLE = 'Disabled';
var STATE_RUNNING = 'Running';
var STATE_READY = 'Haven\'t run';
// This means no item is set in the setting, for url patterns and query fields
// settings.
var NO_ITEM = 'no items';
var SHOW = 'Show all';
var HIDE = 'Hide all';
var EDIT = 'Edit';
var SAVE = 'Save';
var ENABLE = 'Enable';
var DISABLE = 'Disable';
// For sitemap table in "site configuration" page.
var SITEMAP_CUSTOM = 'custom';
var SITEMAP_DEFAULT = 'default';

//////////////////////////////////////////////////////////////////////
var GSGLang = {};

/**
 * The 'value' attributes of the buttons.
 */
GSGLang.values = {
  login: 'Sign in',
  submit: 'Save',
  cancel: 'Cancel',
  revert: 'Revert to default settings',
  change: 'Change',
  close: 'Close',
  add: 'Add'
};

/**
 * The tooltips for settings.
 */
GSGLang.tips = {
  NotifyUrl: 'HTTP URL for submitting a Web Sitemap.',
  hostName: 
      'Host name of the site. If the site has multiple names, choose ' +
      'the most commonly used. Google Sitemap Generator sets all URLs ' +
      'to this site name, to reduce duplication.',
  InAndExUrlPattern:  
      'Start each pattern with a slash (/) and use asterisk (*) wildcards as ' +
      'needed. A URL is included if it matches an inclusion pattern, or ' +
      'excluded if it matches an exclusion pattern or if it matches both ' +
      'types of patterns. If no patterns are defined, all URLs are excluded. ',  
  sitemapEnabled:
      'Allow Google Sitemap Generator to generate this type of Sitemap.',
  filename: 
      'The file name contains ASCII alphanumeric ' +
      'characters and an .xml suffix.'
};

/**
 * The text labels' display contents.
 */
GSGLang.texts = {
  // Message on top of the "login" page.
  welcome: 'Welcome to Google Sitemap Generator',
  // Text message on the loading page.
  loading: 'Loading...',
  // Links texts to different setting pages.
  pref: 'Preferences',
  projHomeLink: 'Online Home',
  logoutLink: 'Sign out',
  dashboard: 'Dashboard',
  siteMng: 'Manage sites',
  defaultSetting: 'Default site settings',
  defaultSitemap: 'Default Sitemap settings',
  // Labels for password-change on "preferences" page.
  pswd: 'Password:',
  chpswd: 'Change password',
  // Labels on "dashboard" page.
  applicationInfoTitle: 'Application Runtime Information',
  appStart: 'Application start time',
  allMem: 'Memory used for internal database',
  allDisk: 'Disk used for internal database',
  siteMngSubTitle: 'Manage Site Activation',
  // Labels on "preferences" page.
  addGeneratorInfo: 'Include Google Sitemap Generator version in Sitemap file',
  remoteAccess: 'Allow remote access to the Administration Console',
  inputOldPassword: 'Current password:',
  inputNewPassword: 'New password:',
  inputPasswordAgain: 'Confirm new password:',
  // Labels on "site manage" page.
  select: 'Select: ',
  all: 'All',
  none: 'None',
  // Left navigation bar.
  siteStat: 'Site status',
  siteConf: 'Site configuration',
  sm: 'Sitemap types', 
  web: 'Web',
  mobile: 'Mobile',
  csrch: 'Code Search',
  bsrch: 'Blog Search',
  news: 'News',
  // Labels on "site status" page.
  runtimeInfoTitle: 'Runtime Info for Site',
  sMem: 'Memory used',
  sDisk: 'Disk used',
  urlInDatabase: 'URLs in database',
  urlInTempfile: 'URLs in temporary file',
  urlInMemory: 'URLs in memory',
  hostName: 'Host name',
  sc: 'Sitemap creators',
  statTitle: 'Status',
  urlInSitemap: 'URLs in Sitemap',  
  setting: 'Settings',
  wf: 'Webserver filter',
  fs: 'File scanner',
  lp: 'Log parser',
  smInfoTime: 'Time of most recent Sitemap file generation',
  fsInfoTime: 'Time of most recent file system scan',
  lpInfoTime: 'Time of most recent log process',
  smInfoUrls: 'Number of URLs in most recent generated Sitemap file',
  srcInfoUrls: 'Number of collected URLs',
  pingTime: 'Last ping time',
  pingUrl: 'Last ping url',
  // Labels on "site configuration" page.
  host: 'Host name',
  logPath: 'Pathname for log file(s)',
  ex: 'Example:',
  rsLim: 'Resource limits',
  maxUrlLife: 'Maximum age of URLs included in Sitemap file',
  muMem: 'Maximum number of URLs in memory',
  muDisk: 'Maximum number of URLs on disk',
  spaceEst: 'Approximate space required:',
  collector: 'URL collectors',
  duration: 'Execution interval',
  qField: 'URL query fields',
  qFieldGuide: 
      'Specify the URL query fields to include in ' +
      'generated Sitemaps. Use a comma to separate multiple entries.',
  exQueryField: 'name1, name2, name3',
  privacyNote: 
      '<b>Notice:</b> By default, Google Sitemap Generator excludes all query' +
      ' fields from URLs in generated Sitemaps, to prevent the transmission ' +
      'of user-identifiable data. To override this policy, you can specify ' +
      'any query field that you want Google Sitemap Generator to include. ' +
      'However, be careful to add a query field here only if it is required ' +
      'by search engines and does not violate user privacy. You must ensure ' +
      'that all information sent to search engines using Google Sitemap ' +
      'Generator complies with your commitments to your users, as stated in ' +
      'your privacy policy.',
  smTypes: 'Sitemap types',
  smType: 'Sitemap type',
  enable: ENABLE,
  settingStat: 'Setting status',
  edit: EDIT,
  // Warning messages.
  sCustomWarn: 'This site configuration is customized.',
  smDisable: 'This Sitemap type is disabled.', 
  smCustom: 'This Sitemap type has been customized.', 
  smNoUrl: NO_URL_INCLUDE,
  // Labels on "sitemap types" pages.
  update: 'Sitemap generation schedule',
  startTime: 'Start date and time',
  interval: 'Interval',
  custom: 'custom value: ',
  smFile: 'Sitemap file settings',
  smFileCompress: 'Compress Sitemap file',
  filename: 'Sitemap file name',
  maxFileUrl: 'Maximum number of URLs',
  maxFileSize: 'Maximum file size',
  smSubmit: 'Sitemap file submission',
  robotsIncluded: 'Include Sitemap URL in robots.txt',
  notifyUrl: 'Search engine notification URLs',
  smUrlFilter: 'Sitemap URL filter',
  inUrlPattern: 'Included URL patterns',
  exUrlPattern: 'Excluded URL patterns',
  // Blogsearch sitemap page.
  ping: 'Ping schedule',
  // News sitemap page.
  newsExpireDuration: 'Time-to-live for URLs',
  // Time Units.
  mmUnit: 'minutes',
  hUnit: 'hour',  
  hhUnit: 'hours',  
  dUnit: 'day',
  ddUnit: 'days',
  wUnit: 'week',
  monUnit: 'month',
  // Space Units.
  byteUnit: ' bytes',
  kiloByteUnit: ' KB',
  megaByteUnit: ' MB',
  gigaByteUnit: ' GB',
  teraByteUnit: ' TB'
};
