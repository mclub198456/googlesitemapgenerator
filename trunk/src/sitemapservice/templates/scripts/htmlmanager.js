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
 * @fileoverview This class manages html document,
 * any 'document' related code must in this class.
 * @author chaiying@google.com (Ying Chai)
 */

/**
 * @constructor
 */
function TabManager(){
  /**
   *
   */
  this.siteListItems_ = {};

  /**
   * @type {SiteSetting?}
   */
  this.setting_ = null;

  /**
   *
   */
  this.tabPages_ = {};

  /**
   * @type {String?} current site id (suffix), the value is GLOBAL_SETTING_ID 
   *     or index string of site in XML.
   */
  this.currentSiteId_ = null;

  /**
   * Since tabs are not all sitemaps, so we can't just use sitemapType to
   * represent the current tab.
   * @type {TabPage.types?} current tab id (no site id suffix)
   */
  this.currentTabType_ = null;

  this.tabHeights_ = {};
}

/**
 * Initializes the object.
 */
TabManager.prototype.init = function() {
  var that = this;
  Util.object.apply(TabPage.types, function(type) {
    that.tabPages_[type] = new TabPage(type);
  });


  var siteId, tabId;
  var cookie = new Cookie(TAB_COOKIE);
  if (cookie.tabId && cookie.siteId) {
    /**
     * When user press 'F5' or click the refresh button of browser, we will
     * remember the current site and tab with the help of cookie.
     */
    siteId = cookie.siteId;
    tabId = cookie.tabId;

  } else {
    /**
     * When user login, the cookie will be clear. So user will go to the
     * global setting's 'site setting' tab.
     */
    siteId = GLOBAL_SETTING_ID;
    tabId = TabPage.types.GENERAL;
  }
  this.setCurrentSite(siteId);
  this.setCurrentTab(tabId);
};

/**
 * Gets the TabManager singleton object.
 * @return {TabManager} The TabManager singleton object
 */
TabManager.getInstance = function() {
  if (TabManager.getInstance.instance_ == null) {
    TabManager.getInstance.instance_ = new TabManager();
  }
  return TabManager.getInstance.instance_;
};

TabManager.prototype.tabHeightChanged = function() {
  var tabHeights = this.tabHeights_[this.currentSiteId_];
  
  // update the cache
  tabHeights[this.currentTabType_] = 
      this.tabPages_[this.currentTabType_].getPageHeight();
      
  PageManager.getInstance().adjustPageHeight();
};

/**
 * Gets current HTML tab page content height.
 */
TabManager.prototype.getCurrentTabHeight = function() {
  var tabHeights = this.tabHeights_[this.currentSiteId_];
  if (!tabHeights) { // initialize
    tabHeights = [];
    this.tabHeights_[this.currentSiteId_] = tabHeights;
  }

  if (this.currentTabType_ == null) {
    return 0;
  }
  
  // using cache if it exists
  if (!tabHeights[this.currentTabType_]) { // calc the height
    tabHeights[this.currentTabType_] =
        this.tabPages_[this.currentTabType_].getPageHeight();
  }
  return tabHeights[this.currentTabType_];
};

/**
 * Adds site list item element to the manager.
 * @param {Element} siteListItem  The HTML element of the site list item
 * @param {String} siteId  The site id of the site list item
 */
TabManager.prototype.addSiteListItem = function(siteListItem, siteId) {
  this.siteListItems_[siteId] = siteListItem;
};

/**
 * Sets current tab type.
 * @param {TabPage.types} type  The type of the tab page
 */
TabManager.prototype.setCurrentTab = function(type) {
  if (this.currentTabType_ == type)
    return;

  // deal with old one
  if (this.currentTabType_ != null)
    this.tabPages_[this.currentTabType_].hide();

  // switch to new one
  this.currentTabType_ = type;
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    cookie.tabId = type;
    cookie.store(ONE_DAY);
  }

  this.tabPages_[this.currentTabType_].show();
};

/**
 * Gets current tab object.
 * @return {TabPage} Current Tab page object
 */
TabManager.prototype.currentTab = function() {
  return this.tabPages_[this.currentTabType_];
};

/**
 * Saves current site setting.
 */
TabManager.prototype.saveCurrentSite = function() {
  /* save the old site */
  // save it even if it is global, since other site may load from it
  // So global may save twice when click save button from normal site page.
  if (this.setting_)
    this.setting_.save();
};
/**
 * Sets current site id. Don't forget to save the site before switch it if
 * needed.
 * @param {String} siteId  GLOBAL_SETTING_ID and index string of site in XML
 */
