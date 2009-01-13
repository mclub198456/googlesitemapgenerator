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
var languageFlag = 'zh-cn';

// server response inform messages
var DIRTY_WARNING_MSG = '淇敼杩樻湭淇濆瓨锛岃缁х画閲嶅惎鍚楋紵'
var PASSWD_WRONG_MSG = '瀵嗙爜涓嶆纭�';
var SAVE_SUCCESS_MSG = '璁惧畾宸茶淇濆瓨銆�;
var SAVE_FAIL_MSG = '璁惧畾淇濆瓨澶辫触锛�;
var SAVE_WARNING_MSG = '璁惧畾宸茶繃鏈燂紝鏄惁缁х画淇濆瓨锛�;
var REFRESH_MSG = '鏄惁鍒锋柊椤甸潰浠ヨ幏鍙栨渶鏂拌瀹氾紵';
var RELOAD_SUCCESS_MSG = 'Google Sitemap Generator鏈嶅姟閲嶅惎鎴愬姛锛�;
var RELOAD_FAIL_MSG = 'Google Sitemap Generator鏈嶅姟閲嶅惎澶辫触锛�;
var RELOAD_WARNING_MSG = '璇锋墜鍔ㄩ噸鍚綉绔欐湇鍔″櫒锛屼互浣挎洿鏀圭敓鏁�;
var CHPSWD_SUCCESS_MSG = '淇敼瀵嗛挜鎴愬姛';
var CHPSWD_FAIL_MSG = '淇敼瀵嗛挜澶辫触';
var CHPSWD_FAIL_MSG1 = '淇敼瀵嗛挜澶辫触锛屾棫瀵嗛挜涓嶆纭�;
var CHPSWD_FAIL_MSG2 = '鍥犳湇鍔″櫒鍘熷洜锛屼慨鏀瑰瘑閽ュけ璐�;

var LOGOUT_SUCCESS_MSG = '娉ㄩ攢鎴愬姛锛�;
var LOGOUT_FAIL_MSG = '娉ㄩ攢澶辫触锛�;
var LOGIN_SUCCESS_MSG = '鐧诲綍鎴愬姛锛�;
var LOGIN_FAIL_MSG = '鐧诲綍澶辫触锛佸鏋滄偍鏄涓�鐧婚檰锛岃灏濊瘯瀵嗙爜\'admin\'銆傚湪鎮ㄧ涓�' +
    '鐧诲綍鎴愬姛鍚庯紝璇蜂笉瑕佸繕璁颁慨鏀瑰垵濮嬪瘑鐮併�';

// guide messages
var NEWS_ENABLE_MSG = '璇风‘璁ゆ偍鐨剆itemap浼氳Google News鎺ュ彈';
var NEED_RELOAD_WEBSERVER_CONFIRM_MSG = '瀵硅椤归厤缃墍浣滅殑淇敼灏嗛渶瑕侀噸鍚綉绔欐湇鍔″櫒' +
    '鎵嶈兘鐢熸晥锛屾偍纭畾瑕佷慨鏀瑰悧锛�
var ROBOTS_INCLUDE_CONFIRM_MSG = '璇ュ姛鑳戒細淇敼鏈珯鐐圭殑robots.txt鏂囦欢锛屼綘纭畾瑕� +
    '缁х画鍚楋紵';
