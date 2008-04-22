// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview
 * This file includes the classes that used to build the core-logical model of 
 * the site setting. It's a setting object tree, which act as a bridge between
 * the XML DOM Tree (The configuration data from server) and the HTML DOM Tree
 * (The UI to user). Each middle node (BaseSetting) of the tree represents a
 * setting group, while each leaf (SettingComponent, in another JS file)
 * represents a single setting.
 * 
 * The setting object tree has the following tasks:
 * 1. Can load/save value from/to XML DOM.
 * 2. Can load/save value from/to HTML DOM.
 * 3. Can validate the setting.
 *
 * In XML DOM, each setting group has a root element, all the single value
 * setting fields are the attributes of the root element, while the complex
 * setting fields that have multiple sub-setting items are treated as
 * lists and exist as sub-elements of the site element. The list elements 
 * also have sub-elements as the list items , and the setting values of each
 * list item are the attributes of the list item element.
 *
 * @author chaiying@google.com
 */

//////////// Class BaseSetting /////////////////
/**
 * The base setting group class, represents as a middle node of the setting 
 * Tree.
 * @param {String} xpath  The xpath of the setting group
 */
function BaseSetting(xpath) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }
  /**
   * The setting components as leaves of this node.
   * @type {Array.<SettingComponent>}
   */
  this.components_ = [];

  /**
   * The XML node corresponding to this setting node.
   * It will be set in the 'load' function.
   * @type {XmlManager}
   */
  this.xml_ = null;

  /**
   * The xpath of the setting XML node.
   * Usually it's the tag name of the XML node of this setting.
   * @type {String}
   */
  this.xpath_ = xpath;
}

/**
 * Gets the xpath of the setting.
 * @return {String} The xpath of the setting
 */
BaseSetting.prototype.xpath = function() {
  return this.xpath_;
};

/**
 * Checks if the setting group is customized, if any setting component in this
 * setting group is not inherited, it will be treated as customized.
 * @return {Boolean} If the setting is customeized
 */
BaseSetting.prototype.isCustomizedXML = function() {
  var res = false;
  Util.array.applyWithBreak(this.components_, function(comp) {
    if (comp instanceof CustomizeSettingComponent || comp.isSiteSpecial()) {
      // ignore CustomizeSettingComponent
      return true; // continue iterate
    }
    if (!comp.isInherited()) {
      res = true;
      return false; // stop iterate
    }
    return true; // continue iterate
  });
  return res;
};

/**
 * Validates the components in the setting group.
 * @return {Boolean} If the setting components are all valid
 */
BaseSetting.prototype.validate = function() {
  var res = true;
  Util.array.applyWithBreak(this.components_, function(comp) {
    if (!comp.validate()) {
      res = false;
      return false; // stop iterate
    }
    return true; // continue iterate
  });
  return res;
};

/**
 * Focus on the first setting element in the setting group.
 */
