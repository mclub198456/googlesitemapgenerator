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
var SAVE_SUCCESS_MSG = 'Settings saved';
var SAVE_FAIL_MSG = 'Failed to save';
var SAVE_WARNING_MSG = 'Settings are out-of-date, do you want to save anyway?';
var REFRESH_MSG = 'Do you want to refresh the page to get latest settings?';

var RESTART_SUCCESS_MSG = 'Restart success';
var RESTART_FAIL_MSG = 'Restart failed';
var RESTART_WARNING_MSG = 
    'Please restart webserver manually to put the change into effect';

var CHPSWD_SUCCESS_MSG = 'Change password success';
var CHPSWD_FAIL_MSG = 'Change password failed';
var CHPSWD_FAIL_MSG1 = 'Change password failed, old password is not correct';
var CHPSWD_FAIL_MSG2 = 'Change password failed due to server error';

var LOGOUT_SUCCESS_MSG = 'Sign out success';
var LOGOUT_FAIL_MSG = 'Sign out failed';

var LOGIN_SUCCESS_MSG = 'Sign in success';
var LOGIN_FAIL_MSG = 'Sign in failed. You may try \'admin\' if it\'s your ' +
    'first time to sign in. Please don\'t forget to change the password when ' +
    'you sign in.';

// guide messages
var NEWS_ENABLE_MSG = 'Please make sure that the news Sitemap file of ' +
    'this site could be accepted by Google News';
var NEED_RESTART_WEBSERVER_CONFIRM_MSG = 'This change may need restart ' +
    'webserver to take effect, do you still want to change it?';
var ROBOTS_INCLUDE_CONFIRM_MSG = 'This function will modify your robots.txt' +
    ' file.\nAre you sure to continue?';
var SITE_NOT_RUNNING = 'Site services are not running';
var SERVER_NOT_REACHABLE = 'Server is unreachable!';

var BROWSER_REQUIRE_MSG = 'The browser you\'re using is not on the support' +
    'list, please use one of the following browser: ';
var REFRESH_CONFIRM_MSG =
    'Refreshing pages will syncronize with the server\n' +
    'and abandon all your unsaved edits.\n' +
    'Are you sure you want to continue?';
var REFRESH_DONE_MSG = 'Refresh is done';

var CANCEL_CUSTOMIZE_SETTING_MSG =
    'Are you sure you want to abandon your customized settings?';

var NEW_PASSWORDS_CONFLICT =
    'Error: Input passwords are not equal,<br /> please enter again';
var NEW_PASSWORDS_INVALID =
    'Error: New input password doesn\'t match' +
    '<br /> the length requirement, please enter again';

var VALIDATING_ONSAVE_FAIL_MSG =
    'The site settings are incompletely or incorrectly filled out.\n' +
    'Please correct the highlighted fields and try again.'

var VALIDATING_FAIL_MSG = 'Input validation failed! Please correct the error.';
var DISCARD_CHANGE_TO_CURRENT_TAB = 'Some settings have wrong value, do you' +
    ' want to discard all the editings on this page?';

var SAVE_RESTART_MSG = 'Are you sure you want to restart the server?';
var WAITING_FOR_SERVER_RESTART_MSG = 'Waiting for server to restart...';

// setting labels
var ADD_BUTTON_VALUE = 'Add';
var DELETE_BUTTON_VALUE = 'Delete';

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
  changePassword: 'change',
  confirm: 'OK',
  cancel: 'Cancel',
  updateRuntime: 'Refresh\nRuntime Info',
  submit: 'Save',
  refresh: 'Refresh',
  restart: 'Restart',
  logout: 'Sign out',
  login: 'Sign in'
};

/**
 * The tooltips of the settings.
 */
