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
var languageFlag = 'zh-cn';

// server response inform messages
var SAVE_SUCCESS_MSG = '设定已被保存。';
var SAVE_FAIL_MSG = '设定保存失败！';
var SAVE_WARNING_MSG = '设定已过期，是否继续保存？';
var REFRESH_MSG = '是否刷新页面以获取最新设定？';
var RESTART_SUCCESS_MSG = 'Google Sitemap Generator服务重启成功！';
var RESTART_FAIL_MSG = 'Google Sitemap Generator服务重启失败！';
var RESTART_WARNING_MSG = '请手动重启网站服务器，以使更改生效';
var CHPSWD_SUCCESS_MSG = '修改密钥成功';
var CHPSWD_FAIL_MSG = '修改密钥失败';
var CHPSWD_FAIL_MSG1 = '修改密钥失败，旧密钥不正确';
var CHPSWD_FAIL_MSG2 = '因服务器原因，修改密钥失败';

var LOGOUT_SUCCESS_MSG = '注销成功！';
var LOGOUT_FAIL_MSG = '注销失败！';
var LOGIN_SUCCESS_MSG = '登录成功！';
var LOGIN_FAIL_MSG = '登录失败！如果您是第一次登陆，请尝试密码\'admin\'。在您第一次' +
    '登录成功后，请不要忘记修改初始密码。';

// guide messages
var NEWS_ENABLE_MSG = '请确认您的sitemap会被Google News接受';
var NEED_RESTART_WEBSERVER_CONFIRM_MSG = '对该项配置所作的修改将需要重启网站服务器' +
    '才能生效，您确定要修改吗？'
var ROBOTS_INCLUDE_CONFIRM_MSG = '该功能会修改本站点的robots.txt文件，你确定要' +
    '继续吗？';
var SITE_NOT_RUNNING = '该站点的生成工具服务未运行';
var SERVER_NOT_REACHABLE = '服务器不可达!';
var BROWSER_REQUIRE_MSG = '对您目前使用的游览器我们不提供支持，推荐使用下列游览器: ';
var REFRESH_CONFIRM_MSG = '刷新页面将丢失未保存的修改，确定要继续吗?';
var REFRESH_DONE_MSG = '刷新页面已完成';
var CANCEL_CUSTOMIZE_SETTING_MSG = '与全局设定同步将丢失所有已作的修改，确定要同步吗?';
var NEW_PASSWORDS_CONFLICT = '两次输入密码不匹配，请重新输入';
var NEW_PASSWORDS_INVALID = '密码长度不符合要求，请重新输入';
var VALIDATING_ONSAVE_FAIL_MSG = '设定中有错误，请更改高亮显示的设定。'
var VALIDATING_FAIL_MSG = '输入错误，请更正。';
var DISCARD_CHANGE_TO_CURRENT_TAB = '某些设定有误，要取消当前页已作的所有更改吗？';
var SAVE_RESTART_MSG = '确定要重启服务器吗?';
var WAITING_FOR_SERVER_RESTART_MSG = '正在等待服务器重启...';

// setting labels
var ADD_BUTTON_VALUE = '添加';
var DELETE_BUTTON_VALUE = '删除';

var GLOBAL_SETTING_NAME = '默认设定';

////////////////////////////////////////////////////////////////////////
var SettingEditorLanguage = {};

/**
 * The title of the browser window.
 */
SettingEditorLanguage.title = '谷歌Sitemap生成器设置';

/**
 * The 'value' attributes of the buttons.
 */
SettingEditorLanguage.values = {
  changePassword: '修改',
  confirm: '确定',
  cancel: '取消',
  updateRuntime: '刷新',
  submit: '保存设置',
  refresh: '刷新',
  restart: '重启服务',
  logout: '注销',
  login: '登录'
};

/**
 * The tooltips of the settings.
 */
