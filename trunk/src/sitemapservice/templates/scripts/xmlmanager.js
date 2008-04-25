// Copyright 2008 Google Inc.
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
 * @fileoverview This file contains Class XmlManager that extends the XML node
 * functionality, specially for xpath.
 * 
 * Notes: not support multi-node select xpath, like 'xxx[*]'
 *
 * @author chaiying@google.com (Ying Chai)
 */

/**
 * @constructor
 * @param {Document|Element} dom  The XML document or node
 */
function XmlManager(dom) {
  this.xmldom_ = dom;
}
/**
 * Gets the XML document/node.
 * @return {Document|Element} The XML document/node
 */
XmlManager.prototype.dom = function() {
  return this.xmldom_;
};

/**
 * Gets the XML attribute value.
 * @param {String} name  The attribute name
 */
XmlManager.prototype.getAttribute = function(name) {
  return this.xmldom_.getAttribute(name);
};

/**
 * Gets the XML attribute value by xpath.
 * @param {String} xpath  The attribute xpath
 */
XmlManager.prototype.getAttributeByXpath = function(xpath) {
  return XmlManager.getAttributeByXpath_(xpath, this.xmldom_);
};

/**
 * Checks if the XML node has the attribute.
 * @param {String} xpath  The attribute xpath
 */
XmlManager.prototype.hasAttributeByXpath = function(xpath) {
  return XmlManager.hasAttributeByXpath_(xpath, this.xmldom_);
};

/**
 * Sets the XML attribute value by xpath.
 * @param {String} xpath  The attribute xpath
 * @param {String} value  The attribute value
 */
XmlManager.prototype.setAttributeByXpath = function(xpath, value) {
  return XmlManager.setAttributeByXpath_(xpath, value, this.xmldom_);
};

/**
 * Gets the XML node by xpath.
 * @param {String} xpath  The node xpath
 */
XmlManager.prototype.getNodeByXpath = function(xpath) {
  var node = XmlManager.getNodeByXpath_(xpath, this.xmldom_);
  return node ? new XmlManager(node) : null;
};

/**
 * Gets the XML nodes by xpath.
 * @param {String} xpath  The xpath of multiple nodes
 */
XmlManager.prototype.getNodesByXpath = function(xpath) {
  var nodes = XmlManager.getNodesByXpath_(xpath, this.xmldom_);
  return !nodes ? null : Util.array.apply(nodes, function(node) {
    return new XmlManager(node);
  });
};

/**
 * Removes the XML node by xpath.
 * @param {String} xpath  The node xpath
 */
XmlManager.prototype.removeNodeByXpath = function(xpath) {
  XmlManager.removeNodeByXpath_(xpath, this.xmldom_);
};

/**
 * Removes the XML attribute by xpath.
 * @param {String} xpath  The attribute xpath
 */
XmlManager.prototype.removeAttributeByXpath = function(xpath) {
  XmlManager.removeAttributeByXpath_(xpath, this.xmldom_);
};

/**
 * Creates XML element
 * @param {String} tag  The element tag
 */
XmlManager.createElement = function(tag) {
  if (XmlManager.dom == null)
    XmlManager.dom = XmlManager.createNewXmlDocument();
  return XmlManager.dom.createElement(tag);
};

/**
 * Creates text XML element
 * @param {String} text  The element text
 */
XmlManager.createTextNode = function(text) {
  if (XmlManager.dom == null)
    XmlManager.dom = XmlManager.createNewXmlDocument();
  return XmlManager.dom.createTextNode(text);
};

/**
 * Deserializes an XML DOM from string.
 * @param {Object} str  The XML string
 */
XmlManager.prototype.deserialize = function(str) {
  this.xmldom_ = XmlManager.deserializeFromString_(str);
};

/**
 * Generates the newline separator element in XML.
 * These separators help formating the serialized XML string. *
 * @param {String} indent  The indent level for the element, represented by
 *     whitespace
 * @return {Element} the newline text element.
 */
XmlManager.prototype.generateSeparator = function(indent) {
  var separator;
  if (Util.DOM.isTextNode(this.xmldom_.previousSibling)){
    separator = XmlManager.createTextNode(
        this.xmldom_.previousSibling.nodeValue + indent);
  } else {
    separator = XmlManager.createTextNode('\n');
  }
  return separator;
};

/**
 * Appends a sub node, and seperate sub nodes by separator.
 * @param {Element} node  The sub node to be append
 * @param {Element} separator  The seperator node
 */
XmlManager.prototype.appendNodeWithSeparator = function(node, separator) {
  // first list
  if (this.xmldom_.childNodes.length == 0) {
    this.xmldom_.appendChild(separator.cloneNode(true));
  }
  this.xmldom_.appendChild(node);
  this.xmldom_.appendChild(separator.cloneNode(true));
};

