// Copyright 2007 Google Inc.
// All Rights Reserved.

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
 * Gets current HTML tab page content height.
 */
TabManager.prototype.getCurrentTabHeight = function() {
  return this.tabPages_[this.currentTabType_].getPageHeight();
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
  this.currentSiteId_ = siteId;
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    cookie.siteId = siteId;
    cookie.store(ONE_DAY);
  }

  /* update the siteList */
  // a HTML table
  var currentListItem = this.siteListItems_[this.currentSiteId_];

  // change the sitelist element display
  // currentListItem.parentNode is the list
  var liArray = currentListItem.parentNode.getElementsByTagName('table');
  for (var i = 0; i < liArray.length; i++) {
    if (liArray[i] == currentListItem)
      Util.CSS.changeClass(liArray[i], INACTIVE_SITE, ACTIVE_SITE);
    else
      Util.CSS.changeClass(liArray[i], ACTIVE_SITE, INACTIVE_SITE);
  }

  /* Load the new site */
  // must call from sitesettings since the site need switch dom tree
  SiteSettings.getInstance().load(null, this.currentSiteId_);
  RuntimeManager.getInstance().load();

  /* set current SiteSetting and html page */
  // switch the tab manager
  PageManager.getInstance().switchSite(this.currentSiteId_);
  this.setting_ = SiteSettings.getInstance().site();

  /* switch the tab pages */
  // hide the old page
  if (this.currentTabType_ != null)
    this.tabPages_[this.currentTabType_].hide();

  var that = this;
  Util.object.apply(this.tabPages_, function(tabPage) {
    // set setting to each tab page
    tabPage.setting_ = that.setting_.settingTab(tabPage.type_);
    tabPage.page_ = PageManager.getInstance().curTabs_[tabPage.type_];
  });
  if (this.currentTabType_ != null)
    this.tabPages_[this.currentTabType_].show();
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
  this.page_ = null;

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
  var that = this;
  Util.event.add(link, 'click', function() {
    // when switch tab, check the old tab.valid

    var perf = new Perf('switch tab');
    if (TabManager.getInstance().currentTab().validate()) {
    	TabManager.getInstance().setCurrentTab(that.type_);
    } else {
      // alert user to choose stay in the tab or discard the change.
      if (confirm(DISCARD_CHANGE_TO_CURRENT_TAB)) {
        // discard the change
        TabManager.getInstance().currentTab().load();
    	  TabManager.getInstance().setCurrentTab(that.type_);
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
  this.loginUsername_ = Util.checkElemExistAndReturn('loginUsername');
  this.loginPassword_ = Util.checkElemExistAndReturn('loginPassword');
  this.loginPage_ = Util.checkElemExistAndReturn('loginPage');
  this.cmdBtnArea_ = Util.checkElemExistAndReturn('cmdbuttons');
  this.menuBar_ = Util.checkElemExistAndReturn('hMenu');
  this.settingPage_ = Util.checkElemExistAndReturn('content');
  this.siteListTpl_ = Util.checkElemExistAndReturn('siteListTemplate');

  this.siteListContainer_ = Util.checkElemExistAndReturn('siteList');
  this.settingPageBody_ =
      Util.checkElemExistAndReturn('sitemapContainerDiv');

  this.curTabs_ = null;
  this.global_ = {tabs_: []};
  this.global_.tabs_[TabPage.types.GENERAL] =
      Util.checkElemExistAndReturn('generalSitemapBodyg');
  this.global_.tabs_[TabPage.types.WEB] =
      Util.checkElemExistAndReturn('webSitemapBodyg');
  this.global_.tabs_[TabPage.types.NEWS] =
      Util.checkElemExistAndReturn('newsSitemapBodyg');
  this.global_.tabs_[TabPage.types.VIDEO] =
      Util.checkElemExistAndReturn('videoSitemapBodyg');
  this.global_.tabs_[TabPage.types.MOBILE] =
      Util.checkElemExistAndReturn('mobileSitemapBodyg');
  this.global_.tabs_[TabPage.types.CODESEARCH] =
      Util.checkElemExistAndReturn('codeSearchSitemapBodyg');
  this.global_.tabs_[TabPage.types.BLOGSEARCH] =
      Util.checkElemExistAndReturn('blogSearchPingBodyg');
  this.global_.tabs_[TabPage.types.RUNTIME] =
      Util.checkElemExistAndReturn('runtimeInfoBodyg');
  this.site_ = {tabs_: []};
  this.site_.tabs_[TabPage.types.GENERAL] =
      Util.checkElemExistAndReturn('generalSitemapBody');
  this.site_.tabs_[TabPage.types.WEB] =
      Util.checkElemExistAndReturn('webSitemapBody');
  this.site_.tabs_[TabPage.types.NEWS] =
      Util.checkElemExistAndReturn('newsSitemapBody');
  this.site_.tabs_[TabPage.types.VIDEO] =
      Util.checkElemExistAndReturn('videoSitemapBody');
  this.site_.tabs_[TabPage.types.MOBILE] =
      Util.checkElemExistAndReturn('mobileSitemapBody');
  this.site_.tabs_[TabPage.types.CODESEARCH] =
      Util.checkElemExistAndReturn('codeSearchSitemapBody');
  this.site_.tabs_[TabPage.types.BLOGSEARCH] =
      Util.checkElemExistAndReturn('blogSearchPingBody');
  this.site_.tabs_[TabPage.types.RUNTIME] =
      Util.checkElemExistAndReturn('runtimeInfoBody');
  this.commonTabParts_ = [];
  this.commonTabParts_[TabPage.types.GENERAL] =
      Util.checkElemExistAndReturn('commonGeneralSitemapBody');
  this.commonTabParts_[TabPage.types.WEB] =
      Util.checkElemExistAndReturn('commonWebSitemapBody');
  this.commonTabParts_[TabPage.types.NEWS] =
      Util.checkElemExistAndReturn('commonNewsSitemapBody');
  this.commonTabParts_[TabPage.types.VIDEO] =
      Util.checkElemExistAndReturn('commonVideoSitemapBody');
  this.commonTabParts_[TabPage.types.MOBILE] =
      Util.checkElemExistAndReturn('commonMobileSitemapBody');
  this.commonTabParts_[TabPage.types.CODESEARCH] =
      Util.checkElemExistAndReturn('commonCodeSearchSitemapBody');
  this.commonTabParts_[TabPage.types.BLOGSEARCH] =
      Util.checkElemExistAndReturn('commonBlogSearchPingBody');
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
  // clear the cookie for remembering the site and tab when user login.
  if (Cookie) {
    var cookie = new Cookie(TAB_COOKIE);
    cookie.remove();
  }

  this.loginUsername_.value = 'admin';
  // show login page
  Util.CSS.showElement(this.loginPage_);

  // hide main page
  Util.CSS.hideElement(this.cmdBtnArea_);
  Util.CSS.hideElement(this.menuBar_);
  Util.CSS.hideElement(this.settingPage_);

  // focus to password
  Util.checkElemExistAndReturn('loginPassword').focus();
};

/**
 * Shows setting page.
 * If user has login, it will be called after page load, otherwise, it will be
 * called after user pass login.
 */
PageManager.prototype.showSettingPage = function() {
  var perf = Perf.getPerf('Loading page');
  perf.check('preShow');
  // hide login page and show setting page
  Util.CSS.hideElement(this.loginPage_);
  Util.CSS.showElement(this.cmdBtnArea_);
  Util.CSS.showElement(this.menuBar_);
  Util.CSS.showElement(this.settingPage_);

  perf.check('showPage');
  // Notes: don't change the sequence
  // Gets XML from server
  Controller.updateSettingData();
  perf.check('updateSettingData');

  // update setting page with the data from server
  Controller.updateRuntimeInfo();
  perf.check('updateRuntimeInfo');

  // create setting page, must call after updateSettingData, need to know
  // how many sites to set.
  this.generateSettingPage();
  perf.check('generateSettingPage');

  // must call it after generateSettingPage
  PageManager.setupLanguage();
  perf.check('setupLanguage');

  // must call it after generateSettingPage, updateSettingData and
  // updateRuntimeInfo
  TabManager.getInstance().init();
  perf.check('initTabManager');
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
  var height = TabManager.getInstance().getCurrentTabHeight();
  if (this.historyHeight_ == null)
    this.historyHeight_ = height;

  var list = this.siteListContainer_;

  // Find the max value among site list height, tab page height and site list
  // container height. The last one is equal to the historical max height, which
  // can keep the height for the max height tab page, prevent changing
  // frequently when switch the tab page.
  var maxHeight = Math.max(list.offsetHeight, height);
  maxHeight = Math.max(this.historyHeight_, maxHeight);

  var listDiv = list.parentNode;
  var bodyDiv = this.settingPageBody_;
  listDiv.style.height = maxHeight;
  bodyDiv.style.height = maxHeight;
  this.settingPage_.style.height = maxHeight;
  this.historyHeight_ = maxHeight;
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
  Util.event.add(Util.checkElemExistAndReturn('refresh'), 'click', 
                Controller.onRefreshPage);
  Util.event.add(Util.checkElemExistAndReturn('restart'), 'click', 
                Controller.onSaveXmlAndRestart);
  Util.event.add(Util.checkElemExistAndReturn('logout'), 'click', 
                Controller.onLogout);

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

  // set tips and texts for selects
  var selects = document.getElementsByTagName('SELECT');
  Util.array.apply(selects, function(select) {
    TransMarkSet.transLanguageForSelectElem(select);
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
  // for Global setting
  this.generateSiteList(GLOBAL_SETTING_ID);

  // for site setting
  for (var i = 0; i < SiteSettings.getInstance().sitesNum(); i++) {
    this.generateSiteList(i.toString());
  }
};

/**
 * Switch to a new site.
 * @param {String} siteId  The id of the new site
 */
PageManager.prototype.switchSite = function(siteId) {
  this.curTabs_ = siteId == GLOBAL_SETTING_ID ?
                this.global_.tabs_ : this.site_.tabs_;

  Util.array.applyToMultiple(this.curTabs_, this.commonTabParts_,
                          function(tab, part) { tab.appendChild(part); });
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
  var siteName = siteId == GLOBAL_SETTING_ID ?
                 SiteSettings.getInstance().siteName(GLOBAL_SETTING_ID) :
                 SiteSettings.getInstance().siteName(siteId);
  /**
   * @type {String} siteLinkTip  The tip of the site link
   */
  var siteLinkTip = siteId == GLOBAL_SETTING_ID ?
                    SettingEditorLanguage.tips.globalLink :
                    SettingEditorLanguage.tips.siteLink;


  // generate one site list item from copying the template
  var siteListItem = this.siteListTpl_.cloneNode(true);
  Util.CSS.removeClass(siteListItem, HIDDEN_CLASS);

  // find the link
  var alink = siteListItem.rows.item(1).cells.item(1).firstChild;
  Util.event.add(alink, 'click', function() {
    var perf = new Perf('switch site');
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
    perf.check('done');
    perf.report();
  });

  alink.innerHTML = PageManager.addWrapMark(siteName);
  alink.tip = siteLinkTip;
  Tips.schedule(alink);

  if (siteId == GLOBAL_SETTING_ID) {
    Util.CSS.addClass(alink, SITELIST_GLOBALSETTING_CSS_CLASS);
  }

  this.siteListContainer_.appendChild(siteListItem);

  TabManager.getInstance().addSiteListItem(siteListItem, siteId);
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
  if (this.newPassword_.value == this.newPassword2_.value) {
    // request to server
    if (ServerManager.requestChangePassword(this.oldPassword_.value,
                                            this.newPassword_.value)) {
      // change succeed
      this.password_.value = this.newPassword_.value;
      this.close();
    } else {
      this.error_.innerHTML = ServerManager.errorMsg;
      Util.CSS.showElement(this.error_);
      this.clear();
      this.oldPassword_.focus();
    }
  } else {
    this.error_.innerHTML = NEW_PASSWORDS_CONFLICT;
    Util.CSS.showElement(this.error_);
    this.clear();
    this.oldPassword_.focus();
  }
};
