// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview Class 'Setting' includes all the logic for a setting field.
 *
 * @author chaiying@google.com
 */

//////////// Class Setting /////////////////
/**
 * 
 * @constructor
 * @param {Object} htmlId
 * @param {Object} xmlAttrName
 * @param {Object} type
 */
function Setting(htmlId, xmlAttrName, type) {
  this.name_ = xmlAttrName ? xmlAttrName : htmlId;
  this.type_ = type;

  // TODO: using subclass instead of 'type' param
  this.htmlId_ = htmlId;
  this.html_ = Setting.createHtmlInput(htmlId, type);
  this.xml_ = Setting.createXmlAttr(xmlAttrName, type);

  this.isInherited_ = null;
  this.isSiteSpecial_ = false;
  this.isLoading_ = false;
  this.dirty_ = false;
  this.listenerMng_ = null;
  this.labelMng_ = null;
  this.accessMng_ = null;
  this.validator_ = null;
  this.global_ = null;

  if (this.html_) {
    this.accessMng_ = new AccessManager();

    // responsible for value change by user
    this.html_.registValueChangeHandler(this, function(listener) {
      listener.onUserInput();
    });
  }

  // assume the components that share the same html element have the same
  // validator
  if (this.validator_) {
    var range = this.validator_.range();
    if (range) {
      this.html_.appendRangeToTip(range);
    }
  }

  Component.regist(this);
}

Setting.types = {BOOL: 0, STRING: 1, DURATION: 2, SPACESIZE: 3, LIST: 4,
                 RADIO: 5, QUERYFIELDS: 6, DATE: 7};

Setting.createHtmlInput = function(id, type) {
  if (id == null)
    return null;
  if (id == '')
    return new FakeHtmlInput();
  switch (type) {
    case Setting.types.BOOL:
      return new BoolHtmlInput(id);
    case Setting.types.DATE:
      return new TimeHtmlInput(id);
    case Setting.types.STRING:
    case Setting.types.DURATION:
    case Setting.types.SPACESIZE:
      return new TextHtmlInput(id);
    case Setting.types.RADIO:
      return new RadioGroup(id);
    case Setting.types.LIST:
      return new UrlList(id);
    case Setting.types.QUERYFIELDS:
      return new QueryFieldList(id);
    default:
      _err('Invalid setting type');
      return null;
  }
};

Setting.createXmlAttr = function(tag, type) {
  if (!tag)
    return null;
  switch (type) {
    case Setting.types.BOOL:
    case Setting.types.STRING:
    case Setting.types.DATE:
    case Setting.types.DURATION:
    case Setting.types.SPACESIZE:
    case Setting.types.RADIO:
      return new XmlAttr(tag);
    case Setting.types.LIST:
    case Setting.types.QUERYFIELDS:
      return new ListXmlAttr(ListXmlAttr.getListTagSet(tag));
    default:
      _err('Invalid setting type');
      return null;
  }
};

Setting.prototype.setHtmlPrefix = function(value) {
  this.html_.setPrefix(value);
};

Setting.prototype.setName = function(name) {
  this.name_ = name;
};

/**
 * Get value in html format.
 * Get value from html element prior to xml element.
 */
Setting.prototype.getValue = function() {
  if (this.html_ && this.html_.isElemOwner())
    return this.html_.getValue();
  else if (this.xml_)
    return this.convertValueX2H(this.xml_.getValue());
  else
    return null;
};
Setting.prototype.getXmlValue = function() {
  if (this.xml_)
    return this.xml_.getValue();
  else
    return null;
};
Setting.prototype.cancelHtmlValueChange = function() {
  this.X2H();
};
/**
 * for the value change caused by user.
 */