var SITE_NOT_RUNNING = '璇ョ珯鐐圭殑Google Sitemap Generator鏈嶅姟缁勪欢鏈繍琛�;
var SERVER_NOT_REACHABLE = '鏈嶅姟鍣ㄤ笉鍙揪!';
var BROWSER_REQUIRE_MSG = '瀵规偍鐩墠浣跨敤鐨勬父瑙堝櫒鎴戜滑涓嶆彁渚涙敮鎸侊紝鎺ㄨ崘浣跨敤涓嬪垪娓歌鍣� ';
var REFRESH_DONE_MSG = '鍒锋柊椤甸潰宸插畬鎴�;
var CANCEL_CUSTOMIZE_SETTING_MSG = '涓庡叏灞�瀹氬悓姝ュ皢涓㈠け鎵�湁宸蹭綔鐨勪慨鏀癸紝纭畾瑕佸悓姝ュ悧?';
var NEW_PASSWORDS_CONFLICT = '涓ゆ杈撳叆瀵嗙爜涓嶅尮閰嶏紝璇烽噸鏂拌緭鍏�;
var NEW_PASSWORDS_INVALID = '瀵嗙爜闀垮害涓嶇鍚堣姹傦紝璇烽噸鏂拌緭鍏�;
var VALIDATING_ONSAVE_FAIL_MSG = '璁惧畾涓湁閿欒锛岃鏇存敼楂樹寒鏄剧ず鐨勮瀹氥�'
var VALIDATING_FAIL_MSG = '杈撳叆閿欒锛岃鏇存銆�;
var DISCARD_CHANGE_TO_CURRENT_TAB = '鏌愪簺璁惧畾鏈夎锛岃鍙栨秷褰撳墠椤靛凡浣滅殑鎵�湁鏇存敼鍚楋紵';

// setting labels
var GLOBAL_SETTING_NAME = '榛樿璁惧畾';

////////////////////////////////////////////////////////////////////////
var GSGLang = {};

/**
 * The title of the browser window.
 */
GSGLang.title = '璋锋瓕Sitemap鐢熸垚鍣ㄨ缃�;

/**
 * The 'value' attributes of the buttons.
 */
GSGLang.values = {
  changePassword: '淇敼',
  confirm: '纭畾',
  cancel: '鍙栨秷',
  updateRuntime: '鍒锋柊',
  submit: '淇濆瓨璁剧疆',
  reload: '閲嶅惎鏈嶅姟',
  logout: '娉ㄩ攢',
  login: '鐧诲綍',
  add: '娣诲姞',
  del: '鍒犻櫎'
};

/**
 * The tooltips of the settings.
 */
