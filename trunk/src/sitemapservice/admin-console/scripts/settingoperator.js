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

/**
 * This class works as an interface for the data module.
 * @constructor
 */
function SettingOperator() {
  this.app_ = new AppGroup('SiteSettings');
  this.siteMng_ = new SiteManageGroup();
  /**
   * The sub node of global setting.
   * @type {SiteSetting}
   */
  this.global_ = new SiteGroup('GlobalSetting', this);

  /**
   * The sub node of site setting.
   * @type {SiteSetting}
   */
  this.site_ = new NormalSiteGroup('SiteSetting', this, this.global_);

  this.currentSite_ = GLOBAL_SETTING_ID;

  this.timestamp_ = null;
}

SettingOperator.services = {
  enums: {
    WEBSERVERFILTER: {xmltag: 'webServerFilter', htmlId: 'wf'},
    LOGPARSER: {xmltag: 'logParser', htmlId: 'lp'},
    FILESCANNER: {xmltag: 'fileScanner', htmlId: 'fs'},
    WEBSITEMAP: {xmltag: 'webSitemap', htmlId: 'web'},
    NEWSSITEMAP: {xmltag: 'newsSitemap', htmlId: 'news'},
    MOBILESITEMAP: {xmltag: 'mobileSitemap', htmlId: 'mobile'},
    CODESEARCHSITEMAP: {xmltag: 'codeSearchSitemap', htmlId: 'codesearch'},
    BLOGSEARCHPING: {xmltag: 'blogSearchPing', htmlId: 'blogsearch'}
  },
  getAllSitemaps: function() {
    var e = SettingOperator.services.enums;
    return [e.WEBSITEMAP, e.NEWSSITEMAP, e.MOBILESITEMAP, e.CODESEARCHSITEMAP,
            e.BLOGSEARCHPING];
  },
  /**
   * Gets HTML id's idPrefix of this service's HTML elements
   * @param {SettingOperator.services.enums} type  The service type
   * @return {String?} The html id
   * @private
   */
  getHtmlId: function(type) {
    return type.htmlId;
  }
};

/**
 * Singleton access function.
 */
function _getSetting() {
  if (_getSetting.instance_ == null) {
    _getSetting.instance_ = new SettingOperator();
  }
  return _getSetting.instance_;
}

/**
 * Gets all sitemaps html id prefixs.
 */
function _allSitemaps() {
  return _arr(SettingOperator.services.getAllSitemaps(), function(s) {
    return s.htmlId;
  });
}

SettingOperator.prototype.dirty = function() {
  return this.siteMng_.dirty() || this.app_.dirty() ||
         this.global_.dirty() || this.site_.dirty();
};

SettingOperator.prototype.clearDirty = function() {
  this.app_.clearDirty();
  this.siteMng_.clearDirty();
  this.global_.clearDirty();
  this.site_.clearDirty();
};

/**
 *
 * @param {Object} id  'site', 'web', 'news', 'mobile', 'codesearch',
 * 'blogsearch'
 */
SettingOperator.prototype.revertToDefault = function(id) {
  this.site_.revertToDefault(id);
};
/**
 *
 * @param {Object} id  'site', 'web', 'news', 'mobile', 'codesearch',
 * 'blogsearch'
 */
SettingOperator.prototype.isCustomized = function(id) {
  return this.site_.isCustomized(id);
};
/**
 *
 * @param {Object} id  'site', 'web', 'news', 'mobile', 'codesearch',
 * 'blogsearch'
 */
SettingOperator.prototype.isEnabled = function(id) {
  return this.cursite_().isEnabled(id);
};
/**
 *
 * @param {Object} id  'site', 'web', 'news', 'mobile', 'codesearch',
 * 'blogsearch'
 */
SettingOperator.prototype.setEnable = function(id, flag) {
  this.cursite_().setEnable(id, flag);
};
/**
 * Sets the timestamp of this sitesettings.
 * @param {String} ts  The timestamp
 */
SettingOperator.prototype.SetTimestamp = function(ts) {
  this.timestamp_ = ts;
};

/**
 * Gets the timestamp of this sitesettings.
 * @return {String} The timestamp
 */
SettingOperator.prototype.timestamp = function() {
  return this.timestamp_;
};

/**
 * Loads values of the whole setting tree from XML. Actually, it delay the
 * normal site setting's load until the user requires it (which means
 * this.currentSite_ is set).
 *
 * @param {Document?} dom  The XML Document object, if null, means it's
 * a switch-site load
 */
SettingOperator.prototype.setData = function(dom) {
  // only do it when init page or refresh page
  // The XML Document object
  this.xmldom_ = dom;

  // The XML root node which tag is 'SettingOperator'
  this.xml_ = dom.documentElement;
  // Load app
  this.app_.load(this.xml_);
  this.SetTimestamp(this.xml_.getAttribute('last_modified'));

  // Load global setting
  var globalXml = Util.checkUniqueAndReturn(this.xml_, 'GlobalSetting');
  this.global_.load(globalXml);

  // Get the sites xml nodes, but load on demand
  this.xmlSites_ = this.xml_.getElementsByTagName('SiteSetting');
  // Load site manage
  this.siteMng_.load(this.xmlSites_, globalXml);

  if (this.currentSite_ && this.currentSite_ != GLOBAL_SETTING_ID) {
    var idx = parseInt(this.currentSite_);
    // Load current site
    this.site_.load(this.xmlSites_[idx]);
  }
};