BaseSetting.prototype.focusOnFirst = function() {
  this.components_[0].focus();
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
 * @param {XmlManager} xml  The XML node of this setting
 */
BaseSetting.prototype.load = function(xml) {
  if (xml)
    this.xml_ = xml;

  var that = this;
  Util.array.apply(this.components_, function(component) {
    component.load(that.xml_);
  });
};

/**
 * Saves the setting value to XML node.
 */
BaseSetting.prototype.save = function() {
  Util.array.apply(this.components_, function(component) {
    component.save();
  });
};

/**
 * Sets inherit property of the setting group.
 * @param {Boolean} isInherited  The value of the inherit property
 */
BaseSetting.prototype.setInherit = function(isInherited) {
  Util.array.apply(this.components_, function(component) {
    component.setInherit(isInherited);
  });
};

/**
 * Sets the global setting group that can be inherited by the setting node.
 * @param {BaseSetting?} global  The same level node in global setting
 */
BaseSetting.prototype.setGlobal = function(global) {
  Util.array.apply(this.components_, function(component) {
    component.setGlobal(global.getComponent_(component.name()));
  });
};

/**
 * Sets the access right of the components of this setting, since there are more
 * than one reason to set the access right, we must record the reason.
 * @param {Boolean} readonly  If the setting is readonly
 * @param {String} reason  The reason to set the access right
 */
BaseSetting.prototype.setAccess = function(readonly, reason) {
  Util.array.apply(this.components_, function(component) {
    component.setAccess(readonly, reason);
  });
};

/**
 * Gets the setting component value that is in this setting group.
 * @param {String} name  The component name, which is equal to the XML node tag
 *   or attribute name
 * @return {Boolean|String|Array.<Array.<String>>|null} The value of the setting
 *   component
 */
BaseSetting.prototype.getValue = function(name) {
  var comp = this.getComponent_(name);
  return comp ? comp.getValue() : null;
};

/**
 * Sets value of the setting component in the setting group.
 * @param {String} name  The component name, which is equal to the XML node tag
 *   or attribute name
 * @param {Boolean|String|Array.<Array.<String>>} value  The value of the
 *   setting component
 */
BaseSetting.prototype.setValue = function(name, value) {
  var comp = this.getComponent_(name);
  if (comp != null)
    comp.setValue(value);
};

/**
 * Gets the setting component in the setting group.
 * @param {String} name  The component name, which is equal to the XML node tag
 *   or attribute name
 * @return {SettingComponent?} The component object
 * @private
 */
BaseSetting.prototype.getComponent_ = function(name) {
  var res = null;
  Util.array.apply(this.components_, function(comp) {
    if (comp.name() == name) {
      res = comp;
    }
  });
  return res;
};

/**
 * Adds setting component to the node.
 * @param {SettingComponent} component  The component object
 * @private
 */
BaseSetting.prototype.addComponent_ = function(component) {
  this.components_.push(component);
};

/**
 * Removes leaf setting components from the node.
 * @private
 */
BaseSetting.prototype.clearComponents_ = function() {
  this.components_ = [];
};

/**
 * Tells the name of the node and the string value of the attached leaf
 * settings.
 * @return {String} The string representation of the node
 */
BaseSetting.prototype.toString = function() {
  var strVals = [];
  strVals.push(this.xpath_);
  Util.array.apply(this.components_, function(component) {
    strVals.push(component.toString());
  });
  strVals.push('\n');
  return strVals.join('\n');
};

//////////// Class SiteSettings /////////////////
/**
 * This class represents the whole setting Tree, it is at the top level.
 * Inherits from BaseSetting, no sub class.
 * @constructor
 */
function SiteSettings() {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }
  // set the name of the node
  this.parent.constructor.call(this, 'SiteSettings');

  // add the leaf settings of the node
  this.addComponent_(new SimpleSettingComponent(
      '@auto_add', 'autoAdd', SimpleSettingComponent.types.BOOLEAN));

  this.addComponent_(new SimpleSettingComponent(
      '@remote_admin', 'remoteAccess',
      SimpleSettingComponent.types.BOOLEAN));

  this.addComponent_(new SimpleSettingComponent(
      '@backup_duration_in_seconds', 'backupDurationInSeconds',
      SimpleSettingComponent.types.DURATION,
      ValidateManager.validators.DURATION));

  this.addComponent_(new SimpleSettingComponent(
      '@setting_port', 'settingPort',
      SimpleSettingComponent.types.STRING,
      new ValidateManager('number', '(0,65536)')));

  this.addComponent_(new SimpleSettingComponent(
      '@admin_name', null,
      SimpleSettingComponent.types.STRING, {readonly: true}));

  this.addComponent_(new SimpleSettingComponent(
      '@admin_password', 'password',
      SimpleSettingComponent.types.STRING, 
      ValidateManager.validators.PASSWORD));
  /**
   * The sub node of global setting.
   * @type {SiteSetting}
   */
  this.global_ = new GlobalSetting('GlobalSetting', this);

  /**
   * The sub node of site setting.
   * @type {SiteSetting}
   */
  this.site_ = new NormalSiteSetting('SiteSetting', this.global_);

  this.currentSite_ = GLOBAL_SETTING_ID;
}
SiteSettings.inheritsFrom(BaseSetting);

/**
 * Gets the SiteSettings singleton object.
 * @return {SiteSettings} The SiteSettings singleton object
 */
SiteSettings.getInstance = function() {
  if (SiteSettings.getInstance.instance_ == null) {
    SiteSettings.getInstance.instance_ = new SiteSettings();
  }
  return SiteSettings.getInstance.instance_;
};

/**
 * Loads values of the whole setting tree from XML. Actually, it delay the
 * normal site setting's load until the user requires it.
 * 
 * load(dom, null): first load, if this.currentSite_ != null, reload the current
 *     site, else, don't load any site except global setting.
 * load(dom, siteId): never happen now.
 * load(null, siteId): load the site.
 * load(null, null): reload the current site, may be used to revert the user
 *     change(but not refetch the data from server) in future.
 * 
 * @param {Document?} dom  The XML Document object, if null, means it's
 * a switch-site load
 * @param {String?} siteId  Which site should be loaded now
 */
SiteSettings.prototype.load = function(dom, siteId) {
  // only do it when init page or refresh page
  if (dom) {
    // The XML Document object
    this.xmldom_ = new XmlManager(dom);

    // The XML root node which tag is 'SiteSettings'
    this.xml_ = new XmlManager(dom.documentElement);

    // Load global setting
    this.global_.load(this.xml_.getNodeByXpath('GlobalSetting'));

    // Get the sites xml nodes, but load on demand
    this.xmlSites_ = this.xml_.getNodesByXpath('SiteSetting');
  }


  /* Load site setting if necessary*/
  if (siteId) {
    if (this.currentSite_ == siteId && !dom) {// already load
      return;
    }
    this.currentSite_ = siteId;
  } else {
    if (!this.currentSite_) {// no site need load
      return;
    }
  }

  // since global share the HTML with normal site, must load again each time
  if (this.currentSite_ == GLOBAL_SETTING_ID) {
    if (!dom) {
      this.global_.load(null);
    }
    this.global_.setAccess(false, 'setInherit');
  } else {
    this.site_.load(this.xmlSites_[parseInt(this.currentSite_)]);
  }
};