SettingEditorLanguage.tips = {
  addGeneratorInfo: '将Google Sitemap生成工具的版本信息添加到所生成的sitemap中',
  logPath: '该站点的日志路径',
  host: '该站点的域名',
  robotsIncluded: '通过robots.txt来提交Sitemap文件',
  updateRuntime: '刷新运行时信息',
  loggingLevel: '服务器端运行日志的等级',
  urlInDatabase: '生成工具数据库内URL的数量',
  urlInTempfile: '临时文件中URL的数量',
  urlInMemory: '内存中URL的数量',
  hostName: '当前站点的名称 (最常用的)',
  memoryUsageForSite: '当前站点服务消耗了多少内存',
  diskUsageForSite: '当前站点服务消耗了多少磁盘空间',
  memoryUsage: '生成工具的内部数据库占用了多少内存空间',
  diskUsage: '生成工具的内部数据库占用了多少磁盘空间',
  startTime: '生成工具开始运行时间',
  ServiceInfoSuccess: '显示上一次运行是否成功',
  ServiceInfoLastUpdate: '上次运行的时间',
  ServiceInfoUrlsCount: '生成的Sitemap文件中包含的URL数量',
  changeLoginPassword: '修改登录密码',
  changePassword: '可在此设置新的登录密码',
  remoteAccess: '允许远程访问设置页面',
  loginPassword: '登录密码',
  login: '登录',
  autoAdd: '从网站服务器自动添加新的站点（需重启）',
  InAndExUrlPattern: '如果这两个配置项都没有设置，则所有的URL将被收入。如果这两个' +
      '配置项都被设置了，那么只要URL匹配排除模式或者不匹配包含模式，都会被排除出' +
      'sitemap，只有匹配包含模式且不匹配排除模式的URL才会被收入。 配置项以/开始，' +
      '星号*可以是任意长度的任何值。\n例子：/*example*',
  websitemapurl: '提交网页Sitemap的地址\n例子：' +
      'http: //www.example.com/ping?sitemap=',
  urlFind: '找出特殊的URL并加以替换，使得Sitemap文件中不包含不必要的信息。在[]中的内容' +
      '将被下面对应的[]中的值替换。\n例子：/abc[12*]xyz[45]',
  urlReplace: '该项中[]里的内容将会替换上面对应[]里的值',
  deleteRule: '删除当前规则',
  addRule: '添加一条新规则',
  backupDuration: '每隔多少时间将内存中的URLs备份到硬盘上',
  settingPort: '配置端口是多少',
  siteEnabled: '控制该站点的Google Sitemap Generator服务',
  siteEnabled_true: '启动该站点的Google Sitemap Generator服务',
  siteEnabled_false: '停止该站点的Google Sitemap Generator服务',
  webserverFilterEnabled: '控制该站点的Google Sitemap Generator服务使用' +
      '网站服务器过滤插件来获取URL',
  webserverFilterEnabled_true: '开启该站点的网站服务器过滤插件',
  webserverFilterEnabled_false: '关闭该站点的网站服务器过滤插件',
  fileScannerEnabled: '控制该站点的Google Sitemap Generator服务使用' +
      '文件系统扫描模块来获取URL',
  fileScannerEnabled_true: '开启该站点的文件系统扫描模块',
  fileScannerEnabled_false: '关闭该站点的文件系统扫描模块',
  logParserEnabled: '控制该站点的Google Sitemap Generator服务使用' +
      '网站日志分析模块来获取URL',
  logParserEnabled_true: '开启该站点的网站日志分析模块',
  logParserEnabled_false: '关闭该站点的网站日志分析模块',
  blogSearchPingEnabled: '是否启用博客搜索Ping服务',
  maxUrlLife: 'URL的有效时间是多久，即包含于Sitemap中的最早的URL时间期限',
  maxUrlInMemory: '允许生成工具放在内存中的URL数量上限',
  maxUrlInDisk: '允许生成工具放在磁盘中的URL数量上限',
  sitemapEnabled: '是否生成该Sitemap',
  compress: '是否压缩Sitemap文件',
  filename: 'Sitemap文件名是什么。对于多Sitemap文件，该名称为索引文件名。',
  UpdateStartTime: '该项Sitemap服务启动时间',
  UpdateDuration: 'Sitemap更新频率',
  UpdateDurationForBlogSearchPing: '博客搜索Ping服务运行间隔',
  UpdateDurationForFileScanner: '文件系统扫描服务运行间隔',
  UpdateDurationForLogParser: '网站日志扫描服务运行间隔',
  maxFileUrl: 'Sitemap文件中所能包含的URL数量上限',
  maxFileSize: 'Sitemap文件大小上限（未压缩）',
  newsExpireDuration: '新闻Sitemap中URL的过期淘汰时间',
  customize: '是否设为默认值',
  submit: '保存当前设置到服务器',
  refresh: '刷新当前设置',
  restart: '重启Google Sitemap Generator服务（某些选项修改会需要重启网站服务器）',
  logout: '注销',
  runtimeShow: '转到运行信息页面',
  generalShow: '转到网站设置页面',
  webShow: '转到网页Sitemap设置页面',
  videoShow: '转到视频Sitemap设置页面',
  mobileShow: '转到移动Sitemap设置页面',
  codeSearchShow: '转到代码搜索Sitemap设置页面',
  blogSearchShow: '转到博客搜索Ping服务设置页面',
  newsShow: '转到新闻Sitemap设置页面',
  globalLink: '转到全局设置页面',
  siteLink: '转到该站点设置页面'
};

