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
 * @fileoverview
 */

///////////////////////////////////////////////////////////////
/**
 * 
 * @constructor
 */
function Pager() {
  this.pages_ = new TabSet(['dashboard', 'loginPage', 'SiteManage',
                            'Preference', 'SiteDetail'], false);
  var me = this;
  this.siteTabs_ = new TabSet(
      ['Status', 'Settings', 'Sitemaps'], true,
      function(id) {
        if (me.isDirty_) {
          alert(CHANGE_UNSAVE_WARNING);
          return;
        }
        _getPager().showSiteTab_(id);
      });

  this.sitemapTabs_ = new TabSet(
      _allSitemaps(), true,
      function(id) {
        if (me.isDirty_) {
          alert(CHANGE_UNSAVE_WARNING);
          return;
        }
        _getPager().showSitemapTab_(id);
      });
  this.isDirty_ = false;
  this.prevPages_ = [];
}

/**
 * Singleton access function.
 */
function _getPager() {
  if (_getPager.instance_ == null) {
    _getPager.instance_ = new Pager();
  }
  return _getPager.instance_;
}

///////////////////////// value change functions ///////////////////////
/**
 * Check customize state, toggle the warning bar and the 'revert' button on 
 * current page or opt_page. If opt_customize is defined, use this value instead
 * of check result.
 * @param {Object} opt_page
 * @param {Object} opt_customize
 */
Pager.prototype.checkCustomize = function(opt_page, opt_customize) {
  var page = opt_page || this.curPage();
  var w = document.getElementById(page + '-custom-warning');
  if (w) {
    var customize = opt_customize == null ? (this.isDefault_ ? false :
                    _getSetting().isCustomized(page)) : opt_customize;
    if (customize) {
      _show(w);
      this.curRevertBtn_(page).removeAttribute('disabled');
    } else {
      _hide(w);
      this.curRevertBtn_(page).disabled = true;
    }
  }
};

/**
 * Handler for user changing. Called once user modify some setting field.
 */
Pager.prototype.onSettingChange = function() {
  this.checkCustomize(this.curPage());

  // everywhere that can change the setting value should has a save button
  this.curSaveBtn_().removeAttribute('disabled');
  this.curCancelBtn_().removeAttribute('disabled');
  this.isDirty_ = true;
};

//////////////// page related functions /////////////////////
/**
 * Gets current page id.
 */
Pager.prototype.curPage = function() {
  var id = this.pages_.curId();
  switch (id) {
    case 'SiteDetail':
      id = this.siteTabs_.curId();
      switch (id) {
        case 'Sitemaps':
          return this.sitemapTabs_.curId();
        case 'Settings':
          return 'site';
        default:
          return id;
      }
    default:
      return id;
  }
};

/**
 * From the inner links, not from navbar.
 * @param {Object} sitemapName
 * @param {Object} opt_siteIdx
 */
Pager.prototype.gotoSitemap = function(sitemapName, opt_siteIdx) {
  if (this.isDirty_) {
    alert(CHANGE_UNSAVE_WARNING);
    return;
  }
  var prev = this.curPage();
  if (opt_siteIdx)
    this.switchSite_(opt_siteIdx);
  this.showPage_('SiteDetail');
  this.showSiteTab_('Sitemaps');
  this.showSitemapTab_(sitemapName);
  this.storePrevPage_(prev, this.curPage());
};

/**
 * From the inner links, not from navbar.
 * @param {Object} index
 * @param {Object} tabName
 */
Pager.prototype.gotoSite = function(index, tabName) {
  if (this.isDirty_) {
    alert(CHANGE_UNSAVE_WARNING);
    return;
  }
  var prev = this.curPage();
  this.switchSite_(index);// should call before the following functions
  this.showPage_('SiteDetail');
  this.showSiteTab_(tabName);
  this.storePrevPage_(prev, this.curPage());
};

/**
 *
 * @param {Object} page
 */
Pager.prototype.gotoPage = function(page) {
  if (this.isDirty_) {
    alert(CHANGE_UNSAVE_WARNING);
    return;
  }
  var prev = this.curPage();
  this.showPage_(page);
  this.storePrevPage_(prev, this.curPage());
};

