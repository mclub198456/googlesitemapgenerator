// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview
 *
 * @author chaiying@google.com
 */


/////////////////////////////////////////////////////////////////////
//////////// Class SettingGroup /////////////////
/**
 * The base setting group class, represents as a middle node of the setting
 * Tree.
 * @param {String} xmltag  The xmltag of the setting group
 * @constructor
 */
function SettingGroup(xmltag, ownerGroup) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }
  /**
   * The setting components as leaves of this node.
   * @type {Array.<SettingSetting>}
   */
  this.settings_ = [];

  /**
   * The XML node corresponding to this setting node.
   * It will be set in the 'load' function.
   * @type {XMLNode}
   */
  this.xml_ = null;

  /**
   * The xmltag of the setting XML node.
   * Usually it's the tag name of the XML node of this setting.
   * @type {String}
   */
  this.xmltag_ = xmltag;

  this.ownerGroup_ = ownerGroup;
}

/**
 * Check if the setting group has any setting that is dirty.
 */
SettingGroup.prototype.dirty = function() {
  return Util.array.checkIfAny(this.settings_, function(s) {
    return s.dirty();
  });
};

/**
 * Clear the dirty mark
 */
SettingGroup.prototype.clearDirty = function() {
  _arr(this.settings_, function(s) {
    s.clearDirty();
  });
};

/**
 * Gets the xmltag of the setting.
 * @return {String} The xmltag of the setting group
 */
SettingGroup.prototype.xmltag = function() {
  return this.xmltag_;
};

/**
 * Checks if the setting group is customized, if any setting component in this
 * setting group is not inherited, it will be treated as customized.
 * @return {Boolean} If the setting is customeized
 */
SettingGroup.prototype.isCustomized = function() {
  return Util.array.checkIfAny(this.settings_, function(s) {
    return s.isCustomized();
  });
};

SettingGroup.prototype.revertToDefault = function() {
  _arr(this.settings_, function(s) {
    s.revertToDefault();
  });
};


/**
 * Loads setting values from XML node.
 *
 * This base class method only load the values of the leaf setting of the
 * setting group node.
 *
 * Each sub class method should call this method at first to load the leaves,
 * and then deal with the child setting group nodes' loading.
 *
 * @param {XMLNode} xml  The XML node of this setting
 */
SettingGroup.prototype.load = function(xml) {
  if (xml)
    this.xml_ = xml;

  var me = this;
  _arr(this.settings_, function(s) {
    s.load(me.xml_);
  });
};

/**
 * Validates the components in the setting group.
 * @return {Boolean} If the setting components are all valid
 */
SettingGroup.prototype.validate = function() {
  // return false if any setting is not valide
  return !Util.array.checkIfAny(this.settings_, function(s) {
    return !s.validate();
  });
};

/**
 * Focus on the first setting element in the setting group.
 */
SettingGroup.prototype.focusOnFirst = function() {
  this.settings_[0].focus();
};

/**
 * Saves the setting value to XML node.
 */
SettingGroup.prototype.save = function() {
  _arr(this.settings_, function(s) {
    s.save();
  });
};

/**
 * Sets inherit property of the setting group.
 * @param {Boolean} isInherited  The value of the inherit property
 */
SettingGroup.prototype.setInherit = function(isInherited) {
  _arr(this.settings_, function(s) {
    s.setInherit(isInherited);
  });
};

/**
 * Sets the global setting group that can be inherited by the setting node.
 * @param {SettingGroup?} global  The same level node in global setting
 */
SettingGroup.prototype.setGlobal = function(global) {
  _arr(this.settings_, function(s) {
    s.setGlobal(global.getSetting_(s.name()));
  });
};

/**
 * Sets the access right of the components of this setting, since there are more
 * than one reason to set the access right, we must record the reason.
 * @param {Boolean} readonly  If the setting is readonly
 * @param {String} reason  The reason to set the access right
 */
SettingGroup.prototype.setAccess = function(readonly, reason) {
  _arr(this.settings_, function(s) {
    s.setAccess(readonly, reason);
  });
};