TabManager.prototype.setCurrentSite = function(siteId) {
  if (this.currentSiteId_ == siteId)
    return;

  /* switch current site id and save it to cookie */
  var oldSiteId = this.currentSiteId_;
  this.currentSiteId_ = siteId;
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    cookie.siteId = siteId;
    cookie.store(ONE_DAY);
  }

  /* update the siteList */
  // change the sitelist element display
  Util.CSS.changeClass(this.siteListItems_[this.currentSiteId_], 
                       INACTIVE_SITE, ACTIVE_SITE);
  if (oldSiteId != null)
    Util.CSS.changeClass(this.siteListItems_[oldSiteId], 
                         ACTIVE_SITE, INACTIVE_SITE);

  /* Load the new site */
  // must call from sitesettings since the site need switch dom tree
  SiteSettings.getInstance().load(null, this.currentSiteId_);
  this.setting_ = SiteSettings.getInstance().site();
  var curSetting = this.setting_;
  Util.object.apply(this.tabPages_, function(tabPage) {
    // set setting to each tab page
    tabPage.setting_ = curSetting.settingTab(tabPage.type_);
  });

  RuntimeManager.getInstance().load();

  /* switch the tab pages */
  PageManager.getInstance().switchSite(this.currentSiteId_);
};

/**
 * @return {String} Current site Id
 */
TabManager.prototype.currentSite = function() {
  return this.currentSiteId_;
};


/////////////////////////// TabPage Class ///////////////////
/**
 * @constructor
 * @param {TabPage.types} type  The type of this tab page
 */
function TabPage(type) {
  // switcher is a TD HTML element
  this.switcher_ = Util.checkElemExistAndReturn(TabPage.switcherIds[type]);

  /**
   * @type {Element?}
   */
  this.page_ = PageManager.getInstance().tabs_[type];;

  /**
   * @type {TabPage.types}
   */
  this.type_ = type;

  /**
   * @type {SitemapSetting|SiteSetting|null}
   */
  this.setting_ = null;

  var link = Util.checkUniqueAndReturn(this.switcher_, 'A');
  link.href = '#';
  Util.event.add(link, 'click', function() {
    // when switch tab, check the old tab.valid

    var perf = new Perf('switch tab');
    if (TabManager.getInstance().currentTab().validate()) {
    	TabManager.getInstance().setCurrentTab(type);
    } else {
      // alert user to choose stay in the tab or discard the change.
      if (confirm(DISCARD_CHANGE_TO_CURRENT_TAB)) {
        // discard the change
        TabManager.getInstance().currentTab().load();
    	  TabManager.getInstance().setCurrentTab(type);
      }
    }
    perf.check('done');
    perf.report();
  });
}


/**
 * Different Tabs
 * @enum
 */
TabPage.types = {
  GENERAL: 0, WEB: 1, NEWS: 2, VIDEO: 3, MOBILE: 4, CODESEARCH: 5,
  BLOGSEARCH: 6, RUNTIME: 7};

/**
 * The tab page switcher's HTML id
 * @type {Array.<String>}
 */
TabPage.switcherIds = [];
TabPage.switcherIds[TabPage.types.GENERAL] = 'generalSitemapShow';
TabPage.switcherIds[TabPage.types.WEB] = 'webSitemapShow';
TabPage.switcherIds[TabPage.types.NEWS] = 'newsSitemapShow';
TabPage.switcherIds[TabPage.types.VIDEO] = 'videoSitemapShow';
TabPage.switcherIds[TabPage.types.MOBILE] = 'mobileSitemapShow';
TabPage.switcherIds[TabPage.types.CODESEARCH] = 'codeSearchSitemapShow';
TabPage.switcherIds[TabPage.types.BLOGSEARCH] = 'blogSearchPingShow';
TabPage.switcherIds[TabPage.types.RUNTIME] = 'runtimeInfoShow';

/**
 * Checks if this tab page's setting is valid.
 * @return {Boolean} If this tab page's setting is valid, return true
 */
TabPage.prototype.validate = function() {
  if (this.type_ == TabPage.types.RUNTIME) {
    return true;
  }

  // when switch tab, check the old tab.valid
  return this.setting_.validate();
};

/**
 * Loads this tab page's setting.
 */
TabPage.prototype.load = function() {
  this.setting_.load();
};

/**
 * Gets this tab page's height.
 * @return {Number} The tab page's height.
 */
TabPage.prototype.getPageHeight = function() {
  return this.page_.offsetHeight;
};