/**
 * Find the first page that user want to see. If cookie doesn't store the last
 * page user has accessed, show the default page.
 */
Pager.prototype.showFirstPage = function() {
  // show after load
  var s = _getSession();
  var c = Session.cookieItems;
  var page = s.getCookie(c.PAGE) || DEFAULT_PAGE;
  var site = s.getCookie(c.SITE) || DEFAULT_SITEID;
  var siteTab = s.getCookie(c.SITETAB) || DEFAULT_SITETAB;
  var sitemap = s.getCookie(c.SITEMAP) || DEFAULT_SITEMAPTAB;

  this.switchSite_(site);
  this.showPage_(page);
  if (this.isDefault_ && siteTab == 'Status') {
    // 'Default' has no status page.
    this.showSiteTab_('Settings');
  } else {
    this.showSiteTab_(siteTab);
  }
  this.showSitemapTab_(sitemap);
};

/**
 * Mainly for UI change, and some initialization for the shown page.
 * @param {Object} tid
 */
Pager.prototype.showPage_ = function(tid){
  var oid = this.pages_.show(tid);
  switch (tid) {
    case 'loginPage':
      _show(_gel('topline'));
      _hide(_gel('linkbar'));
      Util.event.clear(_gel('logo'), 'click');
      Util.CSS.removeClass(_gel('logo'), CLICKABLE_CSS);
      // clear password
      _gel('loginPassword').value = '';
      _getSession().setPasswd('');
      // focus to password
      _gel('loginPassword').focus();
      break;
    case 'SiteDetail':
        this.showSiteTab_();
      break;
  }
  switch (oid) {
    case 'loginPage':
      _hide(_gel('topline'));
      _show(_gel('linkbar'));
      _event(_gel('logo'), 'click', function(){
        _getPager().gotoPage('dashboard');
      });
      Util.CSS.addClass(_gel('logo'), CLICKABLE_CSS);
      break;
  }
  if (tid != 'loginPage') // exclude login page
    _getSession().saveCookie(Session.cookieItems.PAGE, tid);
};

/**
 * Show site detail
 * @param {Object} tid
 */
Pager.prototype.showSiteTab_ = function(opt_tid){
  var tid = opt_tid || this.siteTabs_.curId();
  if (!tid) return;

  var oid = this.siteTabs_.show(tid);
  switch (tid) {
    case 'Sitemaps':
      // special code for Default setting page
      if (this.isDefault_) {
        Pager.changeTitle('Default Sitemaps');
      }
      this.showSitemapTab_();

      _show(_gel('sitemaps-sub-bar'));
      break;
    case 'Settings':
      // special code for Default setting page
      if (this.isDefault_) {
        Pager.changeTitle('Default Settings');
      }

      // show sitemap table
      this.initSitemapTable_();

      // show custom warning info
      this.checkCustomize('site');
      break;
    case 'Status':
      break;
  }
  switch (oid) {
    case 'Sitemaps':
      _hide(_gel('sitemaps-sub-bar'));
      break;
  }
  _getSession().saveCookie(Session.cookieItems.SITETAB, tid);
};


/**
 * Show sitemap page.
 * @param {Object} opt_tid
 */
Pager.prototype.showSitemapTab_ = function(opt_tid){
  var tid = opt_tid || this.sitemapTabs_.curId();
  if (!tid) return;

  this.sitemapTabs_.show(tid);

  // show custom warning info
  this.checkCustomize(tid);
  _getSession().saveCookie(Session.cookieItems.SITEMAP, tid);
};

/**
 * Initial sitemap table on site configuration page.
 */
Pager.prototype.initSitemapTable_ = function() {
  var table = _gel('regSitemaps');
  this.isDefault_ ? Util.CSS.addClass(table, 'default') :
                    Util.CSS.removeClass(table, 'default');
  if (!this.isDefault_) {
    // initalize sitemap status
    // It should be arranged at UI layer, not data layer, that's why it is here.
    function showSitemapInfo(id) {
      var elem = _gel(id + '-customize');
      if (_getSetting().isCustomized(id)) {
        elem.innerHTML = SITEMAP_CUSTOM;
        Util.CSS.removeClass(elem, 'status-default');
      } else {
        elem.innerHTML = SITEMAP_DEFAULT;
        Util.CSS.addClass(elem, 'status-default');
      }
    }
    _arr(_allSitemaps(), showSitemapInfo);
  }
};