/**
 * Gets current site setting.
 */
SiteSettings.prototype.site = function() {
  return this.currentSite_ == GLOBAL_SETTING_ID ? this.global_ : this.site_;
};

/**
 * Saves the whole setting tree to XML DOM.
 */
SiteSettings.prototype.save = function() {
  // just save the current site, the other site has been saved before
  if (this.currentSite_ == GLOBAL_SETTING_ID) {
    // Apply to global setting
    this.global_.save();
  } else {
    // Apply to site settings
    this.site_.save();
  }

};

/**
 * Validates the whole setting tree.
 * @return {Boolean} If settings in the whole setting tree are all valid
 */
SiteSettings.prototype.validateAll = function() {
  return this.global_.validateAll() && this.site_.validateAll();
};

/**
 * Gets the number of total sites.
 * @return {Number} The number of total sites
 */
SiteSettings.prototype.sitesNum = function() {
  return this.xmlSites_.length;
};

/**
 * Validates the site index.
 * @param {Number} index  The site index
 * @return {Boolean} If the site index is valid.
 * @private
 */
SiteSettings.prototype.checkSiteIndex_ = function(index) {
  if (typeof index != 'number' || index < 0 || index >= this.xmlSites_.length) {
    return false;
  }
  return true;
};

/**
 * Gets site name in XML.
 * @param {String|Number|null} opt_siteIdx  The site identifier, 'g' for global
 *   setting, 'Num' for the normal site. 'null' means current site.
 * @return {String?} The site name
 */
SiteSettings.prototype.siteName = function(opt_siteIdx) {
  var siteIdx = opt_siteIdx || this.currentSite_;
  if (siteIdx == GLOBAL_SETTING_ID) {
    return this.global_.name();
  } else {
    siteIdx = parseInt(siteIdx);
    if (this.checkSiteIndex_(siteIdx)) {
      return this.xmlSites_[siteIdx].getAttribute('name');
    }
  }

  return null;
};

/**
 * Gets site id in XML.
 * @param {String|Number|null} opt_siteIdx  The site identifier, 'g' for global
 *   setting, 'Num' for the normal site. 'null' means current site.
 * @return {String?} The site id
 */
SiteSettings.prototype.siteId = function(opt_siteIdx) {
  var siteIdx = opt_siteIdx || this.currentSite_;
  if (siteIdx == GLOBAL_SETTING_ID) {
    return this.global_.id();
  } else {
    siteIdx = parseInt(siteIdx);
    if (this.checkSiteIndex_(siteIdx)) {
      return this.xmlSites_[siteIdx].getAttribute('site_id');
    }
  }

  return null;
};

/**
 * Gets the port setting.
 * @return {Number} The port of the configuration
 */
SiteSettings.prototype.getPort = function() {
  return this.getComponent_('setting_port').getValueFromXML_();
};

//////////// Class SiteSetting /////////////////
/**
 * This class represents a setting node of a site. Inherit from BaseSetting.
 * @constructor
 * @param {String} xpath  The xpath of the node
 */
function SiteSetting(xpath) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }
  // Set the name of the node
  SiteSetting.prototype.parent.constructor.call(this, xpath);

  this.name_ = null;
  this.id_ = null;

  // Adds attached settings

  // add simple components
  this.addComponent_(new SimpleSettingComponent(
      '@enabled', 'siteEnabled',
      SimpleSettingComponent.types.BOOLEAN));
      
  this.addComponent_(new SimpleSettingComponent(
      '@max_url_life_in_days', 'maxUrlLifeInDays',
      SimpleSettingComponent.types.STRING,
      ValidateManager.validators.NUMBER));
      
  this.addComponent_(new SimpleSettingComponent(
      '@max_url_in_memory', 'maxSiteUrlInMemory',
      SimpleSettingComponent.types.STRING,
      ValidateManager.validators.NUMBER));

  this.addComponent_(new SimpleSettingComponent(
      '@max_url_in_disk', 'maxUrlInDisk',
      SimpleSettingComponent.types.STRING,
      ValidateManager.validators.NUMBER));

  this.addComponent_(new SimpleSettingComponent(
      '@add_generator_info', 'addGeneratorInfo',
      SimpleSettingComponent.types.STRING));

  // add list
  this.addComponent_(new ListSettingComponent(
      'UrlReplacements',
      {containerId: 'generalUrlReplaceListContainer',
      listTplId: 'urlRepListTpl'}));

  // BINDING
  this.getComponent_('max_url_in_disk').registListener(
      new SpaceConvertComponent('maxUrlInDiskEqualSize'),
      ListenerManager.listenTypes.SPACECONVERT);

  this.getComponent_('max_url_in_memory').registListener(
      new SpaceConvertComponent('maxSiteUrlInMemoryEqualSize'),
      ListenerManager.listenTypes.SPACECONVERT);

  /**
   * The sub setting nodes, each represent a sitemap.
   * @type {Array.<SitemapSetting>}
   */
  this.sitemaps_ = [
    new WebSitemapSetting('WebSitemapSetting'),
    new NewsSitemapSetting('NewsSitemapSetting'),
    new VideoSitemapSetting('VideoSitemapSetting'),
    new MobileSitemapSetting('MobileSitemapSetting'),
    new CodeSearchSitemapSetting('CodeSearchSitemapSetting'),
    new BlogSearchSitemapSetting('BlogSearchPingSetting')
  ];

  /**
   * Other sub setting nodes except sitemap.
   */
  this.webServerFilter_ = new WebServerFilterSetting('WebserverFilterSetting');
  this.fileScanner_ = new FileScannerSetting('FileScannerSetting');
  this.logParser_ = new LogParserSetting('LogParserSetting');

  /**
   * All sub setting nodes.
   * @type {Array.<ServiceSetting>}
   */
  this.allservices_ = this.sitemaps_.concat([
    this.webServerFilter_, this.fileScanner_, this.logParser_
  ]);
}
SiteSetting.inheritsFrom(BaseSetting);