/**
 * Shows this tab page to user.
 */
TabPage.prototype.show = function() {
  Util.CSS.changeClass(this.switcher_, INACTIVE_TAB, ACTIVE_TAB);

  Util.CSS.showElement(this.page_);
  PageManager.getInstance().adjustPageHeight();

  if (this.setting_) {
    this.setting_.focusOnFirst();
  }
};

/**
 * Hides this tab page from user.
 */
TabPage.prototype.hide = function() {
  Util.CSS.changeClass(this.switcher_, ACTIVE_TAB, INACTIVE_TAB);
  Util.CSS.hideElement(this.page_);
};


////////////////////////////PageManager////////////////////////////////////
/**
 * @constructor
 */
function PageManager() {
  this.currentType_ = PageManager.statusType.NONE;
  
  this.loginUsername_ = Util.checkElemExistAndReturn('loginUsername');
  this.loginPassword_ = Util.checkElemExistAndReturn('loginPassword');
  this.loginPage_ = Util.checkElemExistAndReturn('loginPage');
  this.cmdBtnArea_ = Util.checkElemExistAndReturn('cmdbuttons');
  this.menuBar_ = Util.checkElemExistAndReturn('hMenu');
  this.appSettingPage_ = Util.checkElemExistAndReturn('appSettings');
  this.sitesSettingPage_ = Util.checkElemExistAndReturn('content');
  this.siteListTpl_ = Util.checkElemExistAndReturn('siteListTemplate');
  this.linkBar_ = Util.checkElemExistAndReturn('linkbar');
  this.appSettingsLink_ = Util.checkElemExistAndReturn('appSettingsLink');
  this.siteSettingsLink_ = Util.checkElemExistAndReturn('siteSettingsLink');

  this.siteListContainer_ = Util.checkElemExistAndReturn('siteListContainer');
  this.settingPageBody_ =
      Util.checkElemExistAndReturn('sitemapContainerDiv');

  this.tabs_ = [];
  this.tabs_[TabPage.types.GENERAL] =
      Util.checkElemExistAndReturn('generalSitemapBody');
  this.tabs_[TabPage.types.WEB] =
      Util.checkElemExistAndReturn('webSitemapBody');
  this.tabs_[TabPage.types.NEWS] =
      Util.checkElemExistAndReturn('newsSitemapBody');
  this.tabs_[TabPage.types.VIDEO] =
      Util.checkElemExistAndReturn('videoSitemapBody');
  this.tabs_[TabPage.types.MOBILE] =
      Util.checkElemExistAndReturn('mobileSitemapBody');
  this.tabs_[TabPage.types.CODESEARCH] =
      Util.checkElemExistAndReturn('codeSearchSitemapBody');
  this.tabs_[TabPage.types.BLOGSEARCH] =
      Util.checkElemExistAndReturn('blogSearchPingBody');
  this.tabs_[TabPage.types.RUNTIME] =
      Util.checkElemExistAndReturn('runtimeInfoBody');
      
  this.globalSection_ = [];
  this.globalSection_[TabPage.types.GENERAL] =
      Util.checkElemExistAndReturn('generalSitemapBodyGlobal');
  this.globalSection_[TabPage.types.WEB] =
      Util.checkElemExistAndReturn('webSitemapBodyGlobal');
  this.globalSection_[TabPage.types.NEWS] =
      Util.checkElemExistAndReturn('newsSitemapBodyGlobal');
  this.globalSection_[TabPage.types.VIDEO] =
      Util.checkElemExistAndReturn('videoSitemapBodyGlobal');
  this.globalSection_[TabPage.types.MOBILE] =
      Util.checkElemExistAndReturn('mobileSitemapBodyGlobal');
  this.globalSection_[TabPage.types.CODESEARCH] =
      Util.checkElemExistAndReturn('codeSearchSitemapBodyGlobal');
  this.globalSection_[TabPage.types.BLOGSEARCH] =
      Util.checkElemExistAndReturn('blogSearchPingBodyGlobal');
  this.globalSection_[TabPage.types.RUNTIME] =
      Util.checkElemExistAndReturn('runtimeInfoBodyGlobal');
      
  this.siteSection_ = [];
  this.siteSection_[TabPage.types.GENERAL] =
      Util.checkElemExistAndReturn('generalSitemapBodySite');
  this.siteSection_[TabPage.types.WEB] =
      Util.checkElemExistAndReturn('webSitemapBodySite');
  this.siteSection_[TabPage.types.NEWS] =
      Util.checkElemExistAndReturn('newsSitemapBodySite');
  this.siteSection_[TabPage.types.VIDEO] =
      Util.checkElemExistAndReturn('videoSitemapBodySite');
  this.siteSection_[TabPage.types.MOBILE] =
      Util.checkElemExistAndReturn('mobileSitemapBodySite');
  this.siteSection_[TabPage.types.CODESEARCH] =
      Util.checkElemExistAndReturn('codeSearchSitemapBodySite');
  this.siteSection_[TabPage.types.BLOGSEARCH] =
      Util.checkElemExistAndReturn('blogSearchPingBodySite');
  this.siteSection_[TabPage.types.RUNTIME] =
      Util.checkElemExistAndReturn('runtimeInfoBodySite');
      
//  this.commonSection_ = [];
//  this.commonSection_[TabPage.types.GENERAL] =
//      Util.checkElemExistAndReturn('commonGeneralSitemapBody');
//  this.commonSection_[TabPage.types.WEB] =
//      Util.checkElemExistAndReturn('commonWebSitemapBody');
//  this.commonSection_[TabPage.types.NEWS] =
//      Util.checkElemExistAndReturn('commonNewsSitemapBody');
//  this.commonSection_[TabPage.types.VIDEO] =
//      Util.checkElemExistAndReturn('commonVideoSitemapBody');
//  this.commonSection_[TabPage.types.MOBILE] =
//      Util.checkElemExistAndReturn('commonMobileSitemapBody');
//  this.commonSection_[TabPage.types.CODESEARCH] =
//      Util.checkElemExistAndReturn('commonCodeSearchSitemapBody');
//  this.commonSection_[TabPage.types.BLOGSEARCH] =
//      Util.checkElemExistAndReturn('commonBlogSearchPingBody');
}