SettingEditorLanguage.tips = {
  addGeneratorInfo: 'add version information of Google Sitemap Generator ' +
      'into the generated Sitemap',
  logPath: 'The log path of the site',
  host: 'The host name of the site',
  robotsIncluded: 'Submit Sitemaps through the robots.txt file',
  updateRuntime: 'Refresh the runtime information',
  loggingLevel: 'The logging level on the server side',
  urlInDatabase: 'The number of urls in the Generator database',
  urlInTempfile: 'The number of urls in the temple files',
  urlInMemory: 'The number of urls in the memory',
  hostName: 'Host name of the site (the most common used)',
  memoryUsageForSite: 
      'How much memory space has been occupied by the site services',
  diskUsageForSite: 
      'How much disk space has been occupied by the site services',
  memoryUsage: 
      'How much memory space has been occupied by the internal URL database',
  diskUsage: 
      'How much disk space has been occupied by the internal URL database',
  startTime: 'When did the application start running',
  ServiceInfoSuccess: 'display if the last running is successful',
  ServiceInfoLastUpdate: 'The time of this service\'s last running',
  ServiceInfoUrlsCount: 'The amount of URLs that are generated into the ' +
      'Sitemap files of the service',
  changeLoginPassword: 'Change the Sign in password',
  changePassword: 'Input new password here if you want to change it',
  remoteAccess: 'Allow remote computer to access the settings page',
  loginPassword: 'Sign in password',
  login: 'Sign in',
  autoAdd: 'Add new site from web server automatically (need restart).',
  InAndExUrlPattern: 'The URLs that match patterns in \'Included Urls\' will ' +
      'be added to Sitemap, while the URLs that match patterns in \'Excluded ' +
      'Urls\' will never be added to Sitemap. If neither of them is set, all ' +
      'the URLs that match other limitations will add to Sitemap. If both ' +
      'are set, the URLs that match both will be excluded, while the URLs ' +
      'that match neither will be excluded too. The pattern start with / and ' +
      'mark * means anything.\neg. /*example*',
  websitemapurl: 'Http URL to submit the web Sitemap.\neg.' +
      'http: //www.example.com/ping?sitemap=',
  urlFind: 'Find the URL that you do not want to add to Sitemap without ' +
      'change, the content that in the [] will be replaced by other content ' +
      'before add to Sitemap file.\neg./abc[12*]xyz[45]',
  urlReplace: 'The content with the [] in \'replace\' field will be used to ' +
      'replace the content in \'find\' field with the [].',
  deleteRule: 'Delete this rule.',
  addRule: 'Add a new rule.',
  backupDuration: 'How long of the interval to save the in-memory URLs to ' +
      'the disk storage.',
  settingPort: 'Which port Google Sitemap Generator is listening ' +
      'for the configuration.',
  siteEnabled: 'Control Google Sitemap Generator service ' +
      'for the site.',
  siteEnabled_true: 'Start Google Sitemap Generator service for the site.',
  siteEnabled_false: 'Stop Google Sitemap Generator service for the site.',
  webserverFilterEnabled: 'Control Google Sitemap Generator using ' +
      'web server filter to collect URLs for the site.',
  webserverFilterEnabled_true: 'Enable web server filter for the site.',
  webserverFilterEnabled_false: 'Disable web server filter for the site.',
  fileScannerEnabled: 'Control Google Sitemap Generator using file scanner ' +
      'to collect URLs for the site.',
  fileScannerEnabled_true: 'Enable file scanner for the site.',
  fileScannerEnabled_false: 'Disable file scanner for the site.',
  logParserEnabled: 'Control Google Sitemap Generator using log parser ' +
      'to collect URLs for the site.',
  logParserEnabled_true: 'Enable log parser for the site.',
  logParserEnabled_false: 'Disable log parser for the site.',
  blogSearchPingEnabled: 'If checked, Google Sitemap Generator will send ' +
      'ping to search engine for new blog URL.',
  maxUrlLife: 'This defines the maximum age of the URLs that will be ' +
      'included in the Sitemap.\n The age of a URL is the duration from ' +
      'last access time to now.',
  maxUrlInMemory: 'Max URL numbers that Google Sitemap Generator can ' +
      'store in the memory.',
  maxUrlInDisk: 'Max URL numbers that Google Sitemap Generator can ' +
      'store in all the Sitemap files.',
  sitemapEnabled: 'If checked, Google Sitemap Generator will generate the ' +
      'Sitemap.',
  compress: 'If checked, the generated Sitemap files will be compressed.',
  filename: 'Sitemap file name. If there are more than one Sitemap file, ' +
      'it will be the Sitemap index file name.',
  UpdateStartTime: 'Start time of this Sitemap service.',
  UpdateDuration: 'It defines how long the Sitemap file will be regenerated.',
  UpdateDurationForBlogSearchPing: 
      'It defines how long the ping for blog search will be sent',
  UpdateDurationForFileScanner: 
      'It defines how long the file scanner wait to run again',
  UpdateDurationForLogParser: 
      'It defines how long the log parser wait to run again',
  maxFileUrl: 'You can use URL number to limit each Sitemap file\'s size.',
  maxFileSize: 'You can use disk size to limit each Sitemap file\'s size.',
  newsExpireDuration: 
      'This defines the maximum age of the URLs that will be ' +
      'included in the news Sitemap.\n The age of a URL is the duration from ' +
      'last access time to now.',
  customize: 'If checked, this part of settings will be synchronized to ' +
      'global settings.',
  submit: 'Save settings change to Google Sitemap Generator',
  refresh: 'Get newest settings from Google Sitemap Generator',
  restart: 'Save settings change and restart Google Sitemap Generator service',
  logout: 'Sign out from this editor',
  runtimeShow: 'Go to Runtime Info page',
  generalShow: 'Go to site settings page',
  webShow: 'Go to web Sitemap settings page',
  mobileShow: 'Go to mobile Sitemap settings page',
  codeSearchShow: 'Go to code search Sitemap settings page',
  blogSearchShow: 'Go to blog search ping settings page',
  videoShow: 'Go to video Sitemap settings page',
  newsShow: 'Go to news Sitemap settings page',
  globalLink: 'Go to Global settings page',
  siteLink: 'Go to this site settings page'
};

/**
 * The text labels' display contents.
 */