/**
 * Converts the managed XML document to string.
 * @return {String} the string of the XML content
 */
XmlManager.prototype.serialize = function() {
  return XmlManager.serializeToString_(this.xmldom_);
};

/**
 * Gets the attribute identifier in the xpath.
 * @param {String} xpath  The xpath of the attribute
 * @return {String} The attribute name
 */
XmlManager.getAttrNameFromXpath = function(xpath) {
  if (xpath.search(/@/) != -1) {
    var tagTree = xpath.split(/[@]/);
    // If xpath='@abc', in firefox, the tagTree.length = 2 ['','abc'];
    // In IE, the length is 1 ['abc']
    return tagTree[tagTree.length - 1];
  } else {
    // for xpath='abc', we think that's a attribute identifier without
    // sub-node definition
    return xpath;
  }
};

/**
 * Gets the xpath that without attribute identifier.
 * @param {String} xpath  The xpath of the attribute
 * @return {String} The xpath without the attribute name
 */
XmlManager.removeAttrNameFromXpath = function(xpath) {
  if (xpath.search(/@/) != -1) {
    var tagTree = xpath.split(/[@]/);
    // If xpath='@abc', in firefox, the tagTree.length = 2 ['','abc'];
    // In IE, the length is 1 ['abc']
    switch (tagTree.length) {
      case 0: // '@' in IE
      case 1: // '@abc' in IE
        return '';
      case 2: // normal issue
        return tagTree[0];
      default: // error, more than one '@'
        return null;
    }
  } else {
    // for xpath='abc', we think that's a attribute identifier without
    // sub-node definition
    return null;
  }
};

/**
 * Gets the node that has the attribute and the attribute name by the xpath of
 * the attribute.
 * @param {String} xpath  The xpath of the attribute
 * @param {ELement} context  The node that has the attribute
 * @param {Boolean} mustExist  If true, throw exception when node does not
 *   exist, else, return null when node does not exist
 * @private
 */
XmlManager.getContextAndAttrName_ = function(xpath, context, mustExist) {
  if (xpath.search(/@/) != -1) {
    var tagTree = xpath.split(/[\/@]/);
    for (var i = 0; i < tagTree.length - 1; i++) {
      if (tagTree[i] == '')
        continue; // for firefox

      if (mustExist) {
        context = Util.checkUniqueAndReturn(context, tagTree[i]);
      } else {
        var nodes = context.getElementsByTagName(tagTree[i]);
        if (nodes.length != 1)
          return null;
        context = nodes[0];
      }
    }
    return {node: context, attrname: tagTree[i]};
  } else {
    return {node: context, attrname: xpath};
  }
};

/**
 * Gets value of the attribute that the xpath points to.
 * @param {String} xpath  The xpath of the attribute
 * @param {Node} context  The root node of the xpath
 * @return {String?} The attribute value
 * @private
 */
XmlManager.getAttributeByXpath_ = function(xpath, context) {
  var res = XmlManager.getContextAndAttrName_(xpath, context, true);

  if (Util.DOM.hasAttribute(res.node, res.attrname))
    return res.node.getAttribute(res.attrname);
  else
    return null;
};

/**
 * Checks if the attribute that the xpath points to exists.
 * @param {String} xpath  The xpath of the attribute
 * @param {Node} context  The root node of the xpath
 * @return {Boolean} If the attribute exists
 * @private
 */
XmlManager.hasAttributeByXpath_ = function(xpath, context) {
  var res = XmlManager.getContextAndAttrName_(xpath, context, false);
  if (res == null)
    return false;

  return Util.DOM.hasAttribute(res.node, res.attrname);
};

/**
 * Sets XMl attribute value
 * @param {String} xpath  XPath of the attribute, or attribute name of root
 * node.
 * @param {String} value  The attribute value
 * @param {Node} context  The root node of the xpath
 * @private
 */
XmlManager.setAttributeByXpath_ = function(xpath, value, context) {
  var res = XmlManager.getContextAndAttrName_(xpath, context, true);
  res.node.setAttribute(res.attrname, value);
};

/**
 * Removes the XML attribute by xpath.
 * @param {String} xpath  The attribute xpath
 * @param {Node} context  The root node of the xpath
 * @private
 */
XmlManager.removeAttributeByXpath_ = function(xpath, context) {
  var res = XmlManager.getContextAndAttrName_(xpath, context, false);
  res.node.removeAttribute(res.attrname);
};

/**
 * Gets the XML node by xpath.
 * Notes:
 * Not support multi-node for the xpath, if so, using getNodesByXpath_ instead;
 * Not support multi-node select xpath like 'xxx[*]', if so, using 
 * getNodesByXpathByIndex_.
 * @param {String} xpath  The xpath of the node
 * @param {Node} context  The root node of the xpath
 * @return {Node?} The node that the xpath points to
 * @private
 */
