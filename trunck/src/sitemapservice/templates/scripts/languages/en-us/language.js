// Copyright 2007 Google Inc.
// All Rights Reserved.

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

var SAVE_RESTART_SUCCESS_MSG = 'save and restart success';
var SAVE_RESTART_FAIL_MSG = 'Save and restart failed';
var SAVE_FAIL_MSG2 = 'Save failed, restart is cancelled';
var RESTART_FAIL_MSG = 'Save success, restart failed';

var CHPSWD_SUCCESS_MSG = 'Change password success';
var CHPSWD_FAIL_MSG = 'Change password failed';
var CHPSWD_FAIL_MSG1 = 'Change password failed, old password is not correct';
var CHPSWD_FAIL_MSG2 = 'Change password failed due to server error';

var LOGOUT_SUCCESS_MSG = 'Logout success';
var LOGOUT_FAIL_MSG = 'Logout failed';

var LOGIN_SUCCESS_MSG = 'Login success';
var LOGIN_FAIL_MSG = 'Login failed. You may try \'admin\' if it\'s your ' +
    'first time to login. Please don\'t forget to change the password when ' +
    'you login.';

// guide messages
var NEWS_ENABLE_MSG = 'Please make sure that the news sitemap file of ' +
    'this site could be accepted by Google News';
var NEED_RESTART_WEBSERVER_CONFIRM_MSG = 'This change may need restart ' +
    'webserver to take effect, do you still want to change it?';
var ROBOTS_INCLUDE_CONFIRM_MSG = 'This function will modify your robots.txt' +
    ' file.\nAre you sure to continue?';
var SITE_NOT_RUNNING = 'Site services are not running';
var SERVER_NOT_REACHABLE = 'Server is unreachable!';

var BROWSER_REQUIRE_MSG = 'Please use one of the following browser: ';
var REFRESH_CONFIRM_MSG =
    'Refreshing pages will syncronize with the server\n' +
    'and abandon all your unsaved edits.\n' +
    'Are you sure you want to continue?';
var REFRESH_DONE_MSG = 'Refresh is done';

var CANCEL_CUSTOMIZE_SETTING_MSG =
    'Are you sure you want to abandon your customized settings?';

var NEW_PASSWORDS_CONFLICT =
    'Input passwords are not equal, please enter again';

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

var GLOBAL_SETTING_NAME = 'Global Setting';

//////////////////////////////////////////////////////////////////////
var SettingEditorLanguage = {};

SettingEditorLanguage.title = 'Site Setting - Google Sitemap Generator';

SettingEditorLanguage.values = {
  changePassword: 'change',
  confirm: 'OK',
  cancel: 'Cancel',
  updateRuntime: 'Refresh\nRuntime Info',
  submit: 'Save',
  refresh: 'Refresh',
  restart: 'Save & Restart',
  logout: 'Logout',
  login: 'Login'
};