/**
 * Gets the setting component value that is in this setting group.
 * @param {String} name  The name of the setting
 * @return {Boolean|String|Array.<Array.<String>>|null} The value of the setting
 *   component
 */
SettingGroup.prototype.getValue = function(name) {
  var s = this.getSetting_(name);
  return s ? s.getValue() : null;
};

/**
 * Sets value of the setting component in the setting group.
 * @param {String} name  The name of the setting
 * @param {Boolean|String|Array.<Array.<String>>} value  The value of the
 *   setting component
 */
SettingGroup.prototype.setValue = function(name, value) {
  var s = this.getSetting_(name);
  if (s != null)
    s.setValue(value);
};

/**
 * Gets the setting component in the setting group.
 * @param {String} name  The name of the setting
 * @return {SettingSetting?} The component object
 * @private
 */
SettingGroup.prototype.getSetting_ = function(name) {
  return Util.array.find(this.settings_, function(s) {
    return s.name() == name;
  });
};

/**
 * Adds setting component to the node.
 * @param {SettingSetting} component  The component object
 * @private
 */
SettingGroup.prototype.addSetting_ = function(s) {
  this.settings_.push(s);
};

/**
 * Removes leaf setting components from the node.
 * @private
 */
SettingGroup.prototype.clearSettings_ = function() {
  this.settings_ = [];
};

/**
 * Tells the name of the node and the string value of the attached leaf
 * settings.
 * @return {String} The string representation of the node
 */
SettingGroup.prototype.toString = function() {
  var strVals = [];
  strVals.push(this.xmltag_);
  _arr(this.settings_, function(s) {
    strVals.push(s.toString());
  });
  strVals.push('\n');
  return strVals.join('\n');
};

/////////////////////////////////////////////////////////////////////
/**
 * This class represents the whole setting Tree, it is at the top level.
 * Inherits from BaseSetting, no sub class.
 * @constructor
 */
function AppGroup(xmltag) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }

  // set the name of the node
  this.parent.constructor.call(this, xmltag, null);

  // add the leaf settings of the node
  var s;

  s = new Setting('addGeneratorInfo', 'add_generator_info', Setting.types.BOOL);
  this.addSetting_(s);

  this.addSetting_(
      new Setting('remoteAccess', 'remote_admin', Setting.types.BOOL));
}
AppGroup.inheritsFrom(SettingGroup);

/////////////////////////////////////////////////////////////////////
/**
 * 
 * @constructor
 */
function SiteSwitcher() {
  // init site switcher
  this.elem_ = _gel('site-s');

  // user switch the site.
  _event(this.elem_, 'change', function(e, t){
    _getPager().switchSite(t.selectedIndex);
    _getPager().checkCustomize();
  });
  Component.regist(this);
}

SiteSwitcher.prototype.release = function() {
  this.elem_ = null;
};

SiteSwitcher.prototype.load = function(names) {
  var options = this.elem_.options;
  options.length = 0;
  var len = names.length;
  for (var i = 0; i < len; i++) {
    options[i] = new Option(names[i], i);
  }
};
SiteSwitcher.prototype.show = function(i) {
  _show(this.elem_.options[i]);
};
SiteSwitcher.prototype.hide = function(i) {
  _hide(this.elem_.options[i]);
};

/**
 * switch from js, not user input.
 * @param {Object} index
 */
SiteSwitcher.prototype.switchTo = function(index){
  this.elem_.selectedIndex = index;
};

/////////////////////////////////////////////////////////////////////
/**
 * @constructor
 */
function SiteManageGroup() {
  // set the name of the node
  SiteManageGroup.prototype.parent.constructor.call(this, null, null);
  this.xmlSites_ = null;
  this.numSites_ = 0;
  this.siteSwitcher_ = new SiteSwitcher();
  this.global_ = new Setting(null, 'enabled', Setting.types.BOOL);

  var me = this;
  // add handler for check all
  _event(_gel('sites-all'), 'click', function() {
    var len = me.numSites_;
    for (var i = 0; i < len; i++) {
      me.enableSite(i);
    }
  });
  // add handler for uncheck all
  _event(_gel('sites-none'), 'click', function() {
    var len = _getSetting().sitesNum();
    for (var i = 0; i < len; i++) {
      me.disableSite(i);
    }
  });
}
SiteManageGroup.inheritsFrom(SettingGroup);

