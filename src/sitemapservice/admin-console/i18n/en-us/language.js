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
 * @fileoverview The message.js files contains const message values,
 *     which is related to language. This file is for en-us.
 *
 * @author chaiying@google.com (Ying Chai)
 */
var languageFlag = 'en-us';

// server response inform messages
var DIRTY_WARNING_MSG = 'Haven\'t saved your edits, continue to reload?';
var PASSWD_WRONG_MSG = 'Password is not correct!';
var SAVE_SUCCESS_MSG = 'Settings saved';
var SAVE_FAIL_MSG = 'Failed to save settings';
var SAVE_WARNING_MSG = 'Settings are out-of-date, do you want to save anyway?';
var REFRESH_MSG = 'Do you want to refresh the page to get latest settings?';

var RELOAD_SUCCESS_MSG =
    'Reload success, your editing will take effect at next running cycle.';
var RELOAD_FAIL_MSG = 'Reload failed';
var RELOAD_WARNING_MSG =
    'Please reload webserver manually to put the change into effect';

var CHPSWD_SUCCESS_MSG = 'Your password has been changed.';
var CHPSWD_FAIL_MSG =
    'Your password failed to changed due to some unknown reason.';
var CHPSWD_FAIL_MSG1 = 'The old password is not correct.';
var CHPSWD_FAIL_MSG2 =
    'A server problem occurred and your password was not changed.';

var LOGOUT_SUCCESS_MSG = 'Sign out success';
var LOGOUT_FAIL_MSG = 'Sign out failed';

var LOGIN_SUCCESS_MSG = 'Sign in success';
var LOGIN_FAIL_MSG = 'Sign in failed.';

// guide messages
var NEWS_ENABLE_MSG = 'Be sure that your site is already authorized to ' +
    'send news content to Google News. ';
var REMOTE_ENABLE_MSG =
    'Remote access is only supported under "https" protocol!';
var NEED_RELOAD_WEBSERVER_CONFIRM_MSG = 'Your change to this setting ' +
    'will take effect only after the web server is reloaded. Continue?';
var ROBOTS_INCLUDE_CONFIRM_MSG = 'This function modifies your robots.txt' +
    ' file.\nAre you sure to continue?';
var SITE_NOT_RUNNING = 'Service components for this site are not running';
var SERVER_NOT_REACHABLE = 'Server is unreachable!';

var BROWSER_REQUIRE_MSG = 'The browser you\'re using is not on the support' +
    'list, please use one of the following browser: ';

var REFRESH_DONE_MSG = 'Current setting values have been refreshed.';

var CANCEL_CUSTOMIZE_SETTING_MSG =
    'Are you sure you want to abandon your customized settings?';

var NEW_PASSWORDS_CONFLICT =
    'The new passwords don\'t match.';
var NEW_PASSWORD_INVALID =
    'The new password must have 5 or more characters.';
var OLD_PASSWORD_INVALID =
    'The old password is invalid.';

var VALIDATING_ONSAVE_FAIL_MSG =
    'The site settings are incomplete or incorrec\n' +
    'Correct the highlighted fields and try again.'

var VALIDATING_FAIL_MSG = 'Input validation failed! Please correct the error.';
var DISCARD_CHANGE_TO_CURRENT_TAB = 'One or more settings are invalid, and ' +
    'edits to this page cannot be saved until invalid settings are corrected.' +
    ' If you continue to another tab, all edits to this tab will be ' +
    'discarded. Still continue?';

var CHANGE_UNSAVE_WARNING = 'You haven\'t saved your changes!';
var PRIVACY_WARNING = 'You have changed the default settings which were ' +
    'designed to protect your users\' privacy.  Your changes may enable ' +
    'sending user information to search engines. Make sure that any ' +
    'information you send complies with any commitments you make to your ' +
    'users under your privacy policy.';

// setting labels
var GLOBAL_SETTING_NAME = 'Defaults for All Sites';

//////////////////////////////////////////////////////////////////////
var SettingEditorLanguage = {};