Pager.prototype.storePrevPage_ = function(prev, next) {
  if (this.curCancelBtn_(next)) {
    this.prevPages_.push(prev);
    this.curCancelBtn_(next).removeAttribute('disabled');
  } else {
    this.prevPages_ = [];
  }
};

/**
 *
 * @param {Object} index
 */
Pager.prototype.switchSite_ = function(index) {
  var old = this.isDefault_;
  this.isDefault_ = index == GLOBAL_SETTING_ID;

  // switch site name
  var setting = _getSetting();
  var name = setting.siteName(index);
  Pager.changeTitle(index == GLOBAL_SETTING_ID ? 'Default Settings' : name);
  // load site data
  setting.loadSite(index + ''/* should be string */);
  // load runtime data
  _getRuntime().loadSite(index + ''/* should be string */);

  // change layout
  if (old != this.isDefault_) {
    if (this.isDefault_) {
      _hide(_gel('Status-s'));
      _hide(_gel('site-sc'));
      _hide(_gel('revert-site'));
      _hide(_gel('SitesS'));
      _arr(_allSitemaps(), function(id) {
        _hide(_gel('revert-' + id));
      });
    } else {
      _show(_gel('Status-s'));
      _show(_gel('site-sc'));
      _show(_gel('revert-site'))
      _show(_gel('SitesS'));
      _arr(_allSitemaps(), function(id) {
        _show(_gel('revert-' + id));
      });
    }
  }
  _getSession().saveCookie(Session.cookieItems.SITE, index);
};

///////////////////// action buttons related functions /////////
/**
 * Gets 'save' button on current page or opt_page.
 * @param {Object} opt_page
 */
Pager.prototype.curSaveBtn_ = function(opt_page) {
  var page = opt_page || this.curPage();
  switch (page) {
    case 'SiteManage':
      return _gel('save-1');
    case 'Preference':
      return _gel('save-2');
    case 'site':
      return _gel('save-3');
    case 'web':
      return _gel('save-4');
    case 'news':
      return _gel('save-5');
    case 'mobile':
      return _gel('save-6');
    case 'codesearch':
      return _gel('save-7');
    case 'blogsearch':
      return _gel('save-8');
  }
  return null;
};

/**
 * Gets 'cancel' button on current page or opt_page.
 * @param {Object} opt_page
 */
Pager.prototype.curCancelBtn_ = function(opt_page) {
  var page = opt_page || this.curPage();
  switch (page) {
    case 'SiteManage':
      return _gel('cnl-1');
    case 'Preference':
      return _gel('cnl-2');
    case 'site':
      return _gel('cnl-3');
    case 'web':
      return _gel('cnl-4');
    case 'news':
      return _gel('cnl-5');
    case 'mobile':
      return _gel('cnl-6');
    case 'codesearch':
      return _gel('cnl-7');
    case 'blogsearch':
      return _gel('cnl-8');
  }
  return null;
};

/**
 * Gets 'Revert to default' button on current page or opt_page.
 * @param {Object} opt_page
 */
Pager.prototype.curRevertBtn_ = function(opt_page) {
  var page = opt_page || this.curPage();
  switch (page) {
    case 'site':
      return _gel('revert-site');
    case 'web':
    case 'news':
    case 'mobile':
    case 'codesearch':
    case 'blogsearch':
      return _gel('revert-' + page);
  }
  return null;
};

/**
 * After 'save' action, clear the state of buttons on current page.
 */
Pager.prototype.refreshActionButtons = function() {
  var btn = this.curSaveBtn_();
  if (btn) btn.disabled = true;
  if (!this.prevPages_.length) {
    btn = this.curCancelBtn_();
    if (btn) btn.disabled = true;
  }
};