/**
 * Gets the PageManager singleton object.
 * @return {PageManager} The PageManager singleton object
 */
PageManager.getInstance = function() {
  if (PageManager.getInstance.instance_ == null) {
    PageManager.getInstance.instance_ = new PageManager();
  }
  return PageManager.getInstance.instance_;
};

/**
 * Current page mode that show to user.
 * @enum
 */
PageManager.statusType = {NONE: 0, LOGIN: 1, APPLICATION: 2, SITES: 3};

/**
 * Gets the sites pages container.
 * @return {HTMLElement}  The sites pages container
 */
PageManager.prototype.getSitesBodyContainer = function() {
  return this.settingPageBody_;
};

/**
 * Shows login page.
 */
PageManager.prototype.showLoginPage = function() {
  this.switchPageType(PageManager.statusType.LOGIN);
  
  // clear the cookie for remembering the site and tab when user login.
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    cookie.remove();
  } 
  
  this.loginUsername_.value = 'admin';

  // focus to password
  this.loginPassword_.focus();
};

/**
 * Shows setting page.
 * If user has login, it will be called after page load, otherwise, it will be
 * called after user pass login.
 */
PageManager.prototype.showSettingPage = function() {
  // hide login page and show setting page  
  this.switchPageType(PageManager.statusType.SITES);

  // Notes: don't change the sequence
  // Gets XML from server
  Controller.updateSettingData();

  // update setting page with the data from server
  Controller.updateRuntimeInfo();

  // create setting page, must call after updateSettingData, need to know
  // how many sites to set.
  this.generateSettingPage();

  // must call it after generateSettingPage, updateSettingData and
  // updateRuntimeInfo
  TabManager.getInstance().init();
};

/**
 * Switchs the page mode between application settings and sites settings.
 */
PageManager.prototype.switchSettingPage = function() {
  if (this.currentType_ == PageManager.statusType.SITES) {
    this.switchPageType(PageManager.statusType.APPLICATION);
  } else if (this.currentType_ == PageManager.statusType.APPLICATION) {
    this.switchPageType(PageManager.statusType.SITES);
  }
};

/**
 * Switchs the page mode that show to users.
 * @param {PageManager.statusType} type  The page mode
 */