/**
 * The title of the browser window.
 */
SettingEditorLanguage.title =
    'Administration Console - Google Sitemap Generator Beta';

/**
 * The 'value' attributes of the buttons.
 */
SettingEditorLanguage.values = {
  confirm: 'OK',
  cancel: 'Cancel',
  updateRuntime: 'Refresh',
  submit: 'Save',
  reload: 'Reload',
  logout: 'Sign out',
  login: 'Sign in',
  add: 'Add rule',
  del: 'Delete'
};

/**
 * The tooltips of the settings.
 */
SettingEditorLanguage.tips = {
  addGeneratorInfo: 'Add version information of Google Sitemap Generator ' +
      'into the generated Sitemap file',
  logPath: 'Pathname of the web server log file for this site. The log parser' +
      ' component of Google Sitemap Generator uses data in the log file to' +
      ' generate Sitemaps for this site.',
  host: 'Host name of the site',
  robotsIncluded: 'Submit Sitemaps through the robots.txt file',
  updateRuntime: 'Refresh the runtime information',
  urlInDatabase: 'Number of URLs in the Google Sitemap Generator database',
  urlInTempfile: 'Number of URLs in the temporary files',
  urlInMemory: 'Number of URLs in memory',
  hostName: 'Host name of the site. If the site has multiple names, choose ' +
      'the most commonly used. Google Sitemap Generator will set all URLs ' +
      'to this site name, to reduce duplication.',
  memoryUsageForSite:
      'Memory used by Google Sitemap Generator for processing this site.',
  diskUsageForSite:
      'Disk space used by Google Sitemap Generator for processing this site.',
  memoryUsage:
      'Amount of memory used by the internal URL database',
  diskUsage:
      'Amount of disk space used by the internal URL database',
  startTime: 'Application start time',
  ServiceInfoSuccess: 'Completion status of last Sitemap generation.',
  ServiceInfoLastUpdate: 'Last running end time for this component',
  ServiceInfoUrlsCount:
      'Number of URLs in current generated Sitemap files by this component',
  changeLoginPassword: 'Change the Sign in password',
  changePassword: 'Enter new password',
  remoteAccess: 'Allow a remote computer to access the administration console',
  loginPassword: 'Sign in password',
  login: 'Sign in',
  autoAdd:
      'Allow the administration console to automatically add websites ' +
      'that are defined on the web server. If the sites on the web server ' +
      'change, you must reload the Google Sitemap Generator service to  ' +
      'view the changes in this administration console.',
  InAndExUrlPattern:
      'A pattern must start with a slash (/) and can include ' +
      'asterisk (*) wildcards.<br>' +
      'URLs that match inclusion patterns will be included into Sitemap.<br>' +
      'URLs that match exclusion patterns will be excluded.<br>' +
      'URLs that match both patterns will be excluded.<br>' +
      'If no patterns have been defined, URLs will be included by default.<br>',
  NotifyUrl: 'HTTP URL for submitting a Web Sitemap.',
  ReplaceUrlPattern:
      '\'Find\' field specifies a pattern for finding URLs that ' +
      'must be transformed. Content in brackets ([]) will be replaced ' +
      'by content in brackets ([]) in \'Replace\' field.',
  deleteRule: 'Delete this rule.',
  addRule: 'Add a new rule.',
  backupDuration: 'Backup interval for in-memory URLs.',
  settingPort: 'Port on which Google Sitemap Generator listens ' +
      'for its configuration.',
  siteEnabled: 'Control Google Sitemap Generator service components ' +
      'for this site.',
  webserverFilterEnabled: 'Allow Google Sitemap Generator to use a ' +
      'web server filter to collect URLs for this site.',
  fileScannerEnabled: 'Allow Google Sitemap Generator to use a file scanner ' +
      'to collect URLs for this site.',
  logParserEnabled: 'Allow Google Sitemap Generator to use a log parser ' +
      'to collect URLs for this site.',
  blogSearchPingEnabled: 'If checked, Google Sitemap Generator will send ' +
      'ping to search engine for new blog URL.',
  sitemapEnabled:
      'Allow Google Sitemap Generator to generate this type of Sitemap for ' +
      'this site.',
  compress: 'Enables compression of generated Sitemap files.',
  filename: 'A Sitemap file name can consist of any ASCII alphanumeric ' +
      'characters and an .xml suffix. If there are multiple Sitemap files, ' +
      'this field specifies the Sitemap index file name.',
  UpdateStartTime:
      'Time at which this Sitemap generation should run. Use 24-hour time, ' +
      'in this format: yyyy-mm-dd hh:mm:ss Example: 2008-08-31 12:34:56',
  UpdateDuration: 'Pause time between Sitemap regenerations.',
  UpdateDurationForBlogSearchPing:
      'Pause time between pings to search engines.',
  maxFileUrl: 'Limit on number of URLs in a Sitemap file.',
  maxFileSize: 'Limit on file size of each Sitemap file.',
  newsExpireDuration:
      'Maximum age of URLs that will be included in News Sitemap files. ' +
      'URL age is equal to the time elapsed since the last access.',
  customize:
      'Whether all the configurations in this sitemap are using default value.'
//submit: 'Save the current settings. You\'ll need to click Reload in order ' +
//      'to apply the changes.',
//reload: 'Reload Google Sitemap Generator service to apply the change ' +
//      'you\'ve saved.',
};