SettingEditorLanguage.texts = {
  passwordHints:
      'Hint: Length of new password should be<br />  at least six',
  URLSources: 'Sources of URLs',
  appSettingsLink: 'Application settings',
  siteSettingsLink: 'Site settings',
  projHomeLink: 'Home',
  helpLink: 'Help',
  logoutLink: 'Sign out',
  addGeneratorInfo: 'Add generator information into Sitemap file',
  webserverFilterEnabled: 'Webserver filter',
  webserverFilterEnabled_true: 'Enable',
  webserverFilterEnabled_false: 'Disable',
  fileScannerEnabled: 'File scanner',
  fileScannerEnabled_true: 'Enable',
  fileScannerEnabled_false: 'Disable',
  logParserEnabled: 'Log parser',
  logParserEnabled_true: 'Enable',
  logParserEnabled_false: 'Disable',
  logPath: 'Log path',
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
  blogSearchPingServiceInfoTitle: 'Blog Search Ping Service',
  logParserServiceInfoTitle: 'Log Parser Service',
  mobileSitemapServiceInfoTitle: 'Mobile Sitemap Service',
  codeSearchSitemapServiceInfoTitle: 'Code Search Sitemap Service',
  videoSitemapServiceInfoTitle: 'Video Sitemap Service',
  newsSitemapServiceInfoTitle: 'News Sitemap Service',
  webSitemapServiceInfoTitle: 'Web Sitemap Service',
  fileScannerServiceInfoTitle: 'File Scanner Service',
  webServerFilterServiceInfoTitle: 'Webserver Filter Service',
  ServiceInfoSuccess: 'Last running result',
  ServiceInfoLastUpdate: 'Last running time',
  ServiceInfoUrlsCount: 'Last update URLs count',
  urlInDatabase: 'URLs in database',
  urlInTempfile: 'URLs in tempfile',
  urlInMemory: 'URLs in memory',
  hostName: 'Host name',
  memoryUsageForSite: 'Memory used for all services',
  diskUsageForSite: 'Disk used for all services',
  memoryUsage: 'Memory used for internal DB',
  diskUsage: 'Disk used for internal DB',
  startTime: 'Start running time',
  appTitle: 'Application Settings',
  blogSearchPingTitle: 'Blog Search Ping Settings',
  password: 'Sign in password',
  changePassword: 'change',
  remoteAccess: 'Allow remote administration',
  loginUsername: 'Username',
  loginPassword: 'Password',
  siteTitle: 'General Settings',
  customize: 'Set to defaults',
  autoAdd: 'Automatically add websites from webserver',
  backupDuration: 'URLs auto save interval',
  secondUnit: 'seconds',
  minuteUnit: 'minutes',
  settingPort: 'Configuration port',
  urlPattern: 'URL pattern',
  url: 'URL',
  find: 'Find',
  replace: 'Replace',
  webEnabled: 'Enable web Sitemap generation',
  newsEnabled: 'Enable news Sitemap generation',
  videoEnabled: 'Enable video Sitemap generation',
  mobileEnabled: 'Enable mobile Sitemap generation',
  codeSearchEnabled: 'Enable code search Sitemap generation',
  blogSearchEnabled: 'Enable blog search Sitemap generation',
  siteEnabled: 'Google Sitemap Generator service',
  webEnabledSite: 'Enable web Sitemap generation for this site',
  newsEnabledSite: 'Enable news Sitemap generation for this site',
  videoEnabledSite: 'Enable video Sitemap generation for this site',
  mobileEnabledSite: 'Enable mobile Sitemap generation for this site',
  codeSearchEnabledSite: 'Enable code search Sitemap generation for this site',
  blogSearchEnabledSite: 'Enable blog search Sitemap generation for this site',
  siteEnabledSite: 'Google Sitemap Generator service for this site',
  siteEnabled_true: 'Start',
  siteEnabled_false: 'Stop',
  maxUrlLife: 'Time to live for URLs',
  dayUnit: 'days',
  maxUrlInMemory: 'Maximum number of URLs in memory cache',
  equal: ' equals to ',
  megaByteUnit: ' MB',
  maxUrlInDisk: 'Maximum number of URLs stored on disk',
  UrlReplaceList: 'Replaced URLs',
  webSitemapTitle: 'Web Sitemap Settings',
  videoSitemapTitle: 'Video Sitemap Settings',
  mobileSitemapTitle: 'Mobile Sitemap Settings',
  codeSearchSitemapTitle: 'Code Search Sitemap Settings',
  compress: 'Compress Sitemap file',
  filename: 'Sitemap file name',
  UpdateStartTime: 'Sitemap update start time',
  UpdateDuration: 'Frequency of updates',
  UpdateDurationForBlogSearchPing: 'Frequency of the ping',
  UpdateDurationForFileScanner: 'Frequency of the execution',
  UpdateDurationForLogParser: 'Frequency of the execution',
  maxFileUrl: 'Maximum number of URLs per Sitemap file',
  maxFileSize: 'Maximum size of Sitemap file',
  byteUnit: ' bytes',
  kbyteUnit: ' KB',
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
  example: 'example'
};