SiteManageGroup.prototype.switchTo = function(index) {
  this.siteSwitcher_.switchTo(index);
};

SiteManageGroup.prototype.enableSite = function(i) {
  if (this.numEnabled_ == 0) {
    _hide(_gel('nosite'));
  }

  var setting = this.settings_[i];
  setting.setValue(true);
  setting.setInherit(true);

  _gel('siteEnabled-' + i).checked = true;
  Util.CSS.changeClass(
      _gel('siteName-' + i), 'sitename-disable', 'sitename-enable');
  _show(_gel('line-'+i));
  this.siteSwitcher_.show(i);

  this.numEnabled_++;
  this.numDisabled_--;
  _getPager().onSettingChange();
};

SiteManageGroup.prototype.disableSite = function(i) {
  var setting = this.settings_[i];
  setting.setValue(false);
  setting.setInherit(false);

  _gel('siteEnabled-' + i).checked = false;
  Util.CSS.changeClass(
      _gel('siteName-' + i), 'sitename-enable', 'sitename-disable');
  _hide(_gel('line-'+i));
  this.siteSwitcher_.hide(i);

  this.numDisabled_++;
  this.numEnabled_--;

  if (this.numEnabled_ == 0) {
    _show(_gel('nosite'));
  }
  _getPager().onSettingChange();
};

SiteManageGroup.prototype.load = function(opt_xmls, opt_globalXml) {
  if (!opt_xmls) { // reload the settings.
    var len = this.numSites_;
    for (var i = 0; i < len; i++) {
      var setting = this.getSetting_('site-enable-' + i);
      setting.load();
      switch (setting.getValue()) {
        case true:
        case 'true':
          this.enableSite(i);
          break;
        case false:
        case 'false':
          this.disableSite(i);
          break;
      }
    }
    return;
  }
  this.xmlSites_ = opt_xmls;
  this.numSites_ = opt_xmls.length;
  this.numEnabled_ = 0;
  this.numDisabled_ = 0;
  var len = this.numSites_;

  // load global
  this.global_.load(opt_globalXml);
  // set 'siteEnable' settings
  this.clearSettings_();
  for (var i = 0; i < len; i++) {
    // generate settings
    var s = new Setting(''/* FakeHtmlInput */, 'enabled', Setting.types.BOOL);
    s.setName('site-enable-' + i);
    s.setGlobal(this.global_);
    // load xml
    s.load(opt_xmls[i]);
    this.addSetting_(s);
  }

  // load to all-sites table and dashboard table
  var table = document.getElementById('sites');
  // remove the table node, add all the sites, and then insert the table node
  // again. This is for performance.
  var parent = table.parentNode;
  parent.removeChild(table);
  // remove the old content; add table header

  var tHTML = ['<table id=sites>',
               '<col class=c1 /><col class=c2 />',
               '<tr><th class=input-col></th>',
               '<th class=sitename>Sites name</sth></tr>'];
  var names = [];
  var me = this;
  for (var i = 0; i < len; i++) {
    var name = opt_xmls[i].getAttribute('name');
    names.push(name);
    var status;
    var classname;
    switch (this.getSetting_('site-enable-' + i).getValue()) {
      case true:
      case 'true':
        status = ' checked';
        classname = 'sitename-enable';
        this.numEnabled_++;
        break;
      case false:
      case 'false':
        status = '';
        classname = 'sitename-disable';
        this.numDisabled_++;
        break;
      default:
        _err('Invalid status.');
    }

    tHTML.push('<tr>');
    tHTML.push('<td class=input-col>'
                 + '<input type=checkbox id=siteEnabled-' + i + status + '>'
                 + '</td><td class=' + classname + '><label for=siteEnabled-'
                 + i + ' id=siteName-' + i + '>' + name + '</label></td>');
    tHTML.push('</tr>');
  }
  tHTML.push('</table>');
  parent.innerHTML = tHTML.join('');

  for (var i = 0; i < len; i++) {
    _event(_gel('siteEnabled-' + i), 'click', function(e, t) {
      var len = t.id.length;
      var pos = t.id.search('-');
      if (pos == -1) _err('invalid siteEnable id:' + t.id);
      pos += 1; // skip the '-'
      var idx = t.id.substr(pos, len - pos);
      t.checked ? me.enableSite(idx) : me.disableSite(idx);
    });
  }

  // For dashboard table
  var nameHtml = ['<div class=header>',
                  SettingEditorLanguage.texts.enabledSitesTitle,
                  '</div>',
                   '<div id=nosite class=hidden>',
                  SettingEditorLanguage.texts.noEnableSites,
                  '</div>'];
  for (var i = 0; i < len; i++) {
    var visible = this.settings_[i].getValue() ? '' : ' class=hidden';
    nameHtml.push('<div id=line-' + i + visible + '><a siteid=' + i + ' href=#>'
        + names[i] + '</a></div>');
  }
  _gel('regSites').innerHTML = nameHtml.join('');

  if (this.numEnabled_ == 0)
    _show(_gel('nosite'));

  // init links to site name in sites table on dashboard
  _arr(_gel('regSites').getElementsByTagName('a'), function(site){
    if (site.getAttribute('siteid') == null) return;

    _event(site, 'click', function(e, t){
      _getPager().gotoSite(t.getAttribute('siteid'), 'Status');
    });
  });

  _event(_gel('siteMngLink'), 'click', function() {
    _getPager().gotoPage('SiteManage');
  });

  // load site switcher
  this.siteSwitcher_.load(names);
};

