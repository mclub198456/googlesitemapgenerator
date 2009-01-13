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
 * @fileoverview  This file includes the class that deal with the localization
 * of the UI. It fills the web page with the texts in the language.js file.
 *
 * It deals with three type of element's attribute's translation:
 * 1. text: the text content of an element, such as innerHTML of SPAN elements.
 * 2. tip: the tip of a setting.
 * 3. value: the value of a button.
 *
 * The elements in the HTML that need to be deal with will have an attribute
 * named 'transmark', which value will be 'text:mark1;tip:mark2', etc. Here
 * 'mark1' or 'mark2' is the name of the mark, it will be used to get mark value
 * from GSGLang in languages.js. The mark value is the language-
 * specific string which will appears on the UI.
 */

var TransMarkSet = {};
TransMarkSet.marks_ = {texts: {}, tips: {}, values: {}};

/**
 * Transmark types.
 */
TransMarkSet.types = {TEXT: 't', TIP: 'i', VALUE: 'v'};

/**
 * Extracts certain type of mark from this markset.
 * @param {String} markset  The value of the 'transmark' attribute
 * @param {TransMarkSet.types} type  The mark type
 * @return {String?} The mark name, or null if no such type of mark in the
 *   markset
 * @private
 */
TransMarkSet.getMark_ = function(markset, type) {
  var marks = markset.split(/[:;]/);
  if (marks.length < 2) {
    return null;
  }
  for (var i = 0; i < marks.length - 1; i += 2) {
    var name = marks[i];
    var mark = marks[i + 1];
    if (name == type) {
      return mark;
    }
  }
  return null;
};

/**
 * Generates the markset according to the given marks.
 * @param {String} text  The mark name of 'TEXT' type
 * @param {String} tip  The mark name of 'TIP' type
 * @param {String} value  The mark name of 'VALUE' type
 * @return {String} The markset
 * @private
 */
TransMarkSet.fillMark_ = function(text, tip, value) {
  var markset = '';
  if (text)
    markset += TransMarkSet.types.TEXT + ':' + text + ';';
  if (tip)
    markset += TransMarkSet.types.TIP + ':' + tip + ';';
  if (value)
    markset += TransMarkSet.types.VALUE + ':' + value + ';';

  return markset == '' ? null : markset.slice(0, -1); // remove the last ';'
};

/**
 * Provides a way to modify the mark names in the markset.
 * @param {String} markset  The markset
 * @param {Object} modifier  The function that does the modification, which has
 *   the following params and return value:
 *     param {TransMarkSet.types} type  The mark type
 *     param {String} value  The original mark name of the type
 *     return {String} The modified mark name of the type
 */
TransMarkSet.modifyMark = function(markset, modifier) {
  var text = TransMarkSet.getMark_(markset, TransMarkSet.types.TEXT);
  var tip = TransMarkSet.getMark_(markset, TransMarkSet.types.TIP);
  var value = TransMarkSet.getMark_(markset, TransMarkSet.types.VALUE);

  text = modifier(TransMarkSet.types.TEXT, text);
  tip = modifier(TransMarkSet.types.TIP, tip);
  value = modifier(TransMarkSet.types.VALUE, value);

  return TransMarkSet.fillMark_(text, tip, value);
};

/////////////////////
/**
 * Gets the mark value of the 'TEXT' type in the markset.
 * @param {String} markset  The markset
 * @return {String} The mark value (language-specific string)
 * @private
 */
TransMarkSet.getText_ = function(markset) {
  var mark = TransMarkSet.getMark_(markset, TransMarkSet.types.TEXT);
  if (IS_DEBUG) TransMarkSet.marks_.texts[mark] = true;
  return GSGLang.texts[mark];
};

/**
 * Gets the mark value of the 'TIP' type in the markset.
 * @param {String} markset  The markset
 * @return {String} The mark value (language-specific string)
 * @private
 */