/**
 * Sets the access right of the components of this setting, since there are more
 * than one reason to set the access right, we must record the reason.
 * @param {Boolean} readonly  If the setting is readonly
 * @param {String} reason  The reason to set the access right
 */
SiteSetting.prototype.setAccess = function(readonly, reason) {
  // Load components
  SiteSetting.prototype.parent.setAccess.call(this, readonly, reason);

  // Load for sitemaps and other settings
  var that = this;
  Util.array.apply(this.allservices_, function(service) {
	  service.setAccess(readonly, reason);
  });
};

/**
 * Checks if the webserver filter of this site is enabled.
 * @return {Boolean} If the webserver filter is enabled
 */
SiteSetting.prototype.isWebServerFilterEnabled = function() {
  return this.webServerFilter_.getComponent_('enabled').getValueFromHTML_();
};

/**
 * Checks if the site is enabled for sitemap generator.
 * @return {Boolean} If the site is enabled
 */
SiteSetting.prototype.isEnabled = function() {
  return this.getComponent_('enabled').getValueFromHTML_();
};

/**
 * Gets the service setting of this site.
 * @param {ServiceSetting.types} type  The service type
 * @return {ServiceSetting?} The service setting object
 */
SiteSetting.prototype.getService = function(type) {
  var res = null;
  Util.array.apply(this.allservices_, function(service) {
    if (service.type() == type)
      res = service;
  });

  return res;
};
/**
 * Loads values of the site setting from XML.
 * @param {XmlManager?} xml  The XML node of the site
 */
SiteSetting.prototype.load = function(xml) {
  // Load components
  SiteSetting.prototype.parent.load.call(this, xml);

  // Load for sitemaps and other settings
  var that = this;
  Util.array.apply(this.allservices_, function(service) {
	  service.load(that.xml_.getNodeByXpath(service.xpath()));
  });
};

/**
 * Saves the site setting to XML DOM.
 */
SiteSetting.prototype.save = function() {
  // Apply to components
  SiteSetting.prototype.parent.save.call(this);

  // Apply to sitemaps and other settings
  Util.array.apply(this.allservices_, function(service) {
	  service.save();
  });
};

/**
 * Gets the site name.
 * @return {String} The site name
 */
SiteSetting.prototype.name = function() {
  return this.name_;
};

/**
 * Gets the site id.
 * @return {String} The site id
 */
SiteSetting.prototype.id = function() {
  return this.id_;
};

////////////////// functions deal with Tab page //////////////////

/**
 * Gets the setting group that corresponding to the tab page in this site.
 * @param {TabPage.types} type  The type of the tab page
 * @return {BaseSetting?} The tab page setting group
 */
SiteSetting.prototype.settingTab = function(type) {
  if (type == TabPage.types.RUNTIME) {
    return null;
  }
  if (type == TabPage.types.GENERAL) {
    return this;
  }

  // other setting tabs are all sitemap setting
  var res = null;
  Util.array.apply(this.sitemaps_, function(sitemap) {
    if (sitemap.settingTabType() == type)
      res = sitemap;
  });

  return res;
};

/**
 * Checks if the general tab of this site is customized.
 * If any part in it is customized, it is customized.
 * @return {Boolean} If the general tab is customized
 */
SiteSetting.prototype.isCustomizedXML = function() {
  if (SiteSetting.prototype.parent.isCustomizedXML.call(this))
    return true;

  return this.webServerFilter_.isCustomizedXML() ||
         this.fileScanner_.isCustomizedXML() ||
         this.logParser_.isCustomizedXML();
};