/**
 * Validates the site index.
 * @param {Number} index  The site index
 * @return {Boolean} If the site index is valid.
 * @private
 */
SiteManageGroup.prototype.checkSiteIndex_ = function(index) {
  return typeof index == 'number' && index >= 0 && index < this.numSites_;
};

SiteManageGroup.prototype.siteName = function(siteIdx) {
  if (this.checkSiteIndex_(siteIdx)) {
    return this.xmlSites_[siteIdx].getAttribute('name');
  }
  return null;
};

SiteManageGroup.prototype.siteId = function(siteIdx) {
  if (this.checkSiteIndex_(siteIdx)) {
    return this.xmlSites_[siteIdx].getAttribute('site_id');
  }
  return null;
};
///////////////////////////////////////////////////////////////////////////
/**
 * This class represents a setting node of a site. Inherit from BaseSetting.
 * @constructor
 * @param {String} xmltag  The xmltag of the node
 */
function SiteGroup(xmltag, ownerGroup) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }
  // Set the name of the node
  SiteGroup.prototype.parent.constructor.call(this, xmltag, ownerGroup);

  // Adds attached settings
  var s;

  // add simple settings
  s = new Setting('maxUrlLifeInDays', 'max_url_life_in_days',
                  Setting.types.STRING);
  s.setValidator(ValidateManager.validators.NUMBER);
  this.addSetting_(s);

  s = new Setting('maxSiteUrlInMemory', 'max_url_in_memory',
                  Setting.types.STRING);
  s.setValidator(ValidateManager.validators.NUMBER);
  s.registListener(new SpaceConverter(s, 'maxSiteUrlInMemoryEqualSize'));
  this.addSetting_(s);

  s = new Setting('maxUrlInDisk', 'max_url_in_disk', Setting.types.STRING);
  s.setValidator(ValidateManager.validators.NUMBER);
  s.registListener(new SpaceConverter(s, 'maxUrlInDiskEqualSize'));
  this.addSetting_(s);

  // add list

  // TODO: enable it when integrate with new backend
  s = new Setting('queryFieldEditor', 'IncludedQueryFields',
                  Setting.types.QUERYFIELDS);
  s.setValidator(new ValidateManager('queryfieldInput'));
  this.addSetting_(s);

  // add sitemap enable setting
  var me = this;
  _arr(_allSitemaps(), function(id) {
    var htmlId = id + '-enable';
  // TODO: remove the hack prefix when integrate with new backend
    var s = new Setting(htmlId, id + '@enabled', Setting.types.BOOL);
    me.addSetting_(s);
  });


  /**
   * The sub setting nodes, each represent a sitemap.
   * @type {Array.<SitemapGroup>}
   */
  this.sitemaps_ = [
    new WebSitemapGroup('WebSitemapSetting', this),
    new NewsSitemapGroup('NewsSitemapSetting', this),
    new MobileSitemapGroup('MobileSitemapSetting', this),
    new CodeSearchSitemapGroup('CodeSearchSitemapSetting', this),
    new BlogSearchSitemapGroup('BlogSearchPingSetting', this)
  ];

  /**
   * Other sub setting nodes except sitemap.
   */
  this.webServerFilter_ =
      new WebServerFilterGroup('WebserverFilterSetting', this);
  this.fileScanner_ = new FileScannerGroup('FileScannerSetting', this);
  this.logParser_ = new LogParserGroup('LogParserSetting', this);

  /**
   * All sub setting nodes.
   * @type {Array.<ServiceGroup>}
   */
  this.allservices_ = this.sitemaps_.concat([
    this.webServerFilter_, this.fileScanner_, this.logParser_
  ]);
}
SiteGroup.inheritsFrom(SettingGroup);