/**
 * Load another site. Must be called after setData().
 * @param {String} siteId  Which site should be loaded now. Include
 * GLOBAL_SETTING_ID
 */
SettingOperator.prototype.loadSite = function(siteId) {
  if (this.currentSite_ == siteId) {// already load
    return;
  }
  this.currentSite_ = siteId;

  if (siteId == GLOBAL_SETTING_ID) {
    this.global_.load();
  } else {
    var idx = parseInt(siteId);
    this.site_.load(this.xmlSites_[idx]);
    this.siteMng_.switchTo(idx);
  }
};

/**
 * reload the current page, may be used to revert the user change.
 */
SettingOperator.prototype.reloadCurPage = function() {
  switch (_getPager().curPage()) {
    case 'SiteManage':
      this.siteMng_.load();
      break;
    case 'Preference':
      this.app_.load();
      break;
    case 'site':
    case 'web':
    case 'news':
    case 'mobile':
    case 'codesearch':
    case 'blogsearch':
      this.cursite_().load();
      break;
  }
};

SettingOperator.prototype.getXmlString = function() {
  return XmlAttr.serialize(this.xml_);
};

/**
 * Gets current site setting.
 */
SettingOperator.prototype.cursite_ = function() {
  return this.currentSite_ == GLOBAL_SETTING_ID ? this.global_ : this.site_;
};

/**
 * Saves the whole setting tree to XML DOM.
 */
SettingOperator.prototype.save = function() {
  // just save the current site, the other site has been saved before
  switch (_getPager().curPage()) {
    case 'SiteManage':
      this.siteMng_.save();
      break;
    case 'Preference':
      this.app_.save();
      break;
    case 'site':
    case 'web':
    case 'news':
    case 'mobile':
    case 'codesearch':
    case 'blogsearch':
      this.cursite_().save();
      break;
  }
};

SettingOperator.prototype.validate = function() {
  // Generally speaking, it should check all tabs in all sites.
  // But since program will check the current tab when user switch to another,
  // so we can assume that all the other tabs in the site have been checked,
  // and all tabs in other sites have been checked, too.
  // (we assume the values from server's xml are correct)
  // currently, SiteManage and Preference page do not need check.
  return this.cursite_().validate();
};

/**
 * Gets the number of total sites.
 * @return {Number} The number of total sites
 */
SettingOperator.prototype.sitesNum = function() {
  return this.xmlSites_.length;
};


/**
 * Gets site name in XML.
 * @param {String|Number|null} opt_siteIdx  The site identifier, 'g' for global
 *   setting, 'Num' for the normal site. 'null' means current site.
 * @return {String?} The site name
 */
SettingOperator.prototype.siteName = function(opt_siteIdx) {
  var siteIdx = opt_siteIdx == null ? this.currentSite_ : opt_siteIdx;
  if (siteIdx == GLOBAL_SETTING_ID) {
    return GLOBAL_SETTING_NAME;
  } else {
    return this.siteMng_.siteName(parseInt(siteIdx));
  }
};

/**
 * Gets site id in XML.
 * @param {String|Number|null} opt_siteIdx  The site identifier, 'g' for global
 *   setting, 'Num' for the normal site. 'null' means current site.
 * @return {String?} The site id
 */
SettingOperator.prototype.siteId = function(opt_siteIdx) {
  var siteIdx = opt_siteIdx || this.currentSite_;
  if (siteIdx == GLOBAL_SETTING_ID) {
    return GLOBAL_SETTING_ID;
  } else {
    return this.siteMng_.siteId(parseInt(siteIdx))
  }
};

/**
 * Gets the site index.
 * @return {String|Number} The site index
 */
SettingOperator.prototype.getCurSiteIdx = function() {
  return this.currentSite_;
};

SettingOperator.prototype.curMemLimit = function(opt_siteIdx) {
  if (opt_siteIdx) {
    var num = this.xmlSites_[opt_siteIdx].getAttribute('max_url_in_memory');
    if (num) return num;
    return this.global_.getSetting_('max_url_in_memory').getValue();
  }
  return this.cursite_().getSetting_('max_url_in_memory').getValue();
};
SettingOperator.prototype.curDiskLimit = function(opt_siteIdx) {
  if (opt_siteIdx) {
    var num = this.xmlSites_[opt_siteIdx].getAttribute('max_url_in_disk');
    if (num) return num;
    return this.global_.getSetting_('max_url_in_disk').getValue();
  }
  return this.cursite_().getSetting_('max_url_in_disk').getValue();
};