/**
 * Validates all the general tab's settings in the site.
 * @return {Boolean} If all the settings in the site are valid.
 */
SiteSetting.prototype.validate = function() {
  if (!SiteSetting.prototype.parent.validate.call(this))
    return false;

  return this.webServerFilter_.validate() &&
         this.fileScanner_.validate() &&
         this.logParser_.validate();
};

/**
 * Sets the general tab's inherit property.
 * @param {Boolean} isInherited  The inherit property's value
 */
SiteSetting.prototype.setInherit = function(isInherited) {
  SiteSetting.prototype.parent.setInherit.call(this, isInherited);
  this.webServerFilter_.setInherit(isInherited);
  this.fileScanner_.setInherit(isInherited);
  this.logParser_.setInherit(isInherited);
};

//////////// Class GlobalSetting /////////////////
/**
 * This class represents GlobalSetting, which inherits from SiteSetting.
 * @param {String} xpath  The xpath of the setting group
 * @param {SiteSettings} owner  The SiteSettings object this setting belongs to
 */
function GlobalSetting(xpath, owner) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }
  GlobalSetting.prototype.parent.constructor.call(this, xpath);

  this.name_ = GLOBAL_SETTING_NAME;
  /**
   * @type {SiteSettings}
   */
  this.owner_ = owner;
}
GlobalSetting.inheritsFrom(SiteSetting);

/**
 * Checks from HTML that if the general tab of global setting is customized by 
 * user.
 * @return {Boolean} Always return true since the global setting will not
 *   inherit from other setting.
 */
GlobalSetting.prototype.isCustomized = function() {
  return true;
};

/**
 * Validates all the general tab's settings in global setting.
 * @return {Boolean} If all the settings in the site are valid.
 */
GlobalSetting.prototype.validate = function() {
  // Apply to sitesettings components too
  return GlobalSetting.prototype.parent.validate.call(this) &&
         BaseSetting.prototype.validate.call(this.owner_);
};

/**
 * Loads values of the global setting from XML. This method also loads the 
 * sitesettings' components, which is on the general tab in global setting.
 * @param {XmlManager?} xml  The XML node of the site
 */
GlobalSetting.prototype.load = function(xml) {
  GlobalSetting.prototype.parent.load.call(this, xml);
  // Load sitesettings components' values
  BaseSetting.prototype.load.call(this.owner_, this.owner_xml_);
};

/**
 * Focus on the first setting element in the general tab in global setting.
 */
GlobalSetting.prototype.focusOnFirst = function() {
  BaseSetting.prototype.focusOnFirst.call(this.owner_);
};

/**
 * Saves the site setting to XML DOM. It also saves the sitesettings'
 * components, which is on the general tab in global setting.
 */
GlobalSetting.prototype.save = function() {
  GlobalSetting.prototype.parent.save.call(this);
  // save sitesettings components' values
  BaseSetting.prototype.save.call(this.owner_);
};

/**
 * Sets the access right of the components of this setting, since there are more
 * than one reason to set the access right, we must record the reason.
 * @param {Boolean} readonly  If the setting is readonly
 * @param {String} reason  The reason to set the access right
 */
GlobalSetting.prototype.setAccess = function(readonly, reason) {
  GlobalSetting.prototype.parent.setAccess.call(this, readonly, reason);
  // save sitesettings components' values
  BaseSetting.prototype.setAccess.call(this.owner_, readonly, reason);
};
//////////// Class NormalSiteSetting /////////////////
/**
 * This class represents NormalSiteSetting, which inherits from SiteSetting.
 * @param {String} xpath  The xpath of the setting group
 * @param {GlobalSetting} global  The Global Setting object.
 */
function NormalSiteSetting(xpath, global) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }

  NormalSiteSetting.prototype.parent.constructor.call(this, xpath);

  this.customize_ = new CustomizeSettingComponent('generalCustomize', this);


  Util.array.apply(this.sitemaps_, function(sitemap) {
    var prefix = SitemapSetting.getSitemapPrefix_(sitemap.serviceType_);
    sitemap.customize_ = new CustomizeSettingComponent(
        prefix + 'Customize', sitemap);

    sitemap.addComponent_(sitemap.customize_);
  });


  this.addComponent_(this.customize_);
  this.addComponent_(new SimpleSettingComponent(
      '@name', null, SimpleSettingComponent.types.STRING,
      {isSiteSpecial: true, readonly: true}));

  this.addComponent_(new SimpleSettingComponent(
      '@site_id', null, SimpleSettingComponent.types.STRING,
      {isSiteSpecial: true, readonly: true}));

  this.addComponent_(new SimpleSettingComponent(
      '@host', 'host',
      SimpleSettingComponent.types.STRING, new ValidateManager('host'),
      {isSiteSpecial: true}));

  this.addComponent_(new SimpleSettingComponent(
      '@log_path', 'logPath',
      SimpleSettingComponent.types.STRING, new ValidateManager('logPath'),
      {isSiteSpecial: true}));

  this.setGlobal(global);
}
NormalSiteSetting.inheritsFrom(SiteSetting);