/**
 * Checks if the general tab of this site is customized.
 * If any part in it is customized, it is customized.
 * @return {Boolean} If the general tab is customized
 */
SiteGroup.prototype.isCustomized = function(id) {
  Util.assert(id, 'param error for SiteGroup.prototype.isCustomized');
  switch (id) {
    case 'site':
      return SiteGroup.prototype.parent.isCustomized.call(this) ||
             this.webServerFilter_.isCustomized() ||
             this.fileScanner_.isCustomized() ||
             this.logParser_.isCustomized();
    case 'web':
      return this.sitemaps_[0].isCustomized();
    case 'news':
      return this.sitemaps_[1].isCustomized();
    case 'mobile':
      return this.sitemaps_[2].isCustomized();
    case 'codesearch':
      return this.sitemaps_[3].isCustomized();
    case 'blogsearch':
      return this.sitemaps_[4].isCustomized();
  }
};

SiteGroup.prototype.isEnabled = function(id) {
  switch (id) {
    case 'site':
      // assume that all the site that can be config is enabled.
      return true;
    case 'web':
    case 'news':
    case 'mobile':
    case 'codesearch':
    case 'blogsearch':
      return this.getSetting_(id + '@enabled').getValue();
  }
};
SiteGroup.prototype.setEnable = function(id, flag) {
  switch (id) {
    case 'site':
      Util.assert(false,
          'not support this param in SiteGroup.prototype.setEnable');
      break;
    case 'web':
    case 'news':
    case 'mobile':
    case 'codesearch':
    case 'blogsearch':
      this.getSetting_(id + '@enabled').setValue(flag);
      break;
  }
};

SiteGroup.prototype.revertToDefault = function(id) {
  switch (id) {
    case 'site':
      SiteGroup.prototype.parent.revertToDefault.call(this);
      break;
    case 'web':
      this.sitemaps_[0].revertToDefault();
      break;
    case 'news':
      this.sitemaps_[1].revertToDefault();
      break;
    case 'mobile':
      this.sitemaps_[2].revertToDefault();
      break;
    case 'codesearch':
      this.sitemaps_[3].revertToDefault();
      break;
    case 'blogsearch':
      this.sitemaps_[4].revertToDefault();
      break;
  }
};

/**
 * Validates all the general tab's settings in the site.
 * @return {Boolean} If all the settings in the site are valid.
 */
SiteGroup.prototype.validate = function() {
  return SiteGroup.prototype.parent.validate.call(this) &&
         this.webServerFilter_.validate() &&
         this.fileScanner_.validate() &&
         this.logParser_.validate();
};

/**
 * Loads values of the site setting from XML.
 * @param {XMLNode?} xml  The XML node of the site
 */
SiteGroup.prototype.load = function(xml) {
  // Load settings
  SiteGroup.prototype.parent.load.call(this, xml);

  // Load for sitemaps and crawlers
  var me = this;
  _arr(this.allservices_, function(service) {
    service.load(Util.checkUniqueAndReturn(me.xml_, service.xmltag()));
  });
};

/**
 * Gets the site name.
 * @return {String} The site name
 */