/**
 * 'save' button click handler.
 */
Pager.prototype.save = function() {
  if (!Flow.save()) return;
  
  this.curSaveBtn_().disabled = true;
  if (!this.prevPages_.length) {
    this.curCancelBtn_().disabled = true;
  }
  this.isDirty_ = false;
};

/**
 * 'cancel' button click handler.
 */
Pager.prototype.cancel = function() {
  _getSetting().reloadCurPage();
  this.curSaveBtn_().disabled = true;
  this.curCancelBtn_().disabled = true;
  ValidateManager.clear();
  this.isDirty_ = false;
  this.checkCustomize();

  if (this.prevPages_.length) {
    var page = this.prevPages_.pop();
    switch (page) {
      case 'dashboard':
      case 'SiteManage':
      case 'Preference':
      case 'loginPage':
        this.showPage_(page);
        break;
      case 'site':
        this.showPage_('SiteDetail');
        this.showSiteTab_('Settings');
        break;
      case 'Status':
        this.showPage_('SiteDetail');
        this.showSiteTab_('Status');
        break;
      case 'web':
      case 'news':
      case 'mobile':
      case 'codesearch':
      case 'blogsearch':
        this.showPage_('SiteDetail');
        this.showSiteTab_('Sitemaps');
        this.showSitemapTab_(page);
        break;
      default:
        _err('invalid page type!');
    }
  }
  if (!this.prevPages_.length) {
    var btn = this.curCancelBtn_();
    if (btn) btn.disabled = true;
  }
};

/**
 * 'revert' button click handler.
 * If user change the page from default to customize, then click revert button
 * will 'cancel' user's change, but don't jump back. If user directly click
 * revert button for customized page, then it will trigger 'sync to default'
 * and 'save'.
 * @param {Object} id
 */
Pager.prototype.revert = function(id) {
  var old_status = _getSetting().isCustomized(id);
  _getSetting().revertToDefault(id);
  if (old_status) { // This page has been customized before.
    Flow.save();
  }
  this.curSaveBtn_().disabled = true;
  if (!this.prevPages_.length) {
    this.curCancelBtn_().disabled = true;
  }
  this.isDirty_ = false;
  this.checkCustomize(null, false);
};

Pager.changeTitle = function(str) {
  _gel('sitetitle').innerHTML = str;
};

///////////////////////// PromptPage //////////////////////////////
/**
 * 
 * @constructor
 */
function PromptPage() {
  this.divMaskFF_ = _gel('FFMask');
  this.divMaskIE_ = _gel('IEMask');
  this.divPrompt_ = _gel('prompt');
  this.input_ = _gel('promptInput');
  Component.regist(this);
}
PromptPage.prototype.release = function() {
  this.divMaskFF_ = null;
  this.divMaskIE_ = null;
  this.divPrompt_ = null;
  this.input_ = null;
};
PromptPage.prototype.show = function(callback) {
  var input = this.input_;
  input.value = '';

  var me = this;
  Util.event.clearEnterKey(input);
  Util.event.addEnterKey(input, function() {
    me.hide();
    callback(input.value);
  });

  switch (Browser.BROWSER_TYPE) {
    case Browser.types.SAFARI:
    case Browser.types.FIREFOX:
      _show(this.divMaskFF_);
      break;
    case Browser.types.IE:
      _show(this.divMaskIE_);
      break;
  }

  _show(this.divPrompt_);

  input.focus();
};

PromptPage.prototype.hide = function() {
  _hide(this.divPrompt_);
  switch (Browser.BROWSER_TYPE) {
    case Browser.types.SAFARI:
    case Browser.types.FIREFOX:
      _hide(this.divMaskFF_);
      break;
    case Browser.types.IE:
      _hide(this.divMaskIE_);
      break;
  }
};
///////////////////////// LoadingPage /////////////////////////////
/**
 * 
 * @constructor
 */
function LoadingPage() {
  this.count_ = 0;
  this.progressBarIn_ = _gel('progressBarIn');
  this.page_ = _gel('loading');
  Component.regist(this);
}