/**
 * Checks from HTML that if the general tab of site setting is customized by
 * user.
 * @return {Boolean} If this setting group is customized.
 */
NormalSiteSetting.prototype.isCustomized = function() {
  return !this.customize_.getValueFromHTML_();
};

/**
 * Sets the global setting group that can be inherited by the setting node.
 * @param {GlobalSetting} global  The Global Setting object.
 */
NormalSiteSetting.prototype.setGlobal = function(global) {
  // set global to components
  NormalSiteSetting.prototype.parent.setGlobal.call(this, global);

  // set global for this
  this.global_ = global;

  // set global for services
  Util.array.apply(this.allservices_, function(service) {
	  service.setGlobal(global.getService(service.type()));
  });
};

/**
 * Loads values of the site setting from XML. This method also loads the site
 * speical components and the customize component, which is on the general tab
 * in site setting.
 * @param {XmlManager?} xml  The XML node of the site
 */
NormalSiteSetting.prototype.load = function(xml) {
  NormalSiteSetting.prototype.parent.load.call(this, xml);

  this.name_ = this.getComponent_('name').getValueFromXML_();
  this.id_ = this.getComponent_('site_id').getValueFromXML_();

  this.customize_.load(!this.isCustomizedXML());
};

//////////// Class ServiceType /////////////////
/**
 * @constructor
 * @param {String} name  The name of the service type
 */
function ServiceType(name) {
  this.name_ = name;
}
/**
 * Gets the name of this service type.
 * @return {String} The name of the service type
 */
ServiceType.prototype.name = function() {
  return this.name_;
};

//////////// Class ServiceSetting /////////////////
/**
 * This class represents a service, which inherits from BaseSetting.
 * @param {String} xpath  The service xpath
 * @param {ServiceSetting.types} type  The service type
 */
function ServiceSetting(xpath, type) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }

  // Since ServiceSetting may be inherited by sub class, 'this.parent.xxx' may
  // be sub-class.parent.xxx, which is equal to ServiceSetting.prototype.xxx,
  // so directly using 'this.parent.xxx' to call parent method will cause
  // death-cycle call. We have to use explicit parent class name to call the
  // parent method.
  ServiceSetting.prototype.parent.constructor.call(this, xpath);
  this.type_ = type;

  // Add common components for sitemap setting.
  // If the sub sitemap has different components, can use
  // clearComponents_/deleteComponent/addComponent_ to rearrange.
}
ServiceSetting.inheritsFrom(BaseSetting);

/**
 * The type of services.
 * @enum {ServiceType}
 */
ServiceSetting.types = {
  WEBSERVERFILTER: new ServiceType('webServerFilter'), 
  LOGPARSER: new ServiceType('logParser'), 
  FILESCANNER: new ServiceType('fileScanner'),
  WEBSITEMAP: new ServiceType('webSitemap'), 
  NEWSSITEMAP: new ServiceType('newsSitemap'), 
  VIDEOSITEMAP: new ServiceType('videoSitemap'), 
  MOBILESITEMAP: new ServiceType('mobileSitemap'),
  CODESEARCHSITEMAP: new ServiceType('codeSearchSitemap'), 
  BLOGSEARCHPING: new ServiceType('blogSearchPing')
};

/**
 * Gets the type of the service setting.
 * @return {ServiceSetting.types} The type of the service setting
 */
ServiceSetting.prototype.type = function() {
  return this.type_;
};
//////////// Class SitemapSetting /////////////////
/**
 * This class represents a sitemap, which inherits from ServiceSetting.
 * @param {String} xpath  The sitemap xpath
 * @param {ServiceSetting.types} serviceType  The service type of this sitemap
 * @param {TabPage.types} settingTabType  Which tab page this sitemap setting is
 *   on.
 */