GSGLang.tips = {
  addGeneratorInfo: '灏咷oogle Sitemap鐢熸垚宸ュ叿鐨勭増鏈俊鎭坊鍔犲埌鎵�敓鎴愮殑sitemap涓�,
  logPath: '璇ョ珯鐐圭殑鏃ュ織璺緞',
  host: '璇ョ珯鐐圭殑鍩熷悕',
  robotsIncluded: '閫氳繃robots.txt鏉ユ彁浜itemap鏂囦欢',
  updateRuntime: '鍒锋柊杩愯鏃朵俊鎭�,
  urlInDatabase: '鐢熸垚宸ュ叿鏁版嵁搴撳唴URL鐨勬暟閲�,
  urlInTempfile: '涓存椂鏂囦欢涓璘RL鐨勬暟閲�,
  urlInMemory: '鍐呭瓨涓璘RL鐨勬暟閲�,
  hostName: '褰撳墠绔欑偣鐨勫悕绉�(鏈�父鐢ㄧ殑)',
  memoryUsageForSite: '褰撳墠绔欑偣鐨凣oogle Sitemap Generator鏈嶅姟缁勪欢娑堣�浜嗗灏戝唴瀛�,
  diskUsageForSite: '褰撳墠绔欑偣鐨凣oogle Sitemap Generator鏈嶅姟缁勪欢娑堣�浜嗗灏戠鐩樼┖闂�,
  memoryUsage: 'Google Sitemap Generator鐨勫唴閮ㄦ暟鎹簱鍗犵敤浜嗗灏戝唴瀛樼┖闂�,
  diskUsage: 'Google Sitemap Generator鐨勫唴閮ㄦ暟鎹簱鍗犵敤浜嗗灏戠鐩樼┖闂�,
  startTime: 'Google Sitemap Generator寮�杩愯鏃堕棿',
  infoStat: '鏄剧ず涓婁竴娆¤繍琛屾槸鍚︽垚鍔�,
  infoTime: '涓婃杩愯鐨勬椂闂�,
  infoUrls: '鐢熸垚鐨凷itemap鏂囦欢涓寘鍚殑URL鏁伴噺',
  changeLoginPassword: '淇敼鐧诲綍瀵嗙爜',
  changePassword: '鍙湪姝よ缃柊鐨勭櫥褰曞瘑鐮�,
  remoteAccess: '鍏佽杩滅▼璁块棶璁剧疆椤甸潰',
  loginPassword: '鐧诲綍瀵嗙爜',
  login: '鐧诲綍',
  autoAdd: '浠庣綉绔欐湇鍔″櫒鑷姩娣诲姞鏂扮殑绔欑偣锛堥渶閲嶅惎锛�,
  InAndExUrlPattern: '濡傛灉杩欎袱涓厤缃」閮芥病鏈夎缃紝鍒欐墍鏈夌殑URL灏嗚鏀跺叆銆傚鏋滆繖涓や釜' +
      '閰嶇疆椤归兘琚缃簡锛岄偅涔堝彧瑕乁RL鍖归厤鎺掗櫎妯″紡鎴栬�涓嶅尮閰嶅寘鍚ā寮忥紝閮戒細琚帓闄ゅ嚭' +
      'sitemap锛屽彧鏈夊尮閰嶅寘鍚ā寮忎笖涓嶅尮閰嶆帓闄ゆā寮忕殑URL鎵嶄細琚敹鍏ャ� 閰嶇疆椤逛互/寮�锛� +
      '鏄熷彿*鍙互鏄换鎰忛暱搴︾殑浠讳綍鍊笺�',
  NotifyUrl: '鎻愪氦缃戦〉Sitemap鐨勫湴鍧��',
  ReplaceUrlPattern:
      '鎵惧嚭鐗规畩鐨刄RL骞跺姞浠ユ浛鎹紝浣垮緱Sitemap鏂囦欢涓笉鍖呭惈涓嶅繀瑕佺殑淇℃伅銆傚湪\'鏌ユ壘\'椤�[])' +
      '涓殑鍐呭灏嗚\'鏇挎崲\'椤�[])涓殑鍊兼浛鎹�',
  deleteRule: '鍒犻櫎褰撳墠瑙勫垯',
  addRule: '娣诲姞涓�潯鏂拌鍒�,
  backupDuration: '姣忛殧澶氬皯鏃堕棿灏嗗唴瀛樹腑鐨刄RLs澶囦唤鍒扮‖鐩樹笂',
  settingPort: '閰嶇疆绔彛鏄灏�,
  siteEnabled: '鎺у埗璇ョ珯鐐圭殑Google Sitemap Generator鏈嶅姟缁勪欢',
  webserverFilterEnabled: '鎺у埗涓鸿绔欑偣鏈嶅姟鐨勭綉绔欐湇鍔″櫒杩囨护鎻掍欢妯″潡',
  fileScannerEnabled: '鎺у埗涓鸿绔欑偣鏈嶅姟鐨勬枃浠剁郴缁熸壂鎻忔ā鍧�,
  logParserEnabled: '鎺у埗涓鸿绔欑偣鏈嶅姟鐨勭綉绔欐棩蹇楀垎鏋愭ā鍧�,
  blogSearchPingEnabled: '鏄惁鍚敤鍗氬鎼滅储Ping鏈嶅姟',
  maxUrlLife: 'URL鐨勬湁鏁堟椂闂存槸澶氫箙锛屽嵆鍖呭惈浜嶴itemap涓殑鏈�棭鐨刄RL鏃堕棿鏈熼檺',
  maxUrlInMemory: '鍏佽鐢熸垚宸ュ叿鏀惧湪鍐呭瓨涓殑URL鏁伴噺涓婇檺',
  maxUrlInDisk: '鍏佽鐢熸垚宸ュ叿鏀惧湪纾佺洏涓殑URL鏁伴噺涓婇檺',
  sitemapEnabled: '鏄惁鐢熸垚璇itemap',
  compress: '鏄惁鍘嬬缉Sitemap鏂囦欢',
  filename: 'Sitemap鏂囦欢鍚嶆槸浠�箞銆傚浜庡Sitemap鏂囦欢锛岃鍚嶇О涓虹储寮曟枃浠跺悕銆�,
  UpdateStartTime: '璇ラ」Sitemap鐢熸垚鍣ㄥ惎鍔ㄦ椂闂�,
  UpdateDuration: 'Sitemap鏇存柊棰戠巼',
  UpdateDurationForBlogSearchPing: '鍗氬鎼滅储Ping鏈嶅姟缁勪欢杩愯闂撮殧',
  UpdateDurationForFileScanner: '鏂囦欢绯荤粺鎵弿鏈嶅姟缁勪欢杩愯闂撮殧',
  UpdateDurationForLogParser: '缃戠珯鏃ュ織鎵弿鏈嶅姟缁勪欢杩愯闂撮殧',
  maxFileUrl: 'Sitemap鏂囦欢涓墍鑳藉寘鍚殑URL鏁伴噺涓婇檺',
  maxFileSize: 'Sitemap鏂囦欢澶у皬涓婇檺锛堟湭鍘嬬缉锛�,
  newsExpireDuration: '鏂伴椈Sitemap涓璘RL鐨勮繃鏈熸窐姹版椂闂�,
  customize: '鏄惁璁句负榛樿鍊�
  //submit: '淇濆瓨褰撳墠璁剧疆鍒版湇鍔″櫒',
  //reload: '閲嶅惎Google Sitemap Generator鏈嶅姟锛堟煇浜涢�椤逛慨鏀逛細闇�閲嶅惎缃戠珯鏈嶅姟鍣級',
};