SiteGroup.prototype.name = function() {
  var s = this.getSetting_('name');
  return s ? s.getValue() : GLOBAL_SETTING_NAME;
};

/**
 * Gets the site id.
 * @return {String} The site id
 */
SiteGroup.prototype.id = function() {
  var s = this.getSetting_('site_id');
  return s ? s.getValue() : null;
};

SiteGroup.prototype.dirty = function() {
  return SiteGroup.prototype.parent.dirty.call(this) ||
         Util.array.checkIfAny(this.allservices_, function(s) {
           return s.dirty();
         });
};

SiteGroup.prototype.clearDirty = function() {
  SiteGroup.prototype.parent.clearDirty.call(this);
  _arr(this.allservices_, function(s) {
    s.clearDirty();
  });
};

/**
 * Sets the access right of the components of this setting, since there are more
 * than one reason to set the access right, we must record the reason.
 * @param {Boolean} readonly  If the setting is readonly
 * @param {String} reason  The reason to set the access right
 */
SiteGroup.prototype.setAccess = function(readonly, reason) {
  // Load components
  SiteGroup.prototype.parent.setAccess.call(this, readonly, reason);

  // Load for sitemaps and other settings
  _arr(this.allservices_, function(s) {
    s.setAccess(readonly, reason);
  });
};

/**
 * Gets the service setting of this site.
 * @param {SettingOperator.services.enums} type  The service type
 * @return {ServiceGroup?} The service setting object
 */
SiteGroup.prototype.getService = function(type) {
  return Util.array.find(this.allservices_, function(s) {
    return s.type() == type;
  });
};

/**
 * Saves the site setting to XML DOM.
 */
SiteGroup.prototype.save = function() {
  // Apply to components
  SiteGroup.prototype.parent.save.call(this);

  // Apply to sitemaps and other settings
  _arr(this.allservices_, function(s) {
    s.save();
  });
};


/**
 * Sets the global setting group that can be inherited by the setting node.
 * @param {GlobalSetting} global  The Global Setting object.
 */
SiteGroup.prototype.setGlobal = function(global) {
  // set global to components
  SiteGroup.prototype.parent.setGlobal.call(this, global);

  // set global for services
  _arr(this.allservices_, function(s) {
    s.setGlobal(global.getService(s.type()));
  });
};

///////////////////////////////////////////////////////////////////////////

/**
 * This class represents NormalSiteGroup, which inherits from SiteSetting.
 * @param {String} xmltag  The xmltag of the setting group
 * @param {GlobalSetting} global  The Global Setting object.
 */
function NormalSiteGroup(xmltag, ownerGroup, global) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }

  NormalSiteGroup.prototype.parent.constructor.call(this, xmltag, ownerGroup);

  // Adds attached settings
  var s;

  // add simple settings
  s = new Setting(null, 'name', Setting.types.STRING);
  s.setSiteSpecialFlag();
  this.addSetting_(s);

  s = new Setting(null, 'site_id', Setting.types.STRING);
  s.setSiteSpecialFlag();
  this.addSetting_(s);


  s = new Setting('host', 'host', Setting.types.STRING);
  s.setSiteSpecialFlag();
  s.setHtmlPrefix('http://');
  s.setValidator(new ValidateManager('host'));
  this.addSetting_(s);

  s = new Setting('logPath', 'log_path', Setting.types.STRING);
  s.setSiteSpecialFlag();
  s.setValidator(new ValidateManager('logPath'));
  this.addSetting_(s);

  // set global
  this.setGlobal(global);
}
NormalSiteGroup.inheritsFrom(SiteGroup);

//////////// Class ServiceGroup /////////////////
/**
 * This class represents a service, which inherits from BaseSetting.
 * @param {String} xmltag  The service xmltag
 * @param {SettingOperator.services.enums} type  The service type
 * @constructor
 */
