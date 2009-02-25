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
 * @fileoverview Defines some utility classes for setting.js and settinggroup.js
 */

var Component = {
  objects_: [],
  regist: function(obj) {
    Component.objects_.push(obj);
  },
  releaseAll: function() {
    while (true) {
      var obj = Component.objects_.pop();
      if (!obj) {
        break;
      }
      obj.release();
    }
  }
};

/**
 * ListenerManager provides a mechanism to inform one component that another
 * component's value has been changed.
 * @param {SettingComponent} comp  The owner component of this listener manager
 * @constructor
 */
function ListenerManager(comp) {
  /**
   * @type {SettingComponent}
   */
  this.owner_ = comp;

  /**
   * @type {Object} It's a look-up dictionary. The key is the type for the
   * listening reason (<ListenerManager.listenTypes>), while the value is
   * the listening components ({Array.<SettingComponent>}).
   * when the owner component's value is changed, all the listening components
   * that regist to this listener manager will be informed.
   */
  this.listeners_ = [];
}

/**
 * Regists listening component.
 * @param {SettingComponent} comp  The listening component
 * @param {ListenerManager.listenTypes} type  The listening type
 */
ListenerManager.prototype.regist = function(comp) {
  this.listeners_.push(comp);
};

/**
 * Inform the listeners that the value of owner is changed.
 * Up to now, there are two places using the function:
 * 1. OnChange/onclick Event handler, in case that user edit the setting value.
 * 2. Update value from XML.
 *
 * Note: For checkbox, onclick will not trigger onchange in IE, while it will
 * in Firefox.
 */
ListenerManager.prototype.inform = function() {
  var speaker = this.owner_;
  _arr(this.listeners_, function(listener) {
    listener.alert(speaker);
  });
};

/////////////////////////////////
/**
 * Usage:
 *   var listenSetting = new Setting();
 *   var speakSetting = new Setting();
 *   speakSetting.registListener(new AccessListener(listenSetting));
 *
 * @param {Object} setting
 * @constructor
 */
function AccessListener(setting) {
  this.setting_ = setting;
}

/**
 * Deal with the change of the listening target component.
 * @param {SettingComponent} speaker  The listening target component
 */
AccessListener.prototype.alert = function(speaker) {
  // Changes this component's access right according to the target component's
   // value.
  this.setting_.setAccess(!(speaker.getHtml().getValue()), speaker.name());
};
//////////// Class SpaceConverter /////////////////
/**
 * @constructor
 * Usage:
 *   var urlNumSetting = new Setting();
 *   urlNumSetting.registListener(new SpaceConverter(spaceElemId));
 * @param {String} htmlId  The HTML id of the element
 */
function SpaceConverter(setting, htmlId) {
  this.elem_ = _gel(htmlId);
  this.elem_.setting_ = this;
  this.setting_ = setting;
  this.runtime_ = _getRuntime();
  this.runtime_.registOnLoad(this);
  Component.regist(this);
}

/**
 * Deal with the change of the listening target component.
 * @param {SettingComponent} speaker  The listening target component
 */
SpaceConverter.prototype.alert = function(speaker) {
  // Changes this component's value according to the target component's value.
  var urls = this.setting_.getHtml().getValue();
  var size = this.runtime_.avgUrlSize();
  this.elem_.innerHTML = Util.getSpaceString(urls * size);
};

SpaceConverter.prototype.release = function() {
  if (this.elem_) {
    this.elem_.setting_ = null;
    this.elem_ = null;
  }
};

//////////////////////////////////////////////////////////////////////////
/**
 * It provides a mechanism to control a component's access right.
 * Since there are more than one reason to affect the setting component's
 * access right, such as inherit from global setting or the service is disabled.
 * So we have to seperate their effect and record all the status of these
 * reasons. If any of them requires these setting to be readonly, it will be
 * readonly.
 *
 * We use a dictionary to record the status of all the reasons.
 *
 * @constructor
 */
function AccessManager() {
  /**
   * @type {Object} It's a dictionary, which key is the name of the reason, and
   * the value is the status ({Boolean}) of the reason.
   */
  this.disableFlags_ = {};
}