/**
 * The text labels' display contents.
 */
GSGLang.texts = {
  noSitemapEnabled: '娌℃湁sitemap鍙厤缃�,
  fileLimits: 'Sitemap鏂囦欢璁剧疆',
  ServiceInfoUrlPing: '鏈�悗鍙戦�鐨刄RL鍦板潃',
  promptPassword: '椤甸潰宸茶鍒锋柊锛岄渶瑕佹偍鐨勫瘑鐮佷互淇濆瓨璁剧疆锛岃鍐嶆杈撳叆鐧诲綍瀵嗙爜',
  passwordHints: '鏂板瘑鐮侀暱搴﹁嚦灏戣6浣�,
  URLSources: 'URL鏉ユ簮',
  appSettingsLink: '搴旂敤绋嬪簭璁剧疆',
  siteSettingsLink: '绔欑偣璁剧疆',
  projHomeLink: '椤圭洰涓婚〉',
  logoutLink: '娉ㄩ攢',
  addGeneratorInfo: '涓簊itemap娣诲姞鐢熸垚宸ュ叿淇℃伅',
  webserverFilterEnabled: '缃戠珯鏈嶅姟鍣ㄨ繃婊ゆ彃浠�,
  webserverFilterEnabled_true: '寮�惎',
  webserverFilterEnabled_false: '鍏抽棴',
  fileScannerEnabled: '鏂囦欢绯荤粺鎵弿妯″潡',
  fileScannerEnabled_true: '寮�惎',
  fileScannerEnabled_false: '鍏抽棴',
  logParserEnabled: '缃戠珯鏃ュ織鍒嗘瀽妯″潡',
  logParserEnabled_true: '寮�惎',
  logParserEnabled_false: '鍏抽棴',
  logPath: '鏃ュ織璺緞锛�,
  host: '鍩熷悕锛�,
  robotsIncluded: '灏唖itemap URL娣诲姞鍒皉obots.txt鏂囦欢涓�,
  inputOldPassword: '杈撳叆鏃у瘑鐮侊細',
  inputNewPassword: '杈撳叆鏂板瘑鐮侊細',
  inputPasswordAgain: '鍐嶆杈撳叆锛�,
  loggingLevel: '杩愯鏃ュ織鐨勮褰曠骇鍒� ',
  loggingLevelNormal: '鏅�',
  loggingLevelImportant: '閲嶈',
  loggingLevelCritical: '鍏抽敭',
  loggingLevelError: '閿欒',
  runtimeInfo: '杩愯鎯呭喌',
  runtimeInfoTitle: '褰撳墠绔欑偣鐨勮繍琛屾儏鍐�,
  applicationInfoTitle: '绋嬪簭杩愯鎯呭喌',
  logParserServiceInfoTitle: '鏃ュ織鍒嗘瀽鏈嶅姟缁勪欢: ',
  mobileSitemapServiceInfoTitle: '绉诲姩Sitemap鐢熸垚鍣� ',
  codeSearchSitemapServiceInfoTitle: '浠ｇ爜鎼滅储Sitemap鐢熸垚鍣� ',
  videoSitemapServiceInfoTitle: '瑙嗛Sitemap鐢熸垚鍣� ',
  newsSitemapServiceInfoTitle: '鏂伴椈Sitemap鐢熸垚鍣� ',
  webSitemapServiceInfoTitle: '缃戦〉Sitemap鐢熸垚鍣� ',
  fileScannerServiceInfoTitle: '鏂囦欢鎵弿鏈嶅姟缁勪欢: ',
  blogSearchPingServiceInfoTitle: '鍗氬鎼滅储Ping鏈嶅姟缁勪欢: ',
  webServerFilterServiceInfoTitle: '缃戠珯鎻掍欢鏈嶅姟缁勪欢: ',
  infoStat: '鏈�繎涓�杩愯缁撴灉: ',
  infoTime: '鏈�繎涓�杩愯鏃堕棿: ',
  infoUrls: '鏈�繎涓�鏇存柊鐨勫湴鍧�暟閲� ',
  urlInDatabase: 'URL鏁伴噺锛堟暟鎹簱锛夛細',
  urlInTempfile: 'URL鏁伴噺锛堜复鏃舵枃浠讹級锛�,
  urlInMemory: 'URL鏁伴噺锛堝唴瀛橈級锛�,
  hostName: '鍩熷悕: ',
  memoryUsageForSite: '鍐呭瓨鍗犵敤鎬婚噺: ',
  diskUsageForSite: '纾佺洏绌洪棿鍗犵敤鎬婚噺: ',
  memoryUsage: '鍐呭瓨鍗犵敤锛堢▼搴忔暟鎹簱锛� ',
  diskUsage: '纾佺洏绌洪棿鍗犵敤锛堢▼搴忔暟鎹簱锛� ',
  startTime: '寮�杩愯鏃堕棿: ',
  appTitle: '搴旂敤绋嬪簭璁剧疆',
  blogSearchPingTitle: '鍗氬鎼滅储Ping鏈嶅姟缁勪欢璁剧疆',
  password: '鐧诲綍瀵嗙爜锛�,
  changePassword: '淇敼',
  remoteAccess: '杩滅▼璁块棶',
  loginUsername: '鐢ㄦ埛鍚� ',
  loginPassword: '瀵嗙爜: ',
  siteTitle: '涓�埇椤硅缃�,
  customize_false:'浣跨敤榛樿璁剧疆',
  customize_true: '瀹氬埗',
  autoAdd: '浠庣綉绔欐湇鍔″櫒鑷姩娣诲姞绔欑偣',
  backupDuration: 'URL鑷姩淇濆瓨闂撮殧: ',
  secondUnit: ' 绉�,
  minuteUnit: ' 鍒�,
  settingPort: '閰嶇疆绔彛: ',
  urlPattern: '鍦板潃妯″紡: ',
  url: '鍦板潃閾炬帴: ',
  find: '鏌ユ壘: ',
  replace: '鏇挎崲: ',
  webEnabled: '缃戦〉Sitemap鐢熸垚',
  newsEnabled: '鏂伴椈Sitemap鐢熸垚',
  videoEnabled: '瑙嗛Sitemap鐢熸垚',
  mobileEnabled: '绉诲姩Sitemap鐢熸垚',
  codeSearchEnabled: '浠ｇ爜鎼滅储Sitemap鐢熸垚',
  blogSearchEnabled: '鍗氬鎼滅储Sitemap鐢熸垚',
  siteEnabled: 'Sitemap鐢熸垚鏈嶅姟锛�,
  webEnabledSite: '涓鸿绔欑偣鐢熸垚缃戦〉Sitemap',
  newsEnabledSite: '涓鸿绔欑偣鐢熸垚鏂伴椈Sitemap',
  videoEnabledSite: '涓鸿绔欑偣鐢熸垚瑙嗛Sitemap',
  mobileEnabledSite: '涓鸿绔欑偣鐢熸垚绉诲姩Sitemap',
  codeSearchEnabledSite: '涓鸿绔欑偣鐢熸垚浠ｇ爜鎼滅储Sitemap',
  blogSearchEnabledSite: '涓鸿绔欑偣鐢熸垚鍗氬鎼滅储Sitemap',
  siteEnabledSite: '璇ョ珯鐐圭殑sitemap鐢熸垚鏈嶅姟锛�,
  siteEnabled_true: '鍚姩',
  siteEnabled_false: '鍋滄',
  maxUrlLife: '鍖呭惈鍦ㄧ綉绔欏湴鍥炬枃浠朵腑鐨勫湴鍧�笉鏃╀簬: ',
  dayUnit: ' 澶╁墠',
  maxUrlInMemory: '缂撳瓨鍦ㄥ唴瀛樹腑鐨勫湴鍧�暟閲忎笂闄� ',
  equal: '鍗犵敤绌洪棿澶у皬锛氬ぇ绾�',
  megaByteUnit: ' 鍏嗗瓧鑺�,
  maxUrlInDisk: '瀛樺偍鍦ㄧ‖鐩樹笂鐨勫湴鍧�暟閲忎笂闄� ',
  UrlReplaceList: '鏇挎崲鍦板潃鐨勮鍒�,
  webSitemapTitle: '缃戦〉Sitemap璁剧疆',
  videoSitemapTitle: '瑙嗛Sitemap璁剧疆',
  mobileSitemapTitle: '绉诲姩Sitemap璁剧疆',
  codeSearchSitemapTitle: '浠ｇ爜鎼滅储Sitemap璁剧疆',
  compress: '鍘嬬缉Sitemap鏂囦欢',
  filename: 'Sitemap鏂囦欢鍚嶇О: ',
  UpdateStartTime: 'Sitemap鏂囦欢鏇存柊璧峰鏃堕棿: ',
  UpdateDuration: '鏇存柊Sitemap鏂囦欢鐨勬椂闂撮棿闅� ',
  UpdateDurationForBlogSearchPing: '鎵ц鏃堕棿闂撮殧: ',
  UpdateDurationForFileScanner: '鎵ц鏂囦欢鎵弿鏈嶅姟鏃堕棿闂撮殧: ',
  UpdateDurationForLogParser: '鎵ц鏃ュ織鍒嗘瀽鏈嶅姟鏃堕棿闂撮殧: ',
  maxFileUrl: '鍦板潃鏁伴噺涓婇檺: ',
  maxFileSize: '鏂囦欢澶у皬涓婇檺: ',
  byteUnit: ' 瀛楄妭',
  kbyteUnit: ' 鍗冨瓧鑺�,
  IncludedUrlList: '搴斿寘鍚殑鍦板潃妯″紡',
  ExcludedUrlList: '搴旀帓闄ょ殑鍦板潃妯″紡',
  NotifyUrlList: '闇�閫氱煡鐨勬悳绱㈠紩鎿庡湴鍧�,
  newsSitemapTitle: '鏂伴椈Sitemap璁剧疆',
  newsExpireDuration: 'Sitemap涓殑鍐呭涓嶆棭浜� ',
  editorTitle: '绔欑偣閰嶇疆绠＄悊宸ュ叿',
  siteList: '缃戠珯鍒楄〃',
  generalSitemap: '閫氱敤璁剧疆',
  webSitemap: '缃戦〉',
  videoSitemap: '瑙嗛',
  mobileSitemap: '绉诲姩',
  codeSearchSitemap: '浠ｇ爜鎼滅储',
  blogSearchPing: '鍗氬鎼滅储',
  newsSitemap: '鏂伴椈',
  sitemap: 'Sitemap: ',
  dateExample: '渚嬪: 2008-08-31 12:34:56'
};