Setting.prototype.onUserInput = function() {
  // special handlers for special setting, on user inputs value
  switch (this.htmlId_) {
    case 'robotsIncluded':
      // add click warning for robots-included setting
      if (this.html_.getValue() && !confirm(ROBOTS_INCLUDE_CONFIRM_MSG)) {
        this.cancelHtmlValueChange();
        return;
      }
      break;
    case 'news-enable':
      if (this.html_.getValue())
        alert(NEWS_ENABLE_MSG);
      break;
    case 'remoteAccess':
      if (this.html_.getValue() && window.location.protocol != 'https:') {
        alert(REMOTE_ENABLE_MSG);
        this.cancelHtmlValueChange();
        return;
      }
      break;
  }

  if (this.isInherited_ && !this.html_.compareValue(this.global_.getValue())) {
    this.setInherit(false);
  }
  // must after setInherit.
  if (!this.validate()) {
    alert(VALIDATING_FAIL_MSG);
  }

  this.onHtmlValueChange();
  this.onValueChange();

  // value has changed, inform pager. Call it at last.
  _getPager().onSettingChange();
};
/**
 * no matter whether the value is changed for xml or html, by user or by code.
 * @param {Object} value
 */
Setting.prototype.onValueChange = function() {
  if (this.listenerMng_ && this.validate()) // after value is changed
    this.listenerMng_.inform();
};


Setting.prototype.onHtmlValueChange = function() {
  // dirty means only html is changed, but xml is not updated.
  this.setDirty(); // should set it no matter if it is valid.
};
/**
 * responsible for value change by code.
 * @param {Object} value
 */
Setting.prototype.setValue = function(value) {
  if (this.html_) {
    this.html_.setValue(value);
    this.onHtmlValueChange();
  }
  else if (this.xml_)
    this.xml_.setValue(this.convertValueH2X(value));
  else
    return;
  this.onValueChange();
};

Setting.prototype.release = function() {
  if (this.labelMng_) {
    this.labelMng_.release();
  }
  if (this.html_) {
    this.html_.release();
  }
};

/**
 * Sets the global setting component for this component.
 * @param {SettingComponent?} global  The inherit target of this
 * setting component
 */
Setting.prototype.setGlobal = function(global) {
  this.global_ = global;
};

/**
 * Sets the inherit relationship of this component.
 * @param {Boolean} isInherited  Is inherited or not
 */
Setting.prototype.setInherit = function(isInherited) {
  if (this.global_ == null)
    return;  // no global to inherit

  // trigger the inheritance
  this.isInherited_ = isInherited;

  if (isInherited) {
    // update the value
    this.html_.setValue(this.convertValueX2H(this.global_.getXml().getValue()));
  }
  this.html_.setDefault(isInherited);
};

/**
 * Checks if the setting is inherited from global setting.
 * @return {Boolean} If the setting i s inherited
 */
Setting.prototype.isCustomized = function() {
  if (!this.global_) {
    return false;
  }
  return !this.isInherited_;
};

Setting.prototype.revertToDefault = function() {
  if (this.isInherited_ === false) {
    // change from custom to default, need save
    this.setDirty();
  }
  this.setInherit(true);
};

Setting.prototype.getHtml = function() {
  return this.html_;
};

Setting.prototype.getXml = function() {
  return this.xml_;
};

Setting.prototype.name = function() {
  return this.name_;
};

/**
 * Sets access right to the setting, since there are more than one reason to set
 * the access right, we must record the reason.
 * @param {Boolean} readonly  If the setting is readonly
 * @param {String} reason  The reason to set the access right
 */
Setting.prototype.setAccess = function(readonly, reason) {
  if (!this.html_)
    return;

  this.accessMng_.set(readonly, reason);
  this.html_.setAccess(this.accessMng_.readonly());
  if (this.labelMng_) {
    this.labelMng_.setAccess(this.accessMng_.readonly());
  }
};

/**
 * Checks if the setting value is valid.
 * @return {Boolean} If the setting value is valid
 */
Setting.prototype.validate = function() {
  if (!this.validator_) {
    return true;
  }
  if (!this.html_) {
    return true; // return true is acceptable since the value will not be
                 // changed to invalid value
  }
  if (this.isInherited_) {
    return true; // don't check the inherited value, since the inherited value
                 // will not be saved.
  }
  // check the value in XML type
  var isValid =
      this.validator_.check(this.convertValueH2X(this.html_.getValue()));
  this.html_.setValid(isValid);
  return isValid;
};