PageManager.prototype.switchPageType = function(type) {
  // hide old page
  switch (this.currentType_) {
    case PageManager.statusType.LOGIN:
      Util.CSS.hideElement(this.loginPage_);
      break;
    case PageManager.statusType.APPLICATION:
      Util.CSS.hideElement(this.cmdBtnArea_);
      Util.CSS.hideElement(this.linkBar_);
      Util.CSS.hideElement(this.appSettingPage_);
      break;
    case PageManager.statusType.SITES:
      Util.CSS.hideElement(this.cmdBtnArea_);
      Util.CSS.hideElement(this.linkBar_);
      Util.CSS.hideElement(this.menuBar_);
      Util.CSS.hideElement(this.sitesSettingPage_);
      Util.CSS.hideElement(this.siteListContainer_);
      break;
    case PageManager.statusType.NONE:
      Util.CSS.hideElement(this.loginPage_);
      Util.CSS.hideElement(this.cmdBtnArea_);
      Util.CSS.hideElement(this.linkBar_);
      Util.CSS.hideElement(this.appSettingPage_);
      Util.CSS.hideElement(this.menuBar_);
      Util.CSS.hideElement(this.sitesSettingPage_);
      Util.CSS.hideElement(this.siteListContainer_);
      break;
    default:
      Util.console.error('Invalid page type');
  }
  
  // show new page
  switch (type) {
    case PageManager.statusType.LOGIN:
      Util.CSS.showElement(this.loginPage_);
      break;
    case PageManager.statusType.APPLICATION:
      Util.CSS.showElement(this.cmdBtnArea_);
      Util.CSS.showElement(this.linkBar_);
      Util.CSS.showElement(this.appSettingPage_);
      
      Util.CSS.addClass(this.appSettingsLink_, READONLY_CLASS);
      this.appSettingsLink_.removeAttribute('href');

      Util.CSS.removeClass(this.siteSettingsLink_, READONLY_CLASS);
      this.siteSettingsLink_.href = '#';
      break;
    case PageManager.statusType.SITES:
      Util.CSS.showElement(this.cmdBtnArea_);
      Util.CSS.showElement(this.menuBar_);
      Util.CSS.showElement(this.sitesSettingPage_);
      Util.CSS.showElement(this.linkBar_);
      Util.CSS.showElement(this.siteListContainer_);

      Util.CSS.addClass(this.siteSettingsLink_, READONLY_CLASS);
      this.siteSettingsLink_.removeAttribute('href');

      Util.CSS.removeClass(this.appSettingsLink_, READONLY_CLASS);
      this.appSettingsLink_.href = '#';
      break;
    default:
      Util.console.error('Invalid page type');
  }
  
  this.currentType_ = type;
};

/**
 * let the container has the same height of the content, and let the list
 * area and setting area have the same height.
 *
 * If sub divs' position is 'relative', the parent div will calculate its height
 * according to the sum of the sub divs; but if sub divs' position is
 * 'absolute', the parent div will set its height to 0.
 *
 */
PageManager.prototype.adjustPageHeight = function() {
  // Find the max value between site list height and tab page height
  var settingHeight = TabManager.getInstance().getCurrentTabHeight();
  
  // Using cache, since the length will not change.
  if (!this.listHeight_) {
    this.listHeight_ = this.siteListContainer_.offsetHeight;
  }
  var listHeight = this.listHeight_;
  var maxHeight = Math.max(listHeight, settingHeight);

  // adjust to the history max height, which is the height for the max height
  // tab page. It can prevent changing frequently when switch the tab page.
  if (this.historyHeight_ == null)
    this.historyHeight_ = maxHeight;
  maxHeight = Math.max(this.historyHeight_, maxHeight);
  this.historyHeight_ = maxHeight;

  // adjust the page height
  this.settingPageBody_.style.height = maxHeight;
  this.sitesSettingPage_.style.height = maxHeight;
};

/**
 * Set HTML elements' event handlers, for static elements in main.html.
 * Should be called once the page is loaded.
 */
