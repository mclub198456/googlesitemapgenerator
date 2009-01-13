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
 * @fileoverview Defines classes for getting/setting xml value.
 */
//////////////////////////////////XmlAttr//////////////////////////////
/**
 * @constructor
 * @param {Object} name
 */
function XmlAttr(name) {
  this.attrname_ = name;

  /**
   * This is the node that contain the attribute.
   */
  this.xmlnode_ = null;
}

XmlAttr.prototype.setNode = function(xml) {
  this.xmlnode_ = xml;
  // TODO: remove it when integrate with new backend
  if (this.attrname_ == 'add_generator_info') {
    this.xmlnode_ = this.xmlnode_.getElementsByTagName('GlobalSetting')[0];
  }
};

/**
 * Converts XML document to string.
 * @param {Document} xmldom  The xml document object
 * @return {String} the XML string of xmldom
 */
XmlAttr.serialize = function(xmldom) {
  if (typeof XMLSerializer != 'undefined') {
    try {
      return (new XMLSerializer()).serializeToString(xmldom);
    } catch (e) {
      _err('Failed to serialize the xmldom');
      return '';
    }
  } else if (xmldom.xml) {
    return xmldom.xml;
  } else {
    _err(
        'XML.serialize is not supported or we cannot serialize the xmldom');
    return '';
  }
};

/**
 *
 * @return {Boolean|String|null} The setting value. When it return null
 */
XmlAttr.prototype.getValue = function() {
  var value = this.xmlnode_.getAttribute(this.attrname_);
  if (value == null &&
      (this.attrname_ == 'host' || this.attrname_ == 'log_path')) {
    // These special settings are not in the XML until user set it in UI.
    value = '';
  }

  return value;
};

XmlAttr.prototype.setValue = function(value) {
  this.xmlnode_.setAttribute(this.attrname_, value);
};

XmlAttr.prototype.removeValue = function() {
  this.xmlnode_.removeAttribute(this.attrname_);
};

//////////////////////////////////ListXmlAttr//////////////////////////////
/**
 * @constructor
 * @param {Object} name
 */
function ListXmlAttr(name) {
  this.listname_ = name.listname;
  this.itemname_ = name.itemname;
  this.itemattrnames_ = name.attrnames;
  this.xmlnode_ = null;
}

ListXmlAttr.getListTagSet = function(tag) {
  switch (tag) {
    case 'IncludedQueryFields':
      return {listname: 'IncludedQueryFields', itemname: 'QueryField',
              attrnames: ['name', 'enable']};
    case 'IncludedUrls':
      return {listname: 'IncludedUrls', itemname: 'Url',
              attrnames: ['value', 'enabled']};
    case 'ExcludedUrls':
      return {listname: 'ExcludedUrls', itemname: 'Url',
              attrnames: ['value', 'enabled']};
    case 'NotifyUrls':
      return {listname: 'NotifyUrls', itemname: 'Url',
              attrnames: ['value', 'enabled']};
    default:
      return null;
  }
};

/**
 *
 * @param {Object} xml  The node that contains the list node.
 */
ListXmlAttr.prototype.setNode = function(xml) {
  this.xmlnode_ = xml;
};

ListXmlAttr.prototype.getValue = function() {
  var listNode = this.getListNode_();
  if (!listNode) {
    return null;
  }

  var attrnames = this.itemattrnames_;
  var elements = listNode.getElementsByTagName(this.itemname_);
  return _arr(elements, function(elem) {
    return _arr(attrnames, function(attrname) {
      return elem.getAttribute(attrname);
    });
  });
};

ListXmlAttr.prototype.setValue = function(value) {

  // generate listNode if necessary
  // set always after the get, so if the listNode is null, it means
  // no node in XML.
  var listNode = this.getListNode_();
  if (!listNode) {
    listNode = Util.XML.createElement(this.listname_);
    this.xmlnode_.appendChild(listNode);
  }

  // remove the old listItems
  for (var i = 0; i < listNode.childNodes.length;) {
    listNode.removeChild(listNode.childNodes[i]);
  }

  // save listValue into listNode
  var me = this;
  _arr(value, function(val) {
    var listItem = Util.XML.createElement(me.itemname_);
    Util.array.applyToMultiple(me.itemattrnames_, val, function(attr, v) {
      listItem.setAttribute(attr, v);
    });

    listNode.appendChild(listItem);
  });
};

ListXmlAttr.prototype.removeValue = function() {
  var node = this.getListNode_();
  if (node)
    this.xmlnode_.removeChild(node);
};

ListXmlAttr.prototype.getListNode_ = function() {
  return Util.checkUniqueOrNullAndReturn(this.xmlnode_, this.listname_);
};