function SitemapSetting(xpath, serviceType, settingTabType) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }

  this.serviceType_ = serviceType;
  // Since SitemapSetting may be inherited by sub class, 'this.parent.xxx' may
  // be sub-class.parent.xxx, which is equal to SitemapSetting.prototype.xxx,
  // so directly using 'this.parent.xxx' to call parent method will cause
  // death-cycle call. We have to use explicit parent class name to call the
  // parent method.
  SitemapSetting.prototype.parent.constructor.call(this, xpath, serviceType);
  /**
   * @type {TabPage.types}
   */
  this.settingTabType_ = settingTabType;


  // Add common components for sitemap setting.
  // If the sub sitemap has different components, can use
  // clearComponents_/deleteComponent/addComponent_ to rearrange.
  var prefix = SitemapSetting.getSitemapPrefix_(serviceType);

  if (serviceType != ServiceSetting.types.BLOGSEARCHPING) {

    // add simple components
    this.addComponent_(new SimpleSettingComponent(
        '@enabled', prefix + 'Enabled',
        SimpleSettingComponent.types.BOOLEAN));

    this.addComponent_(new SimpleSettingComponent(
        '@compress', prefix + 'Compress',
        SimpleSettingComponent.types.BOOLEAN));

    this.addComponent_(new SimpleSettingComponent(
        '@update_duration_in_seconds', prefix + 'Duration',
        SimpleSettingComponent.types.DURATION,
        ValidateManager.validators.DURATION));

    this.addComponent_(new SimpleSettingComponent(
        '@update_start_time', prefix + 'UpdateStartTime', 
        SimpleSettingComponent.types.STRING,
        ValidateManager.validators.DATE));

    this.addComponent_(new SimpleSettingComponent(
        '@file_name', prefix + 'FileName', 
        SimpleSettingComponent.types.STRING, 
        ValidateManager.validators.XMLFILE));

    this.addComponent_(new SimpleSettingComponent(
        '@max_file_url_number', prefix + 'MaxFileUrl', 
        SimpleSettingComponent.types.STRING,
        new ValidateManager('number', '[1,50000]')));

    this.addComponent_(new SimpleSettingComponent(
        '@max_file_size', prefix + 'MaxFileSize',  
        SimpleSettingComponent.types.SPACESIZE,
        new ValidateManager('number', '(0,10485760]')));

    // add list components
    this.addComponent_(new ListSettingComponent(
        'IncludedUrls',
        {containerId: prefix + 'SitemapIncludedUrlListContainer',
        listTplId: 'urlPatternListTpl'}));
    this.addComponent_(new ListSettingComponent(
        'ExcludedUrls',
        {containerId: prefix + 'SitemapExcludedUrlListContainer',
        listTplId: 'urlPatternListTpl'}));
    this.addComponent_(new ListSettingComponent(
        'NotifyUrls',
        {containerId: prefix + 'SitemapNotifyUrlListContainer',
        listTplId: 'urlListTpl'}));
  }
}
SitemapSetting.inheritsFrom(ServiceSetting);

/**
 * Gets tab page type of the sitemap.
 * @return {TabPage.types} The tab page type
 */
SitemapSetting.prototype.settingTabType = function() {
  return this.settingTabType_;
};

/**
 * Loads values of the sitemap setting from XML. It also loads the customize
 * component on the sitemap tab.
 * in site setting.
 * @param {XmlManager?} xml  The XML node of the setting
 */
SitemapSetting.prototype.load = function(xml) {
  SitemapSetting.prototype.parent.load.call(this, xml);
  if (this.customize_) {
    this.customize_.load(!this.isCustomizedXML());
  }
};

/**
 * Checks from HTML that if the sitemap tab of this setting is customized by 
 * user.
 * @return {Boolean} If this setting group is customized.
 */
SitemapSetting.prototype.isCustomized = function() {
  if (!this.customize_)
    return true;
  return !this.customize_.getValueFromHTML_();
};

/**
 * Gets HTML id's prefix of this sitemap's HTML elements
 * @param {ServiceSetting.types} type  The service type of this sitemap
 * @return {String?} The prefix
 * @private
 */
SitemapSetting.getSitemapPrefix_ = function(type) {
  switch(type) {
  	case ServiceSetting.types.WEBSITEMAP:
      return 'web';
  		break;
  	case ServiceSetting.types.NEWSSITEMAP:
      return 'news';
  		break;
  	case ServiceSetting.types.VIDEOSITEMAP:
      return 'video';
  		break;
  	case ServiceSetting.types.MOBILESITEMAP:
      return 'mobile';
  		break;
  	case ServiceSetting.types.CODESEARCHSITEMAP:
      return 'codeSearch';
  		break;
  	case ServiceSetting.types.BLOGSEARCHPING:
      return 'blogSearch';
  		break;
  }
  return null;
};

//////////// Class WebSitemapSetting /////////////////
/**
 * This class represents a web sitemap, which inherits from SitemapSetting.
 * @param {String} xpath  The xpath of the sitemap
 */
function WebSitemapSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.WEBSITEMAP,
                               TabPage.types.WEB);
  // add simple components
  this.addComponent_(new SimpleSettingComponent(
      '@included_in_robots_txt', 'robotsIncluded',
      SimpleSettingComponent.types.BOOLEAN));
}
WebSitemapSetting.inheritsFrom(SitemapSetting);

//////////// Class NewsSitemapSetting /////////////////
/**
 * This class represents a news sitemap, which inherits from SitemapSetting.
 * @param {String} xpath  The xpath of the sitemap
 */
function NewsSitemapSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.NEWSSITEMAP,
                               TabPage.types.NEWS);
  // add simple components
  this.addComponent_(new SimpleSettingComponent(
      '@expire_duration_in_seconds', 'newsExpireDuration',
      SimpleSettingComponent.types.DURATION,
      ValidateManager.validators.DURATION));
}
NewsSitemapSetting.inheritsFrom(SitemapSetting);

