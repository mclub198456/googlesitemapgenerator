// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview Class 'XmlAttr' is for getting/setting xml value.
 *
 * @author chaiying@google.com
 */
//////////////////////////////////XmlAttr//////////////////////////////
/**
 * @constructor
 * @param {Object} name
 */
function XmlAttr(name) {
  this.attrname_ = name;
  // hack code for sitemap@enable
  if (this.attrname_.indexOf('@enabled') != -1) {
    function tn(id) {
      switch (id) {
        case 'web':
          return 'WebSitemapSetting';
        case 'news':
          return 'NewsSitemapSetting';
        case 'mobile':
          return 'MobileSitemapSetting';
        case 'codesearch':
          return 'CodeSearchSitemapSetting';
        case 'blogsearch':
          return 'BlogSearchPingSetting';
      }
    }
    var strs = this.attrname_.split('@');
    this.attrname_ = strs[1];
    this.subTagname_ = tn(strs[0]);
  }

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
  } else if (this.subTagname_) {
    this.xmlnode_ = this.xmlnode_.getElementsByTagName(this.subTagname_)[0];
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

    ListXmlAttr.appendNodeWithSeparator_(this.xmlnode_, listNode);
  }

  // remove the old listItems
  Util.DOM.removeAllChildren(listNode);

  // save listValue into listNode
  var me = this;
  _arr(value, function(val) {
    var listItem = Util.XML.createElement(me.itemname_);
    Util.array.applyToMultiple(me.itemattrnames_, val, function(attr, v) {
      listItem.setAttribute(attr, v);
    });

    ListXmlAttr.appendNodeWithSeparator_(listNode, listItem);
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

ListXmlAttr.appendNodeWithSeparator_ = function(node, subnode) {
  function genSep(node, indent) {
    var prevNode = node.previousSibling;
    if (Util.DOM.isTextNode(prevNode)){
      return Util.XML.createTextNode(prevNode.nodeValue + indent);
    } else {
      return Util.XML.createTextNode('\n'); // maybe return null is better.
    }
  }
  // first list
  if (node.childNodes.length == 0) {
    node.appendChild(genSep(node, XML_FILE_INDENT));
    node.appendChild(subnode);
    node.appendChild(genSep(node, ''));
  } else {
    node.insertBefore(genSep(node, XML_FILE_INDENT), node.lastChild);
    node.insertBefore(subnode, node.lastChild);
  }
};