TransMarkSet.getTip_ = function(markset) {
  var mark = TransMarkSet.getMark_(markset, TransMarkSet.types.TIP);
  if (IS_DEBUG) TransMarkSet.marks_.tips[mark] = true;
  return GSGLang.tips[mark];
};

/**
 * Gets the mark value of the 'VALUE' type in the markset.
 * @param {String} markset  The markset
 * @return {String} The mark value (language-specific string)
 * @private
 */
TransMarkSet.getValue_ = function(markset) {
  var mark = TransMarkSet.getMark_(markset, TransMarkSet.types.VALUE);
  if (IS_DEBUG) TransMarkSet.marks_.values[mark] = true;
  return GSGLang.values[mark];
};

/////////////////////
/**
 * Gets the element's tip, not only the static-text, maybe add some thing that
 * is dynamically generated if needed.
 * @param {Element} elem  The element to deal with
 * @return {String?} The language-specific tip of the element
 * @private
 */
TransMarkSet.getElementTip_ = function(elem) {
  var mark = elem.getAttribute(TRANSMARK_ATTRNAME);
  if (mark && mark != '') { // have transmark attribute
    var tip = TransMarkSet.getTip_(mark);
    if (tip) { // valid transmark
      return tip;
    }
  }
  return null;
};

/**
 * Gets the element's value of the 'value' attribute.
 * @param {Element} elem  The element to deal with
 * @return {String?} The language-specific value of the element
 * @private
 */
TransMarkSet.getElementValue_ = function(elem) {
  var mark = elem.getAttribute(TRANSMARK_ATTRNAME);
  if (mark && mark != '') { // have transmark attribute
    var value = TransMarkSet.getValue_(mark);
    if (value) { // valid transmark
      return value;
    }
  }
  return null;
};

/**
 * Gets the element's text content.
 * @param {Element} elem  The element to deal with
 * @return {String?} The language-specific text of the element
 * @private
 */
TransMarkSet.getElementText_ = function(elem) {
  var mark = elem.getAttribute(TRANSMARK_ATTRNAME);
  if (mark && mark != '') { // have transmark attribute
    var text = TransMarkSet.getText_(mark);
    if (text) { // valid transmark
      return text;
    }
  }
  return null;
};

/////////////////////
/**
 * Does localization for element's popup tips.
 * @param {Element} elem
 */
TransMarkSet.localizeTip = function(elem, opt_bIcon) {
  var tip = TransMarkSet.getElementTip_(elem);
  if (tip) {
    elem.tip = tip;
    if (opt_bIcon) {
      Tips.scheduleIcon(elem);
    } else {
      Tips.schedule(elem);
    }
  }
};

/**
 * Does localization for element's 'innerHTML' attribute.
 * @param {Element} elem
 */
TransMarkSet.localizeInnerHTML = function(elem) {
  var text = TransMarkSet.getElementText_(elem);
  if (text) {
    elem.innerHTML = text;
  }
};


/**
 * Does localization for element's 'value' attribute.
 * @param {Element} elem
 */
TransMarkSet.localizeValue = function(elem) {
  var value = TransMarkSet.getElementValue_(elem);
  if (value) {
    elem.value = value;
  }
};

/**
 * Does localization for 'SELECT' element.
 * @param {Element} elem  The 'SELECT' element
 */
TransMarkSet.localizeSelectElement = function(elem) {
  // select has tip
  TransMarkSet.localizeTip(elem);

  // option has text
  _arr(elem.options, function(option) {
    var text = TransMarkSet.getElementText_(option);
    if (text) {
      option.innerHTML = text;
    }
  });
};

TransMarkSet.getUnusedTransMarks = function() {
  var unused = [];
  if (!IS_DEBUG) return unused;
  _arr(['texts', 'tips', 'values'], function(type) {
    for (var mark in GSGLang[type])
      if (!(mark in TransMarkSet.marks_[type]))
        unused.push(type + '.' + mark);
  });
  return unused;
};