/**
 * The text labels' display contents.
 */
SettingEditorLanguage.texts = {
  noEnableSites: 'No site is enabled. To add/subtract sites, ' +
      'go to <a id=siteMngLink href=#>this page</a>',
  enabledSitesTitle: 'Registered Sites',
  queryFieldGuide: 'Please add names of the URL query fields that you want to' +
      ' be included in Sitemap file. Use comma to seperate the input names.',
  privacyNote: 'Note: In order to protect web user\'s privacy, all the query ' +
      'fields in URLs that included in sitemaps will be eliminated except ' +
      'what have been specified here. By default  the Sitemap Generator drops' +
      ' all parameters. If you change the default settings, keep in mind that' +
      ' you should only add query fields that are necessary for search engine' +
      ' to know and not related to user\'s privacy information.  You should ' +
      'ensure that all information sent to search engines using the Sitemap ' +
      'Generator complies with your commitments to your users set out in your' +
      ' privacy policy.',
  maxUrlLifeTip:
      'Explain: URL age is measured by the time elapsed since the last access.',
  fileLimits: 'Settings for Sitemap file',
  ServiceInfoUrlPing: 'Last sent URL',
  promptPassword:
      'The page has been refreshed, need your password to save configuration.' +
      '<br>Please input login password again',
  passwordHints:
      'Hint: Length of new password should be<br />  at least six',
  URLSources: 'Sources of URLs',
  appSettingsLink: 'Application settings',
  siteSettingsLink: 'Site settings',
  projHomeLink: 'Online Home',
  logoutLink: 'Sign out',
  addGeneratorInfo: 'Include Google Sitemap Generator version in Sitemap file',
  webserverFilterEnabled: 'Webserver filter',
  webserverFilterEnabled_true: 'Enable',
  webserverFilterEnabled_false: 'Disable',
  fileScannerEnabled: 'File scanner',
  fileScannerEnabled_true: 'Enable',
  fileScannerEnabled_false: 'Disable',
  logParserEnabled: 'Log parser',
  logParserEnabled_true: 'Enable',
  logParserEnabled_false: 'Disable',
  logPath: 'Pathname for log file(s)',
  host: 'Host name',
  robotsIncluded: 'Include Sitemap URL in robots.txt',
  inputOldPassword: 'Current password:',
  inputNewPassword: 'New password:',
  inputPasswordAgain: 'Confirm new password:',
  loggingLevel: 'Logging level',
  loggingLevelNormal: 'Normal',
  loggingLevelImportant: 'Important',
  loggingLevelCritical: 'Critical',
  loggingLevelError: 'Error',
  runtimeInfo: 'Runtime Info',
  runtimeInfoTitle: 'Runtime Info for Site',
  applicationInfoTitle: 'Application Runtime Information',
  webSitemapServiceInfoTitle: 'Web Sitemap Creator',
  newsSitemapServiceInfoTitle: 'News Sitemap Creator',
  mobileSitemapServiceInfoTitle: 'Mobile Sitemap Creator',
  codeSearchSitemapServiceInfoTitle: 'Code Search Sitemap Creator',
  blogSearchPingServiceInfoTitle: 'Blog Search Ping',
  webServerFilterServiceInfoTitle: 'Webserver Filter',
  fileScannerServiceInfoTitle: 'File Scanner',
  logParserServiceInfoTitle: 'Log Parser',
  ServiceInfoSuccess: 'Last running result',
  ServiceInfoLastUpdate: 'Last running time',
  ServiceInfoUrlsCount: 'Last update URLs count',
  urlInDatabase: 'URLs in database',
  urlInTempfile: 'URLs in temporary file',
  urlInMemory: 'URLs in memory',
  hostName: 'Host name',
  memoryUsageForSite: 'Memory used by the service components for this site',
  diskUsageForSite: 'Disk used by the service components for this site',
  memoryUsage: 'Memory used for internal database',
  diskUsage: 'Disk used for internal database',
  startTime: 'Start running time',
  appTitle: 'Application Settings',
  blogSearchPingTitle: 'Blog Search Ping Settings',
  password: 'Sign in password',
  changePassword: 'change',
  remoteAccess: 'Allow remote administration',
  loginUsername: 'Username',
  loginPassword: 'Password',
  siteTitle: 'General Settings',
  customize_false: 'Use default settings',
  customize_true: 'Customize the settings',
  autoAdd: 'Automatically add websites from webserver',
  backupDuration: 'URLs auto save interval',
  secondUnit: 'seconds',
  minuteUnit: 'minutes',
  settingPort: 'Remote administration port',
  urlPattern: 'URL pattern',
  url: 'URL',
  find: 'Find',
  replace: 'Replace',
  maxUrlLife: 'Maximum age of URLs included in Sitemap file',
  dayUnit: 'days',
  maxUrlInMemory: 'Maximum number of URLs in memory cache',
  equal: 'Space required: about ',
  maxUrlInDisk: 'Maximum number of URLs stored on disk',
  UrlReplaceList: 'Replaced URLs',
  webSitemapTitle: 'Web Sitemap Settings',
  videoSitemapTitle: 'Video Sitemap Settings',
  mobileSitemapTitle: 'Mobile Sitemap Settings',
  codeSearchSitemapTitle: 'Code Search Sitemap Settings',
  compress: 'Compress Sitemap file',
  filename: 'Sitemap file name',
  UpdateStartTime: 'Update start time',
  UpdateDuration: 'Update interval',
  UpdateDurationForBlogSearchPing: 'Ping interval',
  UpdateDurationForFileScanner: 'Execution interval',
  UpdateDurationForLogParser: 'Execution interval',
  maxFileUrl: 'Maximum number of URLs',
  maxFileSize: 'Maximum file size',
  byteUnit: ' bytes',
  kiloByteUnit: ' KB',
  megaByteUnit: ' MB',
  gigaByteUnit: ' GB',
  teraByteUnit: ' TB',
  IncludedUrlList: 'Included URLs',
  ExcludedUrlList: 'Excluded URLs',
  NotifyUrlList: 'Search Engine Notification',
  newsSitemapTitle: 'News Sitemap Settings',
  newsExpireDuration: 'Time to live for news URLs',
  editorTitle: 'Administration Console',
  siteList: 'Site List',
  generalSitemap: 'General Settings',
  webSitemap: 'Web',
  mobileSitemap: 'Mobile',
  codeSearchSitemap: 'Code Search',
  blogSearchPing: 'Blog Search',
  videoSitemap: 'Video',
  newsSitemap: 'News',
  sitemap: 'Sitemaps',
  dateExample: 'Example: 2008-08-31 12:34:56'
};