function ServiceGroup(xmltag, type, ownerGroup) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }

  // Since ServiceGroup may be inherited by sub class, 'this.parent.xxx' may
  // be sub-class.parent.xxx, which is equal to ServiceGroup.prototype.xxx,
  // so directly using 'this.parent.xxx' to call parent method will cause
  // death-cycle call. We have to use explicit parent class name to call the
  // parent method.
  ServiceGroup.prototype.parent.constructor.call(this, xmltag, ownerGroup);
  this.type_ = type;

  // Add common components for sitemap setting.
  // If the sub sitemap has different components, can use
  // clearComponents_/deleteComponent/addComponent_ to rearrange.
}
ServiceGroup.inheritsFrom(SettingGroup);

/**
 * Gets the type of the service setting.
 * @return {ServiceGroup.types} The type of the service setting
 */
ServiceGroup.prototype.type = function() {
  return this.type_;
};

///////////////////////////////////////////////////////////////////////////
/**
 * This class represents a sitemap, which inherits from ServiceSetting.
 * @param {String} xmltag  The sitemap xmltag
 * @param {SettingOperator.services.enums} serviceType  The service type of
 * this sitemap.
 */
function SitemapGroup(xmltag, serviceType, ownerGroup) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }

  // Since SitemapGroup may be inherited by sub class, 'this.parent.xxx' may
  // be sub-class.parent.xxx, which is equal to SitemapGroup.prototype.xxx,
  // so directly using 'this.parent.xxx' to call parent method will cause
  // death-cycle call. We have to use explicit parent class name to call the
  // parent method.
  SitemapGroup.prototype.parent.constructor.call(this, xmltag, serviceType,
                                                   ownerGroup);

  // Add common components for sitemap setting.
  // If the sub sitemap has different components, can use
  // clearComponents_/deleteComponent/addComponent_ to rearrange.
  var idPrefix = SettingOperator.services.getSitemapHtmlId(serviceType);
	var s;
  if (serviceType != SettingOperator.services.enums.BLOGSEARCHPING) {

    // add simple components
    s = new Setting(idPrefix + 'Compress', 'compress',
                    Setting.types.BOOL);
    this.addSetting_(s);

    // TODO: implement the UI component for time when review is finished
    s = new Setting(idPrefix + 'UpdateStartTime', 'update_start_time',
                    Setting.types.DATE);
    s.setValidator(ValidateManager.validators.DATE);
    this.addSetting_(s);

    s = new Setting(idPrefix + 'FileName', 'file_name', Setting.types.STRING);
    s.setValidator(ValidateManager.validators.XMLFILE);
    this.addSetting_(s);

    s = new Setting(idPrefix + 'MaxFileUrl', 'max_file_url_number',
                    Setting.types.STRING);
    s.setValidator(new ValidateManager('number', '[1,50000]'));
    this.addSetting_(s);

    s = new Setting(idPrefix + 'MaxFileSize', 'max_file_size',
                    Setting.types.SPACESIZE);
    s.setValidator(new ValidateManager('number', '(0,10485760]'));
    this.addSetting_(s);
  }

  s = new Setting(idPrefix + 'Duration', 'update_duration_in_seconds',
                  Setting.types.RADIO);
  s.setValidator(ValidateManager.validators.RADIO);
  this.addSetting_(s);

  // add list components
  s = new Setting(idPrefix + 'InUrlEditor', 'IncludedUrls',
                  Setting.types.LIST);
  s.setValidator(new ValidateManager('urlmatchInput'));
  this.addSetting_(s);

  s = new Setting(idPrefix + 'ExUrlEditor', 'ExcludedUrls',
                  Setting.types.LIST);
  s.setValidator(new ValidateManager('urlmatchInput'));
  this.addSetting_(s);

}
SitemapGroup.inheritsFrom(ServiceGroup);


///////////////////////////////////////////////////////////////////////////
//////////// Class WebSitemapGroup /////////////////
/**
 * This class represents a web sitemap, which inherits from SitemapGroup.
 * @param {String} xmltag  The xmltag of the sitemap
 */
function WebSitemapGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.WEBSITEMAP,
                               ownerGroup);
  // add simple components
  var s = new Setting('robotsIncluded', 'included_in_robots_txt',
                      Setting.types.BOOL);
  this.addSetting_(s);

  s = new Setting('webNotifyUrlEditor', 'NotifyUrls',
                  Setting.types.LIST);
  s.setValidator(new ValidateManager('urlInput'));
  this.addSetting_(s);
}
WebSitemapGroup.inheritsFrom(SitemapGroup);