/**
 * Sets the access right of the setting component, since there are more
 * than one reason to set the access right, we must record the reason.
 * @param {Boolean} readonly  If the setting is readonly
 * @param {String} reason  The reason to set the access right
 */
AccessManager.prototype.set = function(readonly, reason) {
  this.disableFlags_[reason] = readonly;
};

/**
 * Gets the access right of the setting component.
 * @return {Boolean} The access right
 */
AccessManager.prototype.readonly = function() {
  var disableCount = 0;
  for (var id in this.disableFlags_) {
    if (this.disableFlags_[id])
      disableCount++;
  }

  // set the attribute
  if (disableCount == 0) {
    return false;
  } else {
    return true;
  }
};

AccessManager.util = {};
AccessManager.util.setAccessForElem = function(readonly, elem) {
  if (readonly) {
    elem.setAttribute('disabled', true);
    Util.CSS.addClass(elem, READONLY_CSS);
  } else {
    elem.removeAttribute('disabled');
    Util.CSS.removeClass(elem, READONLY_CSS);
  }
};

//////////////////////////////////////////////////////////////////////////
/**
 * @constructor
 * @param {String|RegExp|null} pattern  The pattern of the validated field
 * @param {String?} opt_range  The range of the number validated field
 * @param {Boolean?} opt_required  The validated field should not be empty
 */
function ValidateManager(pattern, opt_range, opt_required) {
  this.pattern_ = pattern;
  this.range_ = opt_range;
  this.required_ = opt_required;

  // set default pattern if no pattern given
  if (!this.pattern_) {
    if (this.required_ === true) {
      // '\\S' means any character that is not Unicode whitespace
      this.pattern_ = '\\S';
    } else {
      _err('invalid ValidateManager');
    }
  }
}

/**
 * Gets the range value.
 * @return {String} The range
 */
ValidateManager.prototype.range = function() {
  return this.range_;
};

/**
 * Default validators.
 * @enum {ValidateManager}
 */
ValidateManager.validators = {
  // From 10 minutes to infinite.
  DURATION: new ValidateManager('number', '[10,MAX)'),
  // Any integer that is bigger than zero.
  NUMBER: new ValidateManager('number', '(0,MAX)'),
  // Hard coding the rules for password.
  PASSWORD: new ValidateManager('password'),
  // yyyy-mm-dd hh:mm:ss
  DATE: new ValidateManager('date'),
  // End with "xml" extention.
  XMLFILE: new ValidateManager('xmlfilename')
};

/**
 * Checks if the 'INPUT-text' element has valid value.
 * @param {Element} elem  The 'INPUT-text' element
 * @param {Boolean} opt_highlight  If the invalid field should be hightlight
 * @return {Boolean} If the 'INPUT-text' element has valid value
 */
ValidateManager.prototype.check = function(value) {
  // do validation
  return this.validate_(value);
};

/**
 * Checks if the value is valid.
 * @param {String} value  The value to be checked
 * @return {Boolean} If the value is valid
 * @private
 */
ValidateManager.prototype.validate_ = function(value) {
  // If the value does not match the pattern, set the class to 'invalid'.
  var isValid;
  if (ValidateManager.matchPredefinedPattern_(this.pattern_, value) ||
      typeof this.pattern_.test == 'function' && this.pattern_.test(value) ||
      this.required_ === false && value == '') {
    isValid = true;

    // if it is 'number' and define 'range', check the value for the range
    if (this.pattern_ == 'number' && this.range_ && value != '' &&
        !ValidateManager.isInRange_(this.range_, value)) {
      isValid = false;
    }
  } else {
    isValid = false;
  }

  return isValid;
};

/**
 * Checks if the pattern will be a predefined pattern name, and to see if the
 * value matches the predefined pattern.
 * @param {String|RegExp} pattern  The pattern name
 * @param {String} value  The value to be checked
 * @return {Boolean} True if the pattern is a predefined pattern name and the
 * value matches the predefined pattern.
 * @private
 */
ValidateManager.matchPredefinedPattern_ = function(pattern, value) {
  if (pattern == 'password') { // hardcode 'password' pattern
    return value.length >= 5;
  }

  if (typeof pattern != 'string') { // can't be a predefined pattern name
    return false;
  }

  var predefinedPattern = ValidateManager.patterns_[pattern];
  if (!predefinedPattern) { // not found
    return false;
  }
  return predefinedPattern.test(value);
};