LoadingPage.prototype.release = function() {
  this.progressBarIn_ = null;
  this.page_ = null;
};

LoadingPage.prototype.show = function() {
  this.step(1);
  _show(this.page_);
};

LoadingPage.prototype.step = function(opt_count){
  if (opt_count) {
    this.count_ = Math.min(opt_count, 10);
  } else {
    if (this.count_ < 10)
      this.count_++;
  }
  this.progressBarIn_.style.width = this.count_ * 10 + '%';
};

LoadingPage.prototype.hide = function() {
  this.step(10);
  _hide(this.page_);
};

///////////////////////////////////////////////////////////////
function Loader() {
  var loading = new LoadingPage();
  loading.show();

  Loader.initLanguage();
  Loader.initLinksAndButtons();
  _getPasswdManager();
  loading.step();

  Flow.onLoad();

  loading.hide();
}
_event(window, 'load', Loader);
_event(window, 'unload', function() {
  Util.event.flush();
  Component.releaseAll();
});

Loader.initLanguage = function() {
  document.title = WINDOW_TITLE;

  // set text to links, headers, spans and paragraphs
  _arr(_gelt('A'), function(a){
    TransMarkSet.localizeInnerHTML(a);
    TransMarkSet.localizeTip(a);
  });
  _arr(_gelt('H2'), function(h){
    TransMarkSet.localizeInnerHTML(h);
    TransMarkSet.localizeTip(h);
  });
  _arr(_gelt('SPAN'), function(span){
    TransMarkSet.localizeInnerHTML(span);
    TransMarkSet.localizeTip(span);
  });
  _arr(_gelt('P'), function(p){
    TransMarkSet.localizeInnerHTML(p);
    TransMarkSet.localizeTip(p);
  });

  // set values to inputs
  _arr(_gelt('INPUT'), function(input){
    TransMarkSet.localizeValue(input);
    TransMarkSet.localizeTip(input);
  });

  // set tips to img
  _arr(_gelt('img'), function(img){
    TransMarkSet.localizeTip(img, true);
  });
};