PageManager.InitElementEventHandlerStatic = function() {
  // ensure that the code is called only once to avoid add duplicate
  // event handler to the elements.
  if (PageManager.InitElementEventHandlerStatic.isDone) {
    alert('error, InitElementEventHandlerStatic is called twice');
    return;
  }
  PageManager.InitElementEventHandlerStatic.isDone = true;

  Util.event.addEnterKey('loginPassword', Controller.onLogin);
  Util.event.addEnterKey('inputPasswordAgain', function() {
    PasswordManager.getInstance().confirm();
  });

  Util.event.add(Util.checkElemExistAndReturn('submit'), 'click', 
                 Controller.onSubmitXml);
  Util.event.add(Util.checkElemExistAndReturn('restart'), 'click', 
                 Controller.onRestart);

  var fn = function(e, target) {
    if (target.href) {
      PageManager.getInstance().switchSettingPage();
    }
    return false; // always return false as event handler
  };
  var link = Util.checkElemExistAndReturn('appSettingsLink');
  link.href = '#';
  Util.event.add(link, 'click', fn);
  
  link = Util.checkElemExistAndReturn('siteSettingsLink');
  link.href = '#';
  Util.event.add(link, 'click', fn);
  
  link = Util.checkElemExistAndReturn('projHomeLink');
  link.href = 'http://code.google.com/p/googlesitemapgenerator/';
  
  link = Util.checkElemExistAndReturn('helpLink');
  link.href = 'http://googlesitemapgenerator.googlecode.com/svn/trunk/doc/' +
              'gsg-intro.html';
  
  link = Util.checkElemExistAndReturn('logoutLink');
  link.href = '#';
  Util.event.add(link, 'click', Controller.onLogout);

  var inputs = document.getElementsByTagName('INPUT');
  Util.array.apply(inputs, function(input) {
    if (input.type == 'button') {
      if (Util.CSS.isClass(input, 'login')) {
        Util.event.add(input, 'click', Controller.onLogin);
      } else if (Util.CSS.isClass(input, 'updateRuntime')) {
        Util.event.add(input, 'click', function() {
          Controller.updateRuntimeInfo();
        });
      } else if (Util.CSS.isClass(input, 'confirmPasswordChange')) {
        Util.event.add(input, 'click', function() {
          PasswordManager.getInstance().confirm();
        });
      } else if (Util.CSS.isClass(input, 'cancelPasswordChange')) {
        Util.event.add(input, 'click', function() {
          PasswordManager.getInstance().close();
        });
      }
    }
  });
  PasswordManager.getInstance();
};



/**
 * Add language related info to the page.
 * Called right after static page is loaded, and when the setting page is
 * dynamic generated
 */
PageManager.setupLanguage = function() {
  // set text to spans
  var spans = document.getElementsByTagName('SPAN');
  Util.array.apply(spans, function(span) {
    TransMarkSet.transLanguageForSpanElem(span);
  });

  // set tips and values to inputs
  var inputs = document.getElementsByTagName('INPUT');
  Util.array.apply(inputs, function(input) {
    TransMarkSet.transLanguageForInputElem(input);
  });

  // set tips to links
  var links = document.getElementsByTagName('A');
  Util.array.apply(links, function(link) {
    TransMarkSet.transLanguageForLinkElem(link);
  });

  document.title = SettingEditorLanguage.title;
};

/**
 * Gets the login username.
 * @return {String} The login username
 */
PageManager.prototype.getLoginUsername = function() {
  return this.loginUsername_.value;
};

/**
 * Gets the login password.
 * @return {String} The login password
 */
PageManager.prototype.getLoginPassword = function() {
  return this.loginPassword_.value;
};

/////////////////////  generate html /////////////////////////////
/**
 * Generates the global setting page and only one suite of site page.
 * No need to the XML content.
 */
PageManager.prototype.generateSettingPage = function() {
  // ensure the function only execute once
  if (PageManager.generated == true) {
    return;
  }

  /* generate site Tab for only normal site runtime info page */
  PageManager.generateRuntimeTab();

  /* generate site list */
  var div = document.createElement('DIV');

  // for Global setting
  div.appendChild(this.generateSiteList(GLOBAL_SETTING_ID));

  // for site setting
  var num = SiteSettings.getInstance().sitesNum();
  for (var i = 0; i < num; i++) {
    div.appendChild(this.generateSiteList(i.toString()));
  }

  this.siteListContainer_.appendChild(div);
};

/**
 * Switch to a new site.
 * @param {String} siteId  The id of the new site
 */
PageManager.prototype.switchSite = function(siteId) {
  if (siteId == GLOBAL_SETTING_ID) { // hide site, show global
    var tabs = this.siteSection_;
    Util.array.apply(tabs, function(tab) {      
      Util.CSS.hideElement(tab);
    });

    tabs = this.globalSection_;
    Util.array.apply(tabs, function(tab) {      
      Util.CSS.showElement(tab);
    });
  } else { // hide global, show site
    var tabs = this.globalSection_;
    Util.array.apply(tabs, function(tab) {      
      Util.CSS.hideElement(tab);
    });

    tabs = this.siteSection_;
    Util.array.apply(tabs, function(tab) {      
      Util.CSS.showElement(tab);
    });    
  }
	this.adjustPageHeight();
};

/**
 * Generates the runtime tab page.
 */