XmlManager.getNodeByXpath_ = function(xpath, context) {
  /**
   * If xpath='/', tagTree.length is 0 for IE and 2 for FF.
   * This function will throw error in the case.
   *
   * If xpath='abc', tagTree.length is 1 for both IE and FF.
   * This function will return 'abc' node of the context in the case.
   *
   * If xpath='/abc', tagTree.length is 1 for IE and 2 for FF.
   * This function will return 'abc' node of the context in the case.
   *
   * If xpath='a/b', tagTree.length is 2 for both IE and FF.
   * This function will return 'a/b' node of the context in the cass.
   */
  if (xpath == '/')
    Util.console.error('invalid xpath');

  var tagTree = xpath.split(/[\/]/);
  for (var i = 0; i < tagTree.length; i++) {
    if (tagTree[i] == '')
      continue;

    context = Util.checkUniqueOrNullAndReturn(context, tagTree[i]);
    if (context == null)
      return null;
  }
  return context;
};

/**
 * Removes the XML node by xpath.
 * @param {String} xpath  The xpath of the node
 * @param {Node} context  The root node of the xpath
 * @private
 */
XmlManager.removeNodeByXpath_ = function(xpath, context) {
  var node = XmlManager.getNodeByXpath_(xpath, context);
  if (node != null)
    node.parentNode.removeChild(node);
};

/**
 * Gets the XML nodes by xpath.
 * @param {String} xpath  The xpath of multiple nodes
 * @param {Node} context  The root node of the xpath
 * @return {Array.<Node>?} The nodes that the xpath points to
 * @private
 */
XmlManager.getNodesByXpath_ = function(xpath, context) {
  var tagTree = xpath.split(/[\/]/);
  for (var i = 0; i < tagTree.length - 1; i++) {
    if (tagTree[i] == '')
      continue;

    context = Util.checkUniqueOrNullAndReturn(context, tagTree[i]);
    if (context == null)
      return null;
  }

  var nodes = context.getElementsByTagName(tagTree[tagTree.length - 1]);
  if (nodes.length == 0)
    return null;
  return nodes;
};

/**
 * Converts XML document to string.
 * @param {Document} xmldom  The xml document object
 * @return {String} the XML string of xmldom
 * @private
 */
XmlManager.serializeToString_ = function(xmldom) {
  if (typeof XMLSerializer != 'undefined') {
    try {
      return (new XMLSerializer()).serializeToString(xmldom);
    } catch (e) {
      Util.console.error('Failed to serialize the xmldom');
      return '';
    }
  } else if (xmldom.xml) {
    return xmldom.xml;
  } else {
    Util.console.error(
        'XML.serialize is not supported or we cannot serialize the xmldom');
    return '';
  }
};

/**
 * Creates a XML document from the text string.
 * @param {String} text  XML content string
 * @return {Document} The XML document that is created.
 * @private
 */
XmlManager.deserializeFromString_ = function(text) {
  // load the string to a xml dom object.
  if (window.ActiveXObject) {
    // code for IE
    var doc = new ActiveXObject('Microsoft.XMLDOM');
    doc.async = 'false';
    doc.loadXML(text);
  } else {
    // code for Mozilla, Firefox, Opera, etc.
    var parser = new DOMParser();
    var doc = parser.parseFromString(text, 'text/xml');
  }
  return doc;
};

/**
 * Creates an empty XML document.
 * @param {String} rootTagName  The tag of the root node of the XML document
 */
XmlManager.createNewXmlDocument = function(rootTagName) {
  if (rootTagName == null)
    rootTagName = '';

  var namespaceURL = '';

  if (document.implementation && document.implementation.createDocument) {
    // This is the W3C standard way to do it
    return document.implementation.createDocument(namespaceURL, rootTagName, 
                                                  null);
  } else { // This is the IE way to do it
    // Create an empty document as an ActiveX object
    // If there is no root element, this is all we have to do
    var doc = new ActiveXObject('MSXML2.DOMDocument');

    // If there is a root tag, initialize the document
    if (rootTagName) {
      // Look for a namespace prefix
      var prefix = '';
      var tagname = rootTagName;
      var p = rootTagName.indexOf(':');
      if (p != -1) {
        prefix = rootTagName.substring(0, p);
        tagname = rootTagName.substring(p + 1);
      }

      // If we have a namespace, we must have a namespace prefix
      // If we don't have a namespace, we discard any prefix
      if (namespaceURL) {
        if (!prefix)
          prefix = 'a0'; // What Firefox uses
      } else
        prefix = '';

      // Create the root element (with optional namespace) as a
      // string of text
      var text = '<' + (prefix ? (prefix + ':') : '') + tagname +
      (namespaceURL ? (' xmlns:' + prefix + '=\'' + namespaceURL + '\'') : '') +
      '/>';
      // And parse that text into the empty document
      doc.loadXML(text);
    }
    return doc;
  }
};