//////////// Class VideoSitemapSetting /////////////////
/**
 * This class represents a video sitemap, which inherits from SitemapSetting.
 * @param {String} xpath  The xpath of the sitemap
 */
function VideoSitemapSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.VIDEOSITEMAP,
                               TabPage.types.VIDEO);
}
VideoSitemapSetting.inheritsFrom(SitemapSetting);

//////////// Class MobileSitemapSetting /////////////////
/**
 * This class represents a mobile sitemap, which inherits from SitemapSetting.
 * @param {String} xpath  The xpath of the sitemap
 */
function MobileSitemapSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.MOBILESITEMAP,
                               TabPage.types.MOBILE);
}
MobileSitemapSetting.inheritsFrom(SitemapSetting);

//////////// Class CodeSearchSitemapSetting /////////////////
/**
 * This class represents a code search sitemap,
 * which inherits from SitemapSetting.
 * @param {String} xpath  The xpath of the sitemap
 */
function CodeSearchSitemapSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.CODESEARCHSITEMAP,
                               TabPage.types.CODESEARCH);
}
CodeSearchSitemapSetting.inheritsFrom(SitemapSetting);

//////////// Class BlogSearchSitemapSetting /////////////////
/**
 * This class represents a blog search sitemap,
 * which inherits from SitemapSetting.
 * @param {String} xpath  The xpath of the sitemap
 */
function BlogSearchSitemapSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.BLOGSEARCHPING,
                               TabPage.types.BLOGSEARCH);

  var prefix = SitemapSetting.getSitemapPrefix_(
      ServiceSetting.types.BLOGSEARCHPING);
  this.clearComponents_();

  // add simple components
  if (this.customize_)
    this.addComponent_(this.customize_);

  this.addComponent_(new SimpleSettingComponent(
      '@enabled', prefix + 'Enabled',
      SimpleSettingComponent.types.BOOLEAN));

  this.addComponent_(new SimpleSettingComponent(
      '@update_duration_in_seconds', prefix + 'Duration',
      SimpleSettingComponent.types.DURATION,
      ValidateManager.validators.DURATION));

  // add list
  this.addComponent_(new ListSettingComponent(
      'IncludedUrls',
      {containerId: prefix + 'PingIncludedUrlListContainer',
      listTplId: 'urlPatternListTpl'}));
  this.addComponent_(new ListSettingComponent(
      'ExcludedUrls',
      {containerId: prefix + 'PingExcludedUrlListContainer',
      listTplId: 'urlPatternListTpl'}));
}
BlogSearchSitemapSetting.inheritsFrom(SitemapSetting);

//////////// Class WebServerFilterSetting /////////////////
/**
 * This class represents webserver filter setting,
 * which inherits from BaseSetting.
 * @param {String} xpath  The xpath of the node
 */
function WebServerFilterSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.WEBSERVERFILTER);
  // add components
  this.addComponent_(new SimpleSettingComponent(
      '@enabled', 'webserverFilterEnabled',
      SimpleSettingComponent.types.BOOLEAN));
}
WebServerFilterSetting.inheritsFrom(ServiceSetting);

//////////// Class LogParserSetting /////////////////
/**
 * This class represents log parser setting,
 * which inherits from BaseSetting.
 * @param {String} xpath  The xpath of the node
 */
function LogParserSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.LOGPARSER);

  // add components
  this.addComponent_(new SimpleSettingComponent(
      '@enabled', 'logParserEnabled',
      SimpleSettingComponent.types.BOOLEAN));

  this.addComponent_(new SimpleSettingComponent(
      '@update_duration_in_seconds', 'logParserDuration',
      SimpleSettingComponent.types.DURATION,
      ValidateManager.validators.DURATION));

  // binding
  this.getComponent_('enabled').registListener(
      this.getComponent_('update_duration_in_seconds'),
      ListenerManager.listenTypes.ACCESSCONTROL);
}
LogParserSetting.inheritsFrom(ServiceSetting);

//////////// Class FileScannerSetting /////////////////
/**
 * This class represents file scanner setting,
 * which inherits from BaseSetting.
 * @param {String} xpath  The xpath of the node
 */
function FileScannerSetting(xpath) {
  this.parent.constructor.call(this, xpath,
                               ServiceSetting.types.FILESCANNER);
  // add components
  this.addComponent_(new SimpleSettingComponent(
      '@enabled', 'fileScannerEnabled',
      SimpleSettingComponent.types.BOOLEAN));

  this.addComponent_(new SimpleSettingComponent(
      '@update_duration_in_seconds', 'fileScannerDuration',
      SimpleSettingComponent.types.DURATION,
      ValidateManager.validators.DURATION));

  // binding
  this.getComponent_('enabled').registListener(
      this.getComponent_('update_duration_in_seconds'),
      ListenerManager.listenTypes.ACCESSCONTROL);
}
FileScannerSetting.inheritsFrom(ServiceSetting);