Loader.initLinksAndButtons = function() {
  _event(_gel('loginBtn'), 'click', Flow.login);
  Util.event.addEnterKey(_gel('loginPassword'), Flow.login);
  _event(_gel('logoutLink'), 'click', Flow.logout);

  function initPageLink(id, page) {
    _event(_gel(id), 'click', function(){
      _getPager().gotoPage(page);
    });
  }

  Util.CSS.addClass(_gel('logo'), CLICKABLE_CSS);
  initPageLink('logo', 'dashboard');
  initPageLink('dl-1', 'dashboard');
  initPageLink('dl-2', 'dashboard');
  initPageLink('dl-3', 'dashboard');
  initPageLink('prefLink', 'Preference');
  initPageLink('manageLink', 'SiteManage');

  // add href to links
  _arr(_gelt('a'), function(a){
    if (!a.href)
      a.href = '#';
  });

  // add CSS 'button' to buttons
  _arr(_gelt('input'), function(b){
    if (b.type == 'button')
      Util.CSS.addClass(b, 'button');
  });

  // links on dashboard
  new ExpandHtml(_gel('appRuntimeMore'), [_gel('moreL'), _gel('moreR')]);
  var label = {open: 'Detail', close: 'Hide'};
  new ExpandHtml(_gel('rt-site-s'), [_gel('rt-site')], label);
  var wf = new ExpandHtml(_gel('rt-wf-s'), [_gel('rt-wf')], label);
  var lp = new ExpandHtml(_gel('rt-lp-s'), [_gel('rt-lp')], label);
  var fs = new ExpandHtml(_gel('rt-fs-s'), [_gel('rt-fs')], label);
  var web = new ExpandHtml(_gel('rt-web-s'), [_gel('rt-web')], label);
  var news = new ExpandHtml(_gel('rt-news-s'), [_gel('rt-news')], label);
  var mobile = new ExpandHtml(_gel('rt-mobile-s'), [_gel('rt-mobile')], label);
  var csearch =
      new ExpandHtml(_gel('rt-codesearch-s'), [_gel('rt-codesearch')], label);
  var bsearch =
      new ExpandHtml(_gel('rt-blogsearch-s'), [_gel('rt-blogsearch')], label);

  label = {open: 'Show details', close: 'Hide all'};
  new ExpandHtml(_gel('rt-crawlers-s'), null, label, {open: function() {
    wf.open();
    lp.open();
    fs.open();
  }, close: function() {
    wf.close();
    lp.close();
    fs.close();
  }});
  new ExpandHtml(_gel('rt-sitemaps-s'), null, label, {open: function() {
    web.open();
    news.open();
    mobile.open();
    csearch.open();
    bsearch.open();
  }, close: function() {
    web.close();
    news.close();
    mobile.close();
    csearch.close();
    bsearch.close();
  }});

  _event(_gel('defSettingsLink'), 'click', function(){
    _getPager().gotoSite(GLOBAL_SETTING_ID, 'Settings');
  });
  _event(_gel('defSitemapsLink'), 'click', function(){
    _getPager().gotoSite(GLOBAL_SETTING_ID, 'Sitemaps');
  });

  // save and cancel and revert
  for (var i = 1; i <= 8; i++) {
    _event(_gel('save-' + i), 'click', function(){
      _getPager().save();
    });
    _gel('save-' + i).disabled = true;

    _event(_gel('cnl-' + i), 'click', function(){
      _getPager().cancel();
    });
    _gel('cnl-' + i).disabled = true;
  }

  function set(id) {
    _event(_gel('revert-' + id), 'click', function(){
      // if user want sync to global, confirm it.
      if (confirm(CANCEL_CUSTOMIZE_SETTING_MSG)) {
        _getPager().revert(id);
      }
    });
  }
  set('site');
  _arr(_allSitemaps(), set);

  // init links to sitemap tabs in sitemaps table on site setting page
  _arr(_gel('regSitemaps').getElementsByTagName('a'), function(link){
    _event(link, 'click', function(e, t){
      _getPager().gotoSitemap(t.getAttribute('sitemap'));
    });
  });

  // init links on site status page
  _arr(_gel('Status').getElementsByTagName('a'), function(link){
    _event(link, 'click', function(e, t){
      var s = t.getAttribute('sitemap');
      if (s)
        _getPager().gotoSitemap(s);
    });
  });
};

///////////////////////////////////////////////////////////////

/**
 *
 * @param {Element} click  Clickable html, used to control the expanding.
 * @param {Object} areas  The list of areas that are to be expanded.
 * @constructor
 */
function ExpandHtml(click, areas, opt_clickLabels, opt_actions) {
  this.expanded_ = false;
  this.openLabel_ = 'more &raquo;';
  this.closeLabel_ = '&laquo; fewer';

  this.click_ = click;
  this.extElems_ = areas;
  if (opt_clickLabels) {
    if (opt_clickLabels.open){
      this.openLabel_ = opt_clickLabels.open;
    }
    if (opt_clickLabels.close) {
      this.closeLabel_ = opt_clickLabels.close;
    }
  }
  if (opt_actions) {
    if (opt_clickLabels.open){
      this.open_ = opt_actions.open;
    }
    if (opt_clickLabels.close) {
      this.close_ = opt_actions.close;
    }
  }

  this.close();

  var me = this;
  _event(this.click_, 'click', function() {
    me.switch_();
  });
  Component.regist(this);
}

ExpandHtml.prototype.open = function() {
  this.click_.innerHTML = this.closeLabel_;
  this.open_();
  this.expanded_ = true;
};
ExpandHtml.prototype.open_ = function() {
  _arr(this.extElems_, function(el) {
    _show(el);
  });
};
ExpandHtml.prototype.close = function() {
  this.click_.innerHTML = this.openLabel_;
  this.close_();
  this.expanded_ = false;
};
ExpandHtml.prototype.close_ = function() {
  _arr(this.extElems_, function(el) {
    _hide(el);
  });
};

ExpandHtml.prototype.switch_ = function() {
  if (this.expanded_) {
    this.close();
  } else {
    this.open();
  }
};
ExpandHtml.prototype.release = function() {
  this.click_ = null;
  this.extElems_ = null;
};

 ///////////////////////////////PasswordManager//////////////////////////