//////////// Class NewsSitemapGroup /////////////////
/**
 * This class represents a news sitemap, which inherits from SitemapGroup.
 * @param {String} xmltag  The xmltag of the sitemap
 */
function NewsSitemapGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.NEWSSITEMAP,
                               ownerGroup);
  // add simple components
  var s = new Setting('newsExpireDuration', 'expire_duration_in_seconds',
                      Setting.types.DURATION);
  s.setValidator(ValidateManager.validators.DURATION);
  this.addSetting_(s);
}
NewsSitemapGroup.inheritsFrom(SitemapGroup);

//////////// Class MobileSitemapGroup /////////////////
/**
 * This class represents a mobile sitemap, which inherits from SitemapGroup.
 * @param {String} xmltag  The xmltag of the sitemap
 */
function MobileSitemapGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.MOBILESITEMAP,
                               ownerGroup);
}
MobileSitemapGroup.inheritsFrom(SitemapGroup);

//////////// Class CodeSearchSitemapGroup /////////////////
/**
 * This class represents a code search sitemap,
 * which inherits from SitemapGroup.
 * @param {String} xmltag  The xmltag of the sitemap
 */
function CodeSearchSitemapGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.CODESEARCHSITEMAP,
                               ownerGroup);
}
CodeSearchSitemapGroup.inheritsFrom(SitemapGroup);

//////////// Class BlogSearchSitemapGroup /////////////////
/**
 * This class represents a blog search sitemap,
 * which inherits from SitemapGroup.
 * @param {String} xmltag  The xmltag of the sitemap
 */
function BlogSearchSitemapGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.BLOGSEARCHPING,
                               ownerGroup);
}
BlogSearchSitemapGroup.inheritsFrom(SitemapGroup);

//////////// Class WebServerFilterGroup /////////////////
/**
 * This class represents webserver filter setting,
 * which inherits from BaseSetting.
 * @param {String} xmltag  The xmltag of the node
 */
function WebServerFilterGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.WEBSERVERFILTER,
                               ownerGroup);
  // add components
  this.addSetting_(
      new Setting('wsf-eanbled', 'enabled', Setting.types.BOOL));

}
WebServerFilterGroup.inheritsFrom(ServiceGroup);

WebServerFilterGroup.prototype.isEnabled = function() {
  return this.getSetting_('enabled').getValue();
};

//////////// Class LogParserGroup /////////////////
/**
 * This class represents log parser setting,
 * which inherits from BaseSetting.
 * @param {String} xmltag  The xmltag of the node
 */
function LogParserGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.LOGPARSER,
                               ownerGroup);
  // add components
  var s1 = new Setting('lp-eanbled', 'enabled', Setting.types.BOOL);
  this.addSetting_(s1);

  var s2 = new Setting('lp-duration', 'update_duration_in_seconds',
                       Setting.types.DURATION);
  s2.setValidator(ValidateManager.validators.DURATION);
  // TODO: this is hack for set access
  s2.setLabelManager(new LabelManager(['lp-duration-area']));
  this.addSetting_(s2);
}
LogParserGroup.inheritsFrom(ServiceGroup);

//////////// Class FileScannerGroup /////////////////
/**
 * This class represents file scanner setting,
 * which inherits from BaseSetting.
 * @param {String} xmltag  The xmltag of the node
 */
function FileScannerGroup(xmltag, ownerGroup) {
  this.parent.constructor.call(this, xmltag,
                               SettingOperator.services.enums.FILESCANNER,
                               ownerGroup);
  // add components
  var s1 = new Setting('fs-eanbled', 'enabled', Setting.types.BOOL);
  this.addSetting_(s1);

  var s2 = new Setting('fs-duration', 'update_duration_in_seconds',
                       Setting.types.DURATION);
  s2.setValidator(ValidateManager.validators.DURATION);
  s2.setLabelManager(new LabelManager(['fs-duration-area']));
  this.addSetting_(s2);
}
FileScannerGroup.inheritsFrom(ServiceGroup);