PageManager.generateRuntimeTab = function() {
  // generate runtime info page for site
  // find the runtime Tab
  var rtBody = Util.checkElemExistAndReturn('runtimeInfoContainer');

  // find the runtime Tpl for service
  var rtInfoTpl = Util.checkElemExistAndReturn('runtimeInfoTpl');

  // add service runtime to the Tab
  function addRuntimeServiceInfo(service) {
    // copy the body
    var serviceBody = rtInfoTpl.cloneNode(true);

    // preprocess the Id and Transmark
    var serviceName = service.name();
    PageManager.addServiceNameToIdAndTransmark_(serviceBody, serviceName);

    // append the body to Tab
    rtBody.appendChild(serviceBody);
  }

  // call addRuntimeServiceInfo for each service
  Util.object.apply(ServiceSetting.types, function(service) {
    if (service != ServiceSetting.types.WEBSERVERFILTER) {
    	addRuntimeServiceInfo(service);
    }
  });
  
  // set text to spans
  var spans = rtBody.getElementsByTagName('SPAN');
  Util.array.apply(spans, function(span) {
    TransMarkSet.transLanguageForSpanElem(span);
  });
};

/**
 * Add service name to ids and transmarks in the HTML template clone.
 * @param {Object} root  The template clone
 * @param {Object} name  The service name
 * @private
 */
PageManager.addServiceNameToIdAndTransmark_ = function(root, name) {
  Util.DOM.applyToDomTree(root, function(node) {
    // add service name to 'id' attribute
    if (Util.object.hasProperty(node, 'id') && node.id != 'undefined' &&
        node.id != '') {
      // change the 'id' for blog search, which is different from others
      if (name == 'blogSearchPing') {
        switch (node.id) {
          case 'ServiceInfoLastUpdate':
            node.id = 'ServiceInfoLastPing';
            break;
          case 'ServiceInfoUrlsCount':
            node.id = 'ServiceInfoLastUrl'
            break;
        }
      }
      node.id = name + node.id;
    }
    // add service name to transmark, only 'text:ServiceInfoTitle' need various,
    // other transmark are all the same among services
    if (Util.DOM.hasAttribute(node, TRANSMARK_ATTRNAME)) {
      var markset = node.getAttribute(TRANSMARK_ATTRNAME);

      function addName(type, str) {
        // Up to now, only used by runtimeinfo templates, and only transmark
        // for text:ServiceInfoTitle need add prefix service name.
        // If we want to add name prefix to all the transmark, set the condition
        // to 'str ?'
        return str == 'ServiceInfoTitle' ? name + str : str;
      }

      node.setAttribute(TRANSMARK_ATTRNAME,
                        TransMarkSet.modifyMark(markset, addName));
    }
  });
};

/**
 * Generate site html content according to the html template.
 *
 * @param {String} siteId  The logical ID of the site
 */
PageManager.prototype.generateSiteList = function(siteId) {
  /**
   * @type {String} siteName  The logical name of the site
   */
  var siteName = SiteSettings.getInstance().siteName(siteId);

  /**
   * @type {String} siteLinkTip  The tip of the site link
   */
  var siteLinkTip = siteId == GLOBAL_SETTING_ID ?
                    SettingEditorLanguage.tips.globalLink :
                    SettingEditorLanguage.tips.siteLink;

  // generate one site list item from copying the template
  var siteListItem = this.siteListTpl_.cloneNode(true);
  siteListItem.id = '';
  
  // To improve performance, using className instead of 
  // Util.CSS.removeClass(siteListItem, HIDDEN_CLASS)
  // make sure that the className is set to correct value
  siteListItem.className = 'inactiveSite';

  // find the link
  var alink = siteListItem.rows.item(1).cells.item(1).firstChild;
  
  // To improve performance, using onclick instead of Util.event.add
  // make sure only one function for the 'onclick' event, and no param
  // needed.
  alink.onclick = function() {
    // Generally speaking, it should check all tabs in the site.
    // But since program will check the current tab when user switch to another,
    // so we can assume that all the other tabs in the site have been checked.
    // (we assume the value from server's xml is correct)
    var mng = TabManager.getInstance();
    if (mng.currentTab().validate()) {
      mng.saveCurrentSite();
      mng.setCurrentSite(siteId);
    } else {
      if (confirm(DISCARD_CHANGE_TO_CURRENT_TAB)) {
        // discard the change, don't save
        mng.setCurrentSite(siteId);
      }
    }
  };
  Util.event.handlers_.push({elem:alink, name:'onclick'});

  // To improve performance, first check the length
  alink.innerHTML = siteName.length > MAX_SITELIST_WIDTH ?
      PageManager.addWrapMark(siteName) : siteName;

  // add tips
  alink.tip = siteLinkTip;
  Tips.schedule(alink);

  if (siteId == GLOBAL_SETTING_ID) {
    Util.CSS.addClass(alink, SITELIST_GLOBALSETTING_CSS_CLASS);
  }

  TabManager.getInstance().addSiteListItem(siteListItem, siteId);
  return siteListItem;
};