/**
 * The text labels' display contents.
 */
SettingEditorLanguage.texts = {
  passwordHints: '新密码长度至少要6位',
  URLSources: 'URL来源',
  appSettingsLink: '应用程序设置',
  siteSettingsLink: '站点设置',
  projHomeLink: '主页',
  helpLink: '帮助',
  logoutLink: '注销',
  addGeneratorInfo: '为sitemap添加生成工具信息',
  webserverFilterEnabled: '网站服务器过滤插件',
  webserverFilterEnabled_true: '开启',
  webserverFilterEnabled_false: '关闭',
  fileScannerEnabled: '文件系统扫描模块',
  fileScannerEnabled_true: '开启',
  fileScannerEnabled_false: '关闭',
  logParserEnabled: '网站日志分析模块',
  logParserEnabled_true: '开启',
  logParserEnabled_false: '关闭',
  logPath: '日志路径：',
  host: '域名：',
  robotsIncluded: '将sitemap URL添加到robots.txt文件中',
  inputOldPassword: '输入旧密码：',
  inputNewPassword: '输入新密码：',
  inputPasswordAgain: '再次输入：',
  loggingLevel: '运行日志的记录级别: ',
  loggingLevelNormal: '普通',
  loggingLevelImportant: '重要',
  loggingLevelCritical: '关键',
  loggingLevelError: '错误',
  runtimeInfo: '运行情况',
  runtimeInfoTitle: '当前站点的运行情况',
  applicationInfoTitle: '程序运行情况',
  logParserServiceInfoTitle: '日志分析服务: ',
  mobileSitemapServiceInfoTitle: '移动Sitemap生成服务: ',
  codeSearchSitemapServiceInfoTitle: '代码搜索Sitemap生成服务: ',
  videoSitemapServiceInfoTitle: '视频Sitemap生成服务: ',
  newsSitemapServiceInfoTitle: '新闻Sitemap生成服务: ',
  webSitemapServiceInfoTitle: '网页Sitemap生成服务: ',
  fileScannerServiceInfoTitle: '文件扫描服务: ',
  blogSearchPingServiceInfoTitle: '博客搜索Ping服务: ',
  webServerFilterServiceInfoTitle: '网站插件服务: ',
  ServiceInfoSuccess: '最近一次运行结果: ',
  ServiceInfoLastUpdate: '最近一次运行时间: ',
  ServiceInfoUrlsCount: '最近一次更新的地址数量: ',
  urlInDatabase: 'URL数量（数据库）：',
  urlInTempfile: 'URL数量（临时文件）：',
  urlInMemory: 'URL数量（内存）：',
  hostName: '域名: ',
  memoryUsageForSite: '内存占用总量: ',
  diskUsageForSite: '磁盘空间占用总量: ',
  memoryUsage: '内存占用（程序数据库）: ',
  diskUsage: '磁盘空间占用（程序数据库）: ',
  startTime: '开始运行时间: ',
  appTitle: '应用程序设置',
  blogSearchPingTitle: '博客搜索Ping服务设置',
  password: '登录密码：',
  changePassword: '修改',
  remoteAccess: '远程访问',
  loginUsername: '用户名: ',
  loginPassword: '密码: ',
  siteTitle: '一般项设置',
  customize: '设为默认值',
  autoAdd: '从网站服务器自动添加站点',
  backupDuration: 'URL自动保存间隔: ',
  secondUnit: ' 秒',
  minuteUnit: ' 分',
  settingPort: '配置端口: ',
  urlPattern: '地址模式: ',
  url: '地址链接: ',
  find: '查找: ',
  replace: '替换: ',
  webEnabled: '网页Sitemap生成',
  newsEnabled: '新闻Sitemap生成',
  videoEnabled: '视频Sitemap生成',
  mobileEnabled: '移动Sitemap生成',
  codeSearchEnabled: '代码搜索Sitemap生成',
  blogSearchEnabled: '博客搜索Sitemap生成',
  siteEnabled: 'Sitemap生成服务：',
  webEnabledSite: '为该站点生成网页Sitemap',
  newsEnabledSite: '为该站点生成新闻Sitemap',
  videoEnabledSite: '为该站点生成视频Sitemap',
  mobileEnabledSite: '为该站点生成移动Sitemap',
  codeSearchEnabledSite: '为该站点生成代码搜索Sitemap',
  blogSearchEnabledSite: '为该站点生成博客搜索Sitemap',
  siteEnabledSite: '该站点的sitemap生成服务：',
  siteEnabled_true: '启动',
  siteEnabled_false: '停止',
  maxUrlLife: '包含在网站地图文件中的地址不早于: ',
  dayUnit: ' 天前',
  maxUrlInMemory: '缓存在内存中的地址数量上限: ',
  equal: ' 等于 ',
  megaByteUnit: ' 兆字节',
  maxUrlInDisk: '存储在硬盘上的地址数量上限: ',
  UrlReplaceList: '替换地址的规则',
  webSitemapTitle: '网页Sitemap设置',
  videoSitemapTitle: '视频Sitemap设置',
  mobileSitemapTitle: '移动Sitemap设置',
  codeSearchSitemapTitle: '代码搜索Sitemap设置',
  compress: '压缩Sitemap文件',
  filename: 'Sitemap文件名称: ',
  UpdateStartTime: 'Sitemap文件更新起始时间: ',
  UpdateDuration: '更新Sitemap文件的时间间隔: ',
  UpdateDurationForBlogSearchPing: '执行时间间隔: ',
  UpdateDurationForFileScanner: '执行文件扫描服务时间间隔: ',
  UpdateDurationForLogParser: '执行日志分析服务时间间隔: ',
  maxFileUrl: '单个Sitemap文件的地址数量上限: ',
  maxFileSize: '单个Sitemap文件的大小上限: ',
  byteUnit: ' 字节',
  kbyteUnit: ' 千字节',
  IncludedUrlList: '应包含的地址模式',
  ExcludedUrlList: '应排除的地址模式',
  NotifyUrlList: '需要通知的搜索引擎地址',
  newsSitemapTitle: '新闻Sitemap设置',
  newsExpireDuration: 'Sitemap中的内容不早于: ',
  editorTitle: '站点配置管理工具',
  siteList: '网站列表',
  generalSitemap: '通用设置',
  webSitemap: '网页',
  videoSitemap: '视频',
  mobileSitemap: '移动',
  codeSearchSitemap: '代码搜索',
  blogSearchPing: '博客搜索',
  newsSitemap: '新闻',
  sitemap: 'Sitemap: ',
  example: '例如: '
};