Setting.prototype.setValidator = function(v) {
  if (this.type_ == Setting.types.LIST ||
      this.type_ == Setting.types.QUERYFIELDS) {
    this.html_.setValidator(v);
    return;
  }
  this.validator_ = v;
};

// functions for dirty flag.
Setting.prototype.clearDirty = function() {
  this.dirty_ = false;
};

Setting.prototype.dirty = function() {
  // Up to now, we will not change the value if it has no html element.
  if (this.html_ == null) {
    return false;
  }

  return this.dirty_;
};

Setting.prototype.setDirty = function() {
  if (this.isLoading_) {
    return;
  }
  // set the dirty flag in this setting of current site
  this.dirty_ = true;
};

Setting.prototype.setLabelManager = function(manager) {
  this.labelMng_ = manager;
};
Setting.prototype.setSiteSpecialFlag = function() {
  this.isSiteSpecial_ = true;
};

Setting.prototype.convertValueH2X = function(value) {
  switch (this.type_) {
    case Setting.types.BOOL:
      return value ? 'true' : 'false';
    case Setting.types.DURATION:
    case Setting.types.RADIO:
      return parseInt(value) * 60; // minutes to seconds
    case Setting.types.SPACESIZE:
      return parseInt(value) * 1024; // KBYTES to BYTES
    default:
      return value;
  }
};
Setting.prototype.convertValueX2H = function(value) {
  switch (this.type_) {
    case Setting.types.BOOL:
      return value == 'true';
    case Setting.types.DURATION:
    case Setting.types.RADIO:
      return parseInt(value) / 60; // seconds to minutes
    case Setting.types.SPACESIZE:
      return parseInt(value) / 1024; // BYTES to KBYTES
    default:
      return value;
  }
};

Setting.prototype.X2H = function() {
  if (this.html_ == null || this.xml_ == null) {
    return;
  }

  this.isLoading_ = true;
  var value;

  // Get value
  if (this.isInherited_ == null) { // first load
    // get value from xml
    value = this.getXmlValue();

    // if not found, get value from global
    if (value == null) {
      this.setInherit(true);
      value = this.global_.getXmlValue();
    } else {
      this.setInherit(false);
    }
  } else if (this.isInherited_) {
    value = this.global_.getXmlValue();
  } else {
    value = this.getXmlValue();
  }

  // Set value
  this.html_.setValue(this.convertValueX2H(value));
  this.onValueChange();
  this.isLoading_ = false;
};

Setting.prototype.H2X = function() {
  if (this.html_ == null || this.xml_ == null) {
    return;
  }

  if (this.isInherited_) {
    // remove the value in the XML
    if (!this.isSiteSpecial_) { // don't remove the value that is site special
      this.xml_.removeValue();
    }
    return;
  }

  this.xml_.setValue(this.convertValueH2X(this.html_.getValue()));
};

Setting.prototype.load = function(xml) {
  // since we may share one html element between two components, so we should
  // switch the owner each time we change the setting showing. We do this when
  // setting value to HtmlInput
  if (this.html_) {
    this.html_.setElemOwner();
  }
  if (!this.xml_) {
    return;
  }
  if (xml) {
    this.xml_.setNode(xml);
  }
  this.isInherited_ = null;
  this.X2H();
  if (this.labelMng_) {
    this.labelMng_.switchLabel(_getSetting().getCurSiteIdx());
  }
};

Setting.prototype.save = function() {
  if (this.dirty()) {
    if (this.name_ == 'IncludedQueryFields' &&
        (this.getXmlValue() == null || this.getXmlValue().length == 0) &&
        !confirm(PRIVACY_WARNING)) {
      this.cancelHtmlValueChange();
      alert('The changes in query fields have been reverted.');
      return;
    }
    this.H2X();
  }
};

Setting.prototype.registListener = function(listener) {
  if (this.listenerMng_ == null)
    this.listenerMng_ = new ListenerManager(this);

  this.listenerMng_.regist(listener);
};

/**
 * focus to this setting.
 */
Setting.prototype.focus = function() {
  if (!this.html_) {
    return;
  }
  try {
    this.html_.focus();
  } catch(e) {
  }
};