/**
 * add '- ' to the too-long word in the name, to trigger the auto wrap.
 * @param {String} name  The site name
 * @return {String} The name with '-' in the wrap positions
 */
PageManager.addWrapMark = function(name) {
  var words = name.split(/ /);
  var newWords = [];
  Util.array.apply(words, function(word) {
    if (word.length > MAX_SITELIST_WIDTH) {
      var pos = 0;
      var slices = [];
      var max_sitelist_width_minus_one = MAX_SITELIST_WIDTH - 1;
      while (word.length - pos > MAX_SITELIST_WIDTH) {
        slices.push(word.substr(pos, max_sitelist_width_minus_one));
        pos += max_sitelist_width_minus_one;
      }
      slices.push(word.substr(pos));
      word = slices.join('- ');
    }
    newWords.push(word);
  });
  return newWords.join(' ');
};


///////////////////////////////PasswordManager//////////////////////////////////
/**
 * @constructor
 */
function PasswordManager() {
  /**
   *
   */
  this.page_ = Util.checkElemExistAndReturn('changePasswordDiv');

  /**
   * The 'change' link behind the password readonly setting field
   * @type {String}
   */
  var link = Util.checkElemExistAndReturn('changePassword');
  link.href = '#';
  var that = this;
  Util.event.add(link, 'click', function() {
    that.open();
  });
  this.change_ = link;


  /**
   * The password readonly setting field
   * @type {ELement}
   */
  this.password_ = Util.checkElemExistAndReturn('password');

   /**
   * The old password input field
   * @type {ELement}
   */
  this.oldPassword_ =
      Util.checkElemExistAndReturn('inputOldPassword');

  /**
   * The new password input field
   * @type {ELement}
   */
  this.newPassword_ =
      Util.checkElemExistAndReturn('inputNewPassword');

  /**
   * The new password input again field
   * @type {ELement}
   */
  this.newPassword2_ = Util.checkElemExistAndReturn('inputPasswordAgain');

  /**
   *
   */
  this.error_ = Util.checkElemExistAndReturn('changePasswordError');
}

/**
 * Gets the PasswordManager singleton object.
 * @return {PasswordManager}  The PasswordManager singleton object
 */
PasswordManager.getInstance = function() {
  if (PasswordManager.getInstance.instance_ == null) {
    PasswordManager.getInstance.instance_ = new PasswordManager();
  }
  return PasswordManager.getInstance.instance_;
};

/**
 * Opens the change-password box.
 */
PasswordManager.prototype.open = function(){
  this.change_.disabled = true;
  Util.CSS.showElement(this.page_);
  this.oldPassword_.focus();
};

/**
 * Close the change-password box.
 */
PasswordManager.prototype.close = function(){
  Util.CSS.hideElement(this.page_);
  this.clear();
  this.change_.disabled = false;
};

/**
 * Clear the change-password box's input fields.
 */
PasswordManager.prototype.clear = function(){
  this.oldPassword_.value = '';
  this.newPassword_.value = '';
  this.newPassword2_.value = '';
};

/**
 * Checks the new passwords' consistency, submits the password change.
 */
PasswordManager.prototype.confirm = function() {
  if (!ValidateManager.validators.PASSWORD.check(this.newPassword_, false)) {
    this.error_.innerHTML = NEW_PASSWORDS_INVALID;
    Util.CSS.showElement(this.error_);
    this.clear();
    this.oldPassword_.focus();
    return;
  }
  
  if (this.newPassword_.value != this.newPassword2_.value) {
    this.error_.innerHTML = NEW_PASSWORDS_CONFLICT;
    Util.CSS.showElement(this.error_);
    this.clear();
    this.oldPassword_.focus();
    return;
  }
  
  // request to server
  if (!ServerManager.requestChangePassword(this.oldPassword_.value,
                                           this.newPassword_.value)) {
    this.error_.innerHTML = ServerManager.errorMsg;
    Util.CSS.showElement(this.error_);
    this.clear();
    this.oldPassword_.focus();
    return;
  }
  
  // change succeed
  this.password_.value = this.newPassword_.value;
  this.close();
};