SettingEditorLanguage.tips = {
  addGeneratorInfo: 'add version information of Google Sitemap Generator ' +
      'into the generated sitemap',
  logPath: 'The log path of the site',
  host: 'The host name of the site',
  robotsIncluded: 'Submit sitemaps through the robots.txt file',
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
      'sitemap files of the service',
  changeLoginPassword: 'Change the Login password',
  changePassword: 'Input new password here if you want to change it',
  remoteAccess: 'Allow remote computer to access the settings page',
  loginPassword: 'Login password',
  login: 'Login',
  autoAdd: 'Add new site from web server automatically (need restart).',
  InAndExUrlPattern: 'The URLs that match patterns in \'Included Urls\' will ' +
      'be added to sitemap, while the URLs that match patterns in \'Excluded ' +
      'Urls\' will never be added to sitemap. If neither of them is set, all ' +
      'the URLs that match other limitations will add to sitemap. If both ' +
      'are set, the URLs that match both will be excluded, while the URLs ' +
      'that match neither will be excluded too. The pattern start with / and ' +
      'mark * means anything.\neg. /*example*',
  websitemapurl: 'Http URL to submit the web sitemap.\neg.' +
      'http: //www.example.com/ping?sitemap=',
  urlFind: 'Find the URL that you do not want to add to sitemap without ' +
      'change, the content that in the [] will be replaced by other content ' +
      'before add to sitemap file.\neg./abc[12*]xyz[45]',
  urlReplace: 'The content with the [] in \'replace\' field will be used to ' +
      'replace the content in \'find\' field with the [].',
  deleteRule: 'Delete this rule.',
  addRule: 'Add a new rule.',
  backupDuration: 'How long to backup the sitemap files.',
  settingPort: 'Which port the Google Sitemap Generator is listening ' +
      'for the configuration.',
  siteEnabled: 'If checked, Google Sitemap Generator will generate sitemaps ' +
      'for the site.',
  webserverFilterEnabled: 'If checked, Google Sitemap Generator will use web ' +
      'server filter to collect URLs for the site.',
  blogSearchPingEnabled: 'If checked, Google Sitemap Generator will send ' +
      'ping to search engine for new blog URL.',
  fileScannerEnabled: 'If checked, Google Sitemap Generator will use file ' +
      'scanner to collect URLs for the site.',
  logParserEnabled: 'If checked, Google Sitemap Generator will use log ' +
      'parser to collect URLs for the site.',
  maxUrlLife: 'How long will the URLs be alive.',
  maxUrlInMemory: 'The max URL numbers that Google Sitemap Generator can ' +
      'store in the memory.',
  maxUrlInDisk: 'The max URL numbers that Google Sitemap Generator can ' +
      'store in all the sitemap files.',
  sitemapEnabled: 'If checked, Google Sitemap Generator will generate the ' +
      'sitemap.',
  compress: 'If checked, the generated sitemap files will be compressed.',
  filename: 'The sitemap file name. If there are more than one sitemap file, ' +
      'it will be the sitemap index file name.',
  UpdateStartTime: 'The start time of this sitemap service.',
  UpdateDuration: 'It defines how long the sitemap file will be regenerated.',
  UpdateDurationForBlogSearchPing: 
      'It defines how long the ping for blog search will be sent',
  UpdateDurationForFileScanner: 
      'It defines how long the file scanner wait to run again',
  UpdateDurationForLogParser: 
      'It defines how long the log parser wait to run again',
  maxFileUrl: 'You can use URL number to limit each sitemap file\'s size.',
  maxFileSize: 'You can use disk size to limit each sitemap file\'s size.',
  newsExpireDuration: 
      'The expire time for URLs that can add to news sitmap file.',
  customize: 'If checked, this part of setting will be synchronized to ' +
      'global setting.',
  submit: 'Save setting edit to Google Sitemap Generator',
  refresh: 'Get newest settings from Google Sitemap Generator',
  restart: 'Save setting edit and restart Google Sitemap Generator service',
  logout: 'Logout from this editor',
  runtimeShow: 'Go to Runtime Info page',
  generalShow: 'Go to site setting page',
  webShow: 'Go to web sitemap setting page',
  mobileShow: 'Go to mobile sitemap setting page',
  codeSearchShow: 'Go to code search sitemap setting page',
  blogSearchShow: 'Go to blog search ping setting page',
  videoShow: 'Go to video sitemap setting page',
  newsShow: 'Go to news sitemap setting page',
  globalLink: 'Go to Global setting page',
  siteLink: 'Go to this site setting page'
};