/**
 * Checks if the value is in the range.
 * @param {String} range  The range for the value
 * @param {String} val  The value to be checked
 * @return {Boolean} if the value is in range
 * @private
 */
ValidateManager.isInRange_ = function(range, val) {
  var matchs = range.match(/([\(\[])(\d*), *(\d*|MAX)([\)\]])/);
  if (!matchs) {
    return true;
  }

  var value = Number(val);
  if (isNaN(value)) {
    return false;
  }

  // have got range, check it!
  var lowerBoundFlag = matchs[1];
  var min = matchs[2];
  var max = matchs[3];
  var upperBoundFlag = matchs[4];
  if (min != '') {
    min = parseInt(min);
    if (lowerBoundFlag == '(' && !(value > min)) {
      return false;
    } else if (lowerBoundFlag == '[' && !(value >= min)) {
      return false;
    }
  }

  if (max != '') {
    if (max == 'MAX') {
      max = Number.MAX_VALUE;
    } else {
      max = parseInt(max);
    }
    if (upperBoundFlag == ')' && !(value < max)) {
      // Helper.debug('value '+value+', max '+max);
      return false;
    } else if (upperBoundFlag == ']' && !(value <= max)) {
      return false;
    }
  }
  return true;
};

/**
 * Sets the element's status (the background-color) for validation.
 * @param {Element} elem  The element to be set
 * @param {Boolean} valid  The validation result
 * @private
 */
ValidateManager.setElementValidation_ = function(elem, valid) {
  if (valid) {
    Util.CSS.removeClass(elem, INVALID_CSS);
    if (elem.id in ValidateManager.invalidElems) {
      ValidateManager.invalidElems[elem.id] = null;
      delete ValidateManager.invalidElems[elem.id];
    }
  } else {
    Util.CSS.addClass(elem, INVALID_CSS);
    ValidateManager.invalidElems[elem.id] = elem;
  }
};

/**
 * Clear all the validate markers, clear the invalid-elems database.
 */
ValidateManager.clear = function() {
  for (var p in ValidateManager.invalidElems) {
    var elem = ValidateManager.invalidElems[p];
    Util.CSS.removeClass(elem, INVALID_CSS);
    ValidateManager.invalidElems[p] = null;
    delete ValidateManager.invalidElems[p];
  }
};

ValidateManager.invalidElems = {};

/**
 * ToString function.
 * @return {String} the string representation of this object
 */
ValidateManager.prototype.toString = function() {
  return 'ValidateManager: {patter:' + this.pattern +
      '\trange:' + this.range +
      '\trequired:' + this.required + '}';
};

/**
 * Predefined patterns.
 * @enum {RegExp}
 * @private
 */
ValidateManager.patterns_ = {
  number: /^[\d]+$/,
  // Any ASCII word character. Equivalent to [a-zA-Z0-9_].
  xmlfilename: /^[\w]+\.xml$/,
  // Any ASCII words, plus dot.
  host: /^http:\/\/[\w-\.]*$/,
  // Any characters except newline or another Unicode line terminator.
  logPath: /^.*$/,
  //
  date: /^\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}$/,
  //pattern will hardcode into the function
  password: null,

  /**
   * URL patterns, the input can't be empty:
   * For URL setting content:
   * url = http://(A+\.)*A+(/B*)?   A = [\w-]  B = [\w- ./?%&=] = [A ./?%&=]
   * urlmatch = /C+   C = [\w- ./?%&=\*] = [B\*]
   *
   * For UI input:
   * urlInput = (http://)?(A+\.)*A+(/B*)?
   * urlmatchInput = /?C+   C = [\w- ./?%&=\*] = [B\*]
   */

  // http://...=
  urlInput: /^(http:\/\/)?([\w-]+\.)*[\w-]+(\/[\w- \.\/\?\%\&\=]*)?\=$/,

  // /..*..
  urlmatchInput: /^\/?[\w- \.\/\?\%\&\=\*]+$/,

  // abc,def
  queryfieldInput: /^[\w]+(,[ ]*[\w]+)*$/
};