/**
 * The password-change UI.
 * @constructor
 */
function PasswordManager() {
  /**
   * UI root.
   * @type {ELement}
   */
  this.page_ = _gel('passwd-container');

  /**
   * The 'change' link behind the password readonly setting field
   * @type {ELement}
   */
  this.change_ = _gel('passwd-change');

   /**
   * The old password input field
   * @type {ELement}
   */
  this.oldPassword_ = _gel('passwd-old');

  /**
   * The new password input field
   * @type {ELement}
   */
  this.newPassword_ = _gel('passwd-new');

  /**
   * The new password input again field
   * @type {ELement}
   */
  this.newPassword2_ = _gel('passwd-new2');

  /**
   * The error message box.
   * @type {ELement}
   */
  this.error_ = _gel('passwd-err');

  // add handler
  var me = this;
  _event(this.change_, 'click', function() {
    me.open();
  });
  _event(_gel('passwd-save'), 'click', function() {
    me.confirm();
  });
  _event(_gel('passwd-cancel'), 'click', function() {
    me.close();
  });
  Util.event.addEnterKey(this.oldPassword_, function() {
    me.newPassword_.focus();
  })
  Util.event.addEnterKey(this.newPassword_, function() {
    me.newPassword2_.focus();
  })
  Util.event.addEnterKey(this.newPassword2_, function() {
    me.confirm();
  })
}

/**
 * Singleton access function.
 */
function _getPasswdManager() {
  if (_getPasswdManager.instance_ == null) {
    _getPasswdManager.instance_ = new PasswordManager();
  }
  return _getPasswdManager.instance_;
}

/**
 * Opens the change-password box.
 */
PasswordManager.prototype.open = function(){
  this.clearErr();
  _show(this.page_);
  this.clear();
  this.change_.disabled = true;
};

/**
 * Close the change-password box.
 */
PasswordManager.prototype.close = function(){
  _hide(this.page_);
  this.clearErr();
  this.change_.disabled = false;
};

/**
 * Clear the change-password box's input fields.
 */
PasswordManager.prototype.clear = function(){
  this.oldPassword_.value = '';
  this.newPassword_.value = '';
  this.newPassword2_.value = '';
  this.oldPassword_.focus();
};

PasswordManager.prototype.error = function(msg) {
  this.error_.innerHTML = msg;
  Util.CSS.addClass(this.error_, ERR_CSS);
};
PasswordManager.prototype.clearErr = function(msg) {
  this.error_.innerHTML = '<br>';
  Util.CSS.removeClass(this.error_, ERR_CSS);
};

/**
 * Checks the new passwords' consistency, submits the password change.
 */
PasswordManager.prototype.confirm = function() {
  function invalid(elem) {
    ValidateManager.setElementValidation_(elem, false);
    elem.value = '';
  }
  function valid(elem) {
    ValidateManager.setElementValidation_(elem, true);
  }
  if (!ValidateManager.validators.PASSWORD.check(this.oldPassword_.value)) {
    invalid(this.oldPassword_);
    this.error(OLD_PASSWORD_INVALID);
    this.oldPassword_.focus();
    return;
  }
  valid(this.oldPassword_);
  if (this.newPassword_.value != this.newPassword2_.value) {
    invalid(this.newPassword_);
    invalid(this.newPassword2_);
    this.error(NEW_PASSWORDS_CONFLICT);
    this.newPassword_.focus();
    return;
  }
  if (!ValidateManager.validators.PASSWORD.check(this.newPassword_.value)) {
    invalid(this.newPassword_);
    invalid(this.newPassword2_);
    this.error(NEW_PASSWORD_INVALID);
    this.newPassword_.focus();
    return;
  }
  valid(this.newPassword_);
  valid(this.newPassword2_);

  // request to server
  if (!ServerManager.requestChangePassword(this.oldPassword_.value,
                                           this.newPassword_.value)) {
    this.error(ServerManager.errorMsg);
    this.clear();
    return;
  }

  this.close();
};