SettingEditorLanguage.texts = {
  addGeneratorInfo: 'Add generator information into sitemap file',
  webserverFilterEnabled: 'Enable <b>Webserver Filter</b>',
  fileScannerEnabled: 'Enable File Scanner, ',
  logParserEnabled: 'Enable Log Parser, ',
  logPath: 'Log Path: ',
  host: 'Host Name: ',
  robotsIncluded: 'Include sitemap URL in robots.txt',
  inputOldPassword: 'Input old password: ',
  inputNewPassword: 'Input new password',
  inputPasswordAgain: 'Input again',
  loggingLevel: 'Logging Level: ',
  loggingLevelNormal: 'Normal',
  loggingLevelImportant: 'Important',
  loggingLevelCritical: 'Critical',
  loggingLevelError: 'Error',
  runtimeInfo: 'Runtime Info',
  runtimeInfoTitle: 'Runtime Info for site',
  applicationInfoTitle: 'Application Runtime Information',
  blogSearchPingServiceInfoTitle: 'Blog Search Ping Service: ',
  logParserServiceInfoTitle: 'Log Parser Service: ',
  mobileSitemapServiceInfoTitle: 'Mobile Sitemap Service: ',
  codeSearchSitemapServiceInfoTitle: 'Code Search Sitemap Service: ',
  videoSitemapServiceInfoTitle: 'Video Sitemap Service: ',
  newsSitemapServiceInfoTitle: 'News Sitemap Service: ',
  webSitemapServiceInfoTitle: 'Web Sitemap Service: ',
  fileScannerServiceInfoTitle: 'File Scanner Service: ',
  webServerFilterServiceInfoTitle: 'Webserver Filter Service: ',
  ServiceInfoSuccess: 'Last Running Result: ',
  ServiceInfoLastUpdate: 'Last Running Time: ',
  ServiceInfoUrlsCount: 'Last Update URLs Count: ',
  urlInDatabase: 'URLs in database: ',
  urlInTempfile: 'URLs in tempfile: ',
  urlInMemory: 'URLs in memory: ',
  hostName: 'Host Name: ',
  memoryUsageForSite: 'Memory Used For All Services: ',
  diskUsageForSite: 'Disk Used For All Services: ',
  memoryUsage: 'Memory Used for internal DB: ',
  diskUsage: 'Disk Used for internal DB: ',
  startTime: 'Start Running Time: ',
  globalTitle: 'Global Setting',
  //webserverFilterTitle: 'Webserver Filter Setting',
  //fileScannerTitle: 'File Scanner Setting',
  //logParserTitle: 'Log Parser Setting',
  blogSearchPingTitle: 'Blog Search Ping Setting',
  password: 'Login password: ',
  changePassword: 'change',
  remoteAccess: 'Allow Remote Administration',
  loginUsername: 'Username: ',
  loginPassword: 'Password: ',
  siteTitle: 'Site Setting',
  customize: 'Inherited from "Global Setting"',
  autoAdd: 'Auto add websites from webserver',
  backupDuration: 'Backup duration: ',
  secondUnit: 'seconds',
  minuteUnit: 'minutes',
  settingPort: 'Configuration port: ',
  urlPattern: 'URL Pattern: ',
  url: 'URL: ',
  find: 'Find: ',
  replace: 'Replace: ',
  enabled: 'Enable this sitemap generation',
  siteEnabled: 'Enable the sitemap generation of this site',
  maxUrlLife: 'Discard URL older than: ',
  dayUnit: 'days',
  maxUrlInMemory: 'Max cached URL in memory: ',
  equal: ' equals to ',
  megaByteUnit: ' MB',
  maxUrlInDisk: 'Max URL stored on disk: ',
  UrlReplaceList: 'Replaced URLs: ',
  webSitemapTitle: 'Web Sitemap Setting',
  videoSitemapTitle: 'Video Sitemap Setting',
  mobileSitemapTitle: 'Mobile Sitemap Setting',
  codeSearchSitemapTitle: 'Code Search Sitemap Setting',
  compress: 'Compress sitemap file',
  filename: 'Sitemap file name: ',
  UpdateStartTime: 'Update sitemap file from: ',
  UpdateDuration: 'Update sitemap file every: ',
  UpdateDurationForBlogSearchPing: 'Execute Ping every ',
  UpdateDurationForFileScanner: 'Execute <b>File Scanner</b> every ',
  UpdateDurationForLogParser: 'Execute <b>Log Parser</b> every ',
  maxFileUrl: 'Max URL number per sitemap file: ',
  maxFileSize: 'Max file size per sitemap file: ',
  byteUnit: ' bytes',
  kbyteUnit: ' KB',
  IncludedUrlList: 'Included URLs: ',
  ExcludedUrlList: 'Excluded URLs: ',
  NotifyUrlList: 'Notify following search engine URLs: ',
  newsSitemapTitle: 'News Sitemap Setting',
  newsExpireDuration: 'Exclude news older than: ',
  editorTitle: 'Site Setting Editor',
  siteList: 'Site List',
  generalSitemap: 'General Setting',
  webSitemap: 'Web',
  mobileSitemap: 'Mobile',
  codeSearchSitemap: 'Code Search',
  blogSearchPing: 'Blog Search',
  videoSitemap: 'Video',
  newsSitemap: 'News',
  sitemap: 'Sitemaps: ',
  example: 'example: '
};
