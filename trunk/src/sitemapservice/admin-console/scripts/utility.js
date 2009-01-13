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
 * @fileoverview This class contains some utility functions.
 */

/**
 * The base namespace for this module.
 */
var Util = {};

/**
 * The namespace for msg/log output.
 */
Util.console = {};

/**
 * The namespace for Array functions extension.
 */
Util.array = {};

/**
 * The namespace for mapping Object functions extension.
 */
Util.map = {};

/**
 * The namespace for CSS class utilities.
 */
Util.CSS = {};

/**
 * The namespace for event management utilities.
 */
Util.event = {};

/**
 * The namespace for XML tree utilities.
 */
Util.XML = {};


////////////////////////////////////


/**
 * Creates XML element.
 * @param {String} tag  The element tag
 */
Util.XML.createElement = function(tag) {
  if (Util.XML.dom == null)
    Util.XML.dom = Util.XML.createNewXmlDocument();
  return Util.XML.dom.createElement(tag);
};

/**
 * Creates text XML element.
 * @param {String} text  The element text
 */
Util.XML.createTextNode = function(text) {
  if (Util.XML.dom == null)
    Util.XML.dom = Util.XML.createNewXmlDocument();
  return Util.XML.dom.createTextNode(text);
};
/**
 * Creates an empty XML document.
 * @param {String} rootTagName  The tag of the root node of the XML document
 * @supported IE and Firefox
 */
Util.XML.createNewXmlDocument = function(rootTagName) {
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

/////////////////////////// report functions ////////////////////////////
/**
 * Logs to the debug window.
 * @param {String} msg  The debug message
 */
Util.console.log = function(msg) {
  if (console) { // firebug console
    console.log(msg);
    return;
  }
  
  function writeln(str) {
    box.appendChild(
        document.createTextNode(str));
    box.appendChild(
        document.createElement('br'));
  }
  
  var box = Util.console.log.box;
  if (!box) {
    box = document.getElementById('debug-box');
    if (!box) return;
    Util.console.log.box = box;
    writeln('start debug...');
    _show(box);
  }
  writeln(msg);
};

/**
 * Reports to user and throws exception.
 * @param {String} msg  The error message
 */
Util.console.error = function(msg) {
  alert('Error: ' + msg);debugger;
  throw new Error(msg);
};

/////////////////////// assert function ///////////////////////////
/**
 * Asserts the value is not null and equal to Boolean value 'true', empty
 * string will be treated as true.
 * Exception will be thrown if the value is null or undefined.
 * @param {Object?} val  The value to be asserted
 * @param {String} opt_msg  The msg to be output if the assert is failed
 */
Util.assert = function(val, opt_msg) {
  var defMsg = 'assert failed!';
  if (val === null || val === false || val === undefined) {
    _err(opt_msg ? opt_msg : defMsg);
  }
};

//////////////////////////////////////////

Util.spaceUnits = [
  GSGLang.texts.byteUnit,
  GSGLang.texts.kiloByteUnit,
  GSGLang.texts.megaByteUnit,
  GSGLang.texts.gigaByteUnit,
  GSGLang.texts.teraByteUnit
];

/**
 * Return well formatted string according to the value.
 * @param {number|string} value
 */
Util.getSpaceString = function(value) {
  if (typeof value == 'string') value = parseInt(value);
  var level = 0;
  while (value > KILO_UNIT_NUM) {
    value /= KILO_UNIT_NUM;
    level++;
  }
  if (level > 0) {
    value = value.toFixed(2);
  }
  return value + ' ' + Util.spaceUnits[level];
};

/////////////////////// check function ///////////////////////////

/**
 * Checks if the element with this id exists in current document, and return
 * the element. Exception will be thrown if the check fails.
 * @param {String} id  The element's id
 * @return {Element} The element that is found
 */
Util.checkElemExistAndReturn = function(id) {
  var elem = document.getElementById(id);
  if (elem == null || id == 'save-web') {
    Util.assert(false, 'lack html element with id: ' + id);
  }
  return elem;
};

/**
 * Checks that the node has exact one element that the tag name is equal to
 * the 'tagName'. Exception will be thrown if the check fails. It will return
 * the element that is found,.
 *
 * @param {Node} node  The node to be checked
 * @param {String} tagName  The tag name for the element
 * @return {Element} The element that is found
 */
Util.checkUniqueAndReturn = function(node, tagName) {
  var subnode = node.getElementsByTagName(tagName);
  var l = subnode.length;
  if (l > 1)
    Util.assert(false, 'element (' + tagName + ') is not unique');
  else if (l == 0)
    Util.assert(false, 'element (' + tagName + ') does not exist');
  return subnode[0];
};

/**
 * Checks that the node has at most one element that the tag name is equal to
 * the 'tagName'. Exception will be thrown if the check fails. It will return
 * the element that is found, or null if not found.
 *
 * @param {Node} node  The node to be checked
 * @param {String} tagName  The tag name for the element
 * @return {Element?} The element that is found, or null if not found
 */
Util.checkUniqueOrNullAndReturn = function(node, tagName) {
  var elements = node.getElementsByTagName(tagName);
  var l = elements.length;
  if (l > 1)
    Util.assert(false, 'element (' + tagName + ') is not unique');
  else if (l == 0)
    return null;
  return elements[0];
};

///////////////////////// array/map functions ////////////////

Util.array.equals = function(arr1, arr2) {
  if (typeof arr1 != typeof arr2) {
    return false;
  }
  if (typeof arr1 != 'object') {
    return arr1 === arr2;
  }
  if (arr1 instanceof Array && arr2 instanceof Array) {
    var len1 = arr1.length;
    var len2 = arr2.length;
    if (len1 != len2)
      return false;

    for (var i = arr1.length - 1; i >= 0; i--) {
      var val1 = arr1[i];
      var val2 = arr2[i];
      if (!Util.array.equals(val1, val2))
        return false;
    }
  } else if (typeof arr1.equals == 'function') {
    return arr1.equals(arr2);
  } else {
    return arr1 === arr2;
  }
  return true;
};

/**
 * Checks if the array contains the value, which means one of the array item
 * is equal to the value.
 * @param {Array} array  The array to be checked
 * @param {Object} val  The value for check
 * @return {Boolean} True if the array contains the value
 */
Util.array.contains = function(array, val) {
  var l = array.length;
  for (var i = 0; i < l; i++) {
    if (array[i] == val) {
      return true;
    }
  }
  return false;
};

/**
 * Calls the 'func' function on each item in 'array', the item will be passed to
 * the applied function as the only parameter. It returns
 * the result array that contains the return values of the 'func' function.
 *
 * We assume that the applied function cannot access the array directly, and
 * will never change the array's length.
 *
 * @param {Array} array  The array to be processed
 * @param {Function} func  The function that apply to the array, it takes the
 *     array item as the parameter. The return value will be ignored
 * @return {Array} The array of the results that the function return for each
 *     item.
 */
Util.array.apply = function(array, func) {
  var rets = [];
  var l = array.length;
  for (var i = 0; i < l; i++)
    rets.push(func(array[i]));
  return rets;
};

Util.map.apply = function(mapObj, func) {
  var rets = [];
  for (var prop in mapObj)
    rets.push(func(mapObj[prop]));
  return rets;
};

/**
 * Applies the function to array members, will stop applying to the remaining
 * members if the function return false to current member.
 * @param {Array} array  The array to be applied
 * @param {Function} func  The function that applies to the array
 */
Util.array.applyWithBreak = function(array, func) {
  var l = array.length;
  for (var i = 0; i < l; i++)
    if (!func(array[i]))
      break;
};

Util.array.checkIfAny = function(array, func) {
  var l = array.length;
  for (var i = 0; i < l; i++)
    if (func(array[i]))
      return true;
  return false;
};

Util.map.checkIfAny = function(mapObj, func) {
  for (var prop in mapObj)
    if (func(mapObj[prop]))
      return true;
  return false;
};

Util.array.find = function(array, func) {
  var l = array.length;
  for (var i = 0; i < l; i++)
    if (func(array[i]))
      return array[i];
  return null;
};

Util.map.find = function(mapObj, func) {
  for (var prop in mapObj)
    if (func(mapObj[prop]))
      return mapObj[prop];
  return null;
};

/**
 * Applies the function to each item of these two arrays, if these two arrays
 * are not equal in size, the longer one will be trunc.
 *
 * @param {Array} arr1  The first array to be processed
 * @param {Array} arr2  The second array to be processed
 * @param {Function} func  The function that apply to these two arrays, it takes
 *      two parameters, the first one is an item of the first array, the second
 *      one is the item of the second array that has the same index of the first
 *      one. The return value will be ignored
 */
Util.array.applyToMultiple = function(arr1, arr2, func) {
  var l1 = arr1.length, l2 = arr2.length;
  for (var i = 0; i < l1 && i < l2; i++)
    func(arr1[i], arr2[i]);
};
/**
 * Removes the item from the array
 * @param {Array} array  The array
 * @param {Object} item  The item in the array that need to be removed
 * @param {Boolean} isUnique  If true, only remove the first in the 'array'
 * which is equal to the 'item', else, remove all that match.
 */
Util.array.remove = function(array, item, isUnique) {
  for (var i = array.length - 1; i >= 0; i--) {
    if (array[i] == item) {
      array.splice(i, 1);
      if (isUnique)
        break;
    }
  }
};
/**
 * Passes each element of the array a to the specified predicate function.
 * Return an array that holds the elements for which the predicate returns true.
 *
 * @param {Array} a  The array that need filter
 * @param {Function} predicate  Function that take the array item as the
 *     parameter and return Boolean result
 * @return {Array} The array that has been filtered
 * @private
 */
Util.array.filter_ = function(a, predicate) {
  var results = [];
  var l = a.length;
  for (var i = 0; i < l; i++) {
    var element = a[i];
    if (predicate(element))
      results.push(element);
  }
  return results;
};

//////////////////////// CSS class functions //////////////////
/**
 * Adds a CSS class to the element
 * @param {Element} elem  The HTML element
 * @param {String} name  The CSS class name
 */
Util.CSS.addClass = function(elem, name) {
  if (!elem.className) { // first set
    elem.className = name;
  } else if (Util.CSS.isClass(elem, name)) { // avoid add multiple times
    return;
  } else {
    var newClassName = elem.className;
    newClassName += ' ';
    newClassName += name;
    elem.className = newClassName;
  }
};
/**
 * Checks if the element has the CSS class.
 * @param {Element} elem  The HTML element
 * @param {String} name  The CSS class name
 * @return {Boolean} If the element has the CSS class
 */
Util.CSS.isClass = function(elem, name) {
  if (!elem.className)
    return false;

  var names = elem.className.split(/ /);
  for (var i = 0; i < names.length; i++) {
    if (names[i] == name)
      return true;
  }
  return false;
};
/**
 * Removes a CSS class from the element
 * @param {Element} elem  The HTML element
 * @param {String} name  The CSS class name
 */
Util.CSS.removeClass = function(elem, name) {
  if (!elem.className)
    return;

  var names = elem.className.split(/ /);
  Util.array.remove(names, name, false);
  elem.className = names.join(' ');
};
/**
 * Changes the element from one CSS class to another CSS class
 * @param {Element} elem  The HTML element
 * @param {String} oldname  The CSS class name to be replaced
 * @param {String} newname  The CSS class name to be applied to the element
 */
Util.CSS.changeClass = function(elem, oldname, newname) {
  Util.CSS.removeClass(elem, oldname);
  Util.CSS.addClass(elem, newname);
};

/**
 * Hides the element by using CSS.
 * @param {Element} elem  The HTML element
 */
Util.CSS.hideElement = function(elem) {
  Util.CSS.addClass(elem, HIDDEN_CSS);
};

/**
 * Shows the element by using CSS.
 * @param {Element} elem  The HTML element
 */
Util.CSS.showElement = function(elem) {
  Util.CSS.removeClass(elem, HIDDEN_CSS);
};


///////////////////////////// Event functions //////////////////////////////////
/**
 * Records the handlers that bind to an event.
 */
Util.event.handlers_ = [];

/**
 * Adds event handler to the element.
 * First add, first serve.
 * @param {Element} target  The element that the event handler attach to
 * @param {String} eventName  The event name
 * @param {Function} func  The event hanlder
 */
Util.event.add = function(target, eventName, func) {
  var handlerName = Util.event.check_(eventName);
  var handlerTable = handlerName + 'table';
  if (typeof target[handlerName] != 'function' ||
      target[handlerTable] == null) {
    target[handlerTable] = [func];
    target[handlerName] = function(e) {
      _arr(target[handlerTable], function(f) {
        f(e, target);
      });
      if (target.tagName && target.tagName.toLowerCase() == 'a')
        return false;// stop link navigate off
    };
  } else {
    target[handlerTable].push(func);
  }
  Util.event.handlers_.push({elem: target, name: handlerName});
};

Util.event.clear = function(target, eventName) {
  var handlerName = Util.event.check_(eventName);
  var handlerTable = handlerName + 'table';
  target[handlerName] = null;
  target[handlerTable] = null;
};

/**
 * Flush events handlers attached to html DOM objects.
 * This method should be registered to window.unload method to avoid part of
 * memory leak in IE6.
 * 1. Only events included in Util.event.EVENTS can be removed.
 * 2. The event should be registered by dom_el["event_name"] = event_function.
 *    Events registered by other ways, like attachEvent, can't be removed here.
 */
Util.event.flush = function() {
  while (true) {
    var h = Util.event.handlers_.pop();
    if (!h) {
      break;
    }
    h.elem[h.name] = null;
    h.elem[h.name + 'table'] = null;
  }
};

/**
 * The events that these utilities in the namespace supported.
 */
Util.event.EVENTS = ['mouseover', 'mouseout', 'change', 'keydown', 'keyup',
                     'load', 'click', 'unload', 'mousemove', 'blur'];
/**
 * Checks if the event is supported.
 * @param {String} eventName  The event name
 * @return {String} The event name that has 'on' prefix
 * @private
 */
Util.event.check_ = function(eventName) {
  var name = eventName.toLowerCase();
  if (!Util.array.contains(Util.event.EVENTS, name))
    _err('invalid event - \'' + name + '\'');
  return 'on' + name;
};

/**
 * Triggers event.
 * @param {String} eventname  The event name
 * @param {Element} target  The target element
 * @supported IE6 and Firefox2
 */
Util.event.send = function(eventname, target) {
  if (typeof target == 'string')
    target = _gel(target);

  if (document.createEvent) {
    var e = document.createEvent('Events');
    e.initEvent(eventname, true, false);
  } else if (document.createEventObject) {
    var e = document.createEventObject();
  } else {
    _err('The browser do not support event create!');
  }

  if (target.dispatchEvent)
    target.dispatchEvent(e);
  else if (target.fireEvent)
    target.fireEvent('on' + eventname, e);
};

/**
 * Adds onReturnKey event to the element.
 * @param {Element} target  The target element
 * @param {Function} func  The event handler
 */
Util.event.addEnterKey = function(target, func) {
  // Using 'keyup' will cause trouble when user use keyboard
  // to confirm the alert popup window.
  _event(target, 'keydown', function(event) {
    // Get the event object and character code in a portable way
    var e = event || window.event;         // Key event object
    var code = e.charCode || e.keyCode;    // What key was pressed
    if (code == 13) {
      func();
    }
  });
};

Util.event.clearEnterKey = function(target) {
  // Using 'keyup' will cause trouble when user use keyboard
  // to confirm the alert popup window.
  Util.event.clear(target, 'keydown');
};

//////////////////////// Performance functions //////////////////
/**
 * The performance monitor class.
 * @constructor
 * var perf = new Perf(); // start perf, record the start time.
 * perf.check(msg); // record the current point time.
 * perf.checkAndDebug(max,msg);// record the current point time, if the last
 *                             // time duration is longer than max, trigger
 *                             // debugger.
 * perf.report(); // show the perf data
 * @param {string} opt_name  The name of the perf instance
 */
function Perf(opt_name) {
  this.name_ = opt_name ? opt_name : '';
  this.times = [];
  this.check('start point');
}
/**
 * The enable flag of performance monitor.
 * @type {Boolean}
 * @private
 */
Perf.enable_ = false;

/**
 * The global DB for named perf instances, which can be shared among functions.
 */
Perf.perfs = {};

/**
 * Gets a perf instance in global DB by name.
 * @param {String} name  The name of the perf
 * @return {Perf} The perf instance
 */
Perf.getPerf = function(name) {
  if (!Perf.perfs[name]) {
    Perf.perfs[name] = new Perf(name);
  }
  return Perf.perfs[name];
};
/**
 * The internal class used by perf to record the check point information
 * @param {String} id  The identifier of the check point
 * @param {Number} time  The time of the check point, in milliseconds
 * @private
 * @constructor
 */
Perf.CheckPoint_ = function(id, time) {
  this.id = id;
  this.time = time;
};

/**
 * Records the current point time
 * @param {String} msg  The message that discribes the check point
 */
Perf.prototype.check = function(msg) {
  if (Perf.enable_)
    this.times.push(new Perf.CheckPoint_(msg, (new Date()).getTime()));
};
/**
 * Reports to user the perf summary.
 */
Perf.prototype.report = function() {
  if (Perf.enable_)
    alert(this.toString());
};
/**
 * Gets the perf summary
 * @return {String} The perf summary
 */
Perf.prototype.toString = function() {
  var interval = [];
  var len = this.times.length;
  var times = this.times;
  for (var i = 1; i < len; i++) {
    var prev = times[i - 1];
    var curr = times[i];
    interval.push(curr.id + ': ' + (curr.time - prev.time).toString());
  }
  if (len > 2)
    interval.push('Total: ' +
        (times[len - 1].time - times[0].time).toString());

  return this.name_ + ':\n' + interval.join('\n');
};
/**
 * Records the current point time, checks its duration, debug if it costs too
 * much time.
 * @param {Number} max  The max duration, in milliseconds
 * @param {String} opt_msg  The optional message for the check point
 */
Perf.prototype.checkAndDebug = function(max, opt_msg) {
  if (!Perf.enable_)
    return;
  var time = (new Date()).getTime();
  var msg = opt_msg ? opt_msg : '';
  this.times.push(new Perf.CheckPoint_(msg, time));
  var prev = this.times[this.times.length - 1].time;
  if (time - prev > max) debugger;
};

/**
 * Adds a perf instance to global DB.
 * @param {Perf} perf  The perf instance
 */
Perf.addPerf = function(perf) {
  if (!Perf.perfs)
    Perf.perfs = [];
  Perf.perfs.push(perf);
};
/**
 * Reports all the perfs' summaries in global DB.
 * @param {Perf} perf  The perf instance
 */
Perf.report = function(perf) {
  if (!Perf.enable_)
    return;
  var res = [];
  _arr(Perf.perfs, function(perf) {
    res.push(perf.toString());
  });
  alert(res.join('\n'));
};

///////////// Cookie ////////////////////
/**
 * This is the Cookie( ) constructor function.
 *
 * This constructor looks for a cookie with the specified name for the
 * current document. If one exists, it parses its value into a set of
 * name/value pairs and stores those values as properties of the newly created
 * object.
 *
 * To store new data in the cookie, simply set properties of the Cookie
 * object. Avoid properties named "store" and "remove", since these are
 * reserved as method names.
 *
 * To save cookie data in the web browser's local store, call store( ).
 * To remove cookie data from the browser's store, call remove( ).
 *
 * The static method Cookie.enabled( ) returns true if cookies are
 * enabled and returns false otherwise.
 * @param {String} name  The cookie name
 * @constructor
 */
function Cookie(name) {
  this.$name = name;  // Remember the name of this cookie

  // First, get a list of all cookies that pertain to this document.
  // We do this by reading the magic Document.cookie property.
  // If there are no cookies, we don't have anything to do.
  var allcookies = document.cookie;
  if (allcookies == '') return;

  // Break the string of all cookies into individual cookie strings
  // Then loop through the cookie strings, looking for our name
  var cookies = allcookies.split(';');
  var cookie = null;
  for (var i = 0; i < cookies.length; i++) {
    // Does this cookie string begin with the name we want?
    if (cookies[i].substring(0, name.length + 1) == (name + '=')) {
      cookie = cookies[i];
      break;
    }
  }

  // If we didn't find a matching cookie, quit now
  if (cookie == null) return;

  // The cookie value is the part after the equals sign
  var cookieval = cookie.substring(name.length + 1);

  // Now that we've extracted the value of the named cookie, we
  // must break that value down into individual state variable
  // names and values. The name/value pairs are separated from each
  // other by ampersands, and the individual names and values are
  // separated from each other by colons. We use the split( ) method
  // to parse everything.
  var a = cookieval.split('&'); // Break it into an array of name/value pairs
  for (var i = 0; i < a.length; i++)  // Break each pair into an array
    a[i] = a[i].split(':');

  // Now that we've parsed the cookie value, set all the names and values
  // as properties of this Cookie object. Note that we decode
  // the property value because the store( ) method encodes it.
  for (var i = 0; i < a.length; i++) {
    this[a[i][0]] = decodeURIComponent(a[i][1]);
  }
}

/**
 * This function is the store( ) method of the Cookie object. *
 * @param {Number} daysToLive  The lifetime of the cookie, in days. If you set
 *     this to zero, the cookie will be deleted. If you set it to null, or
 *     omit this argument, the cookie will be a session cookie and will
 *     not be retained when the browser exits. This argument is used to
 *     set the max-age attribute of the cookie.
 * @param {String} path  The value of the path attribute of the cookie
 * @param {String} domain  The value of the domain attribute of the cookie
 * @param {Boolean} secure  If true, the secure attribute of the cookie will be
 *     set
 */
Cookie.prototype.store = function(daysToLive, path, domain, secure) {
  // First, loop through the properties of the Cookie object and
  // put together the value of the cookie. Since cookies use the
  // equals sign and semicolons as separators, we'll use colons
  // and ampersands for the individual state variables we store
  // within a single cookie value. Note that we encode the value
  // of each property in case it contains punctuation or other
  // illegal characters.
  var cookieval = '';
  for (var prop in this) {
    // Ignore properties with names that begin with '$' and also methods
    if ((prop.charAt(0) == '$') || ((typeof this[prop]) == 'function'))
      continue;
    if (cookieval != '') cookieval += '&';
    cookieval += prop + ':' + encodeURIComponent(this[prop]);
  }

  // Now that we have the value of the cookie, put together the
  // complete cookie string, which includes the name and the various
  // attributes specified when the Cookie object was created
  var cookie = this.$name + '=' + cookieval;
  if (daysToLive || daysToLive == 0) {
    cookie += '; max-age=' + (daysToLive * 24 * 60 * 60);
  }

  if (path) cookie += '; path=' + path;
  if (domain) cookie += '; domain=' + domain;
  if (secure) cookie += '; secure';

  // Now store the cookie by setting the magic Document.cookie property
  document.cookie = cookie;
};

/**
 * This function is the remove( ) method of the Cookie object; it deletes the
 * properties of the object and removes the cookie from the browser's
 * local store.
 *
 * The arguments to this function are all optional, but to remove a cookie
 * you must pass the same values you passed to store( ).
 * @param {String} path  The value of the path attribute of the cookie
 * @param {String} domain  The value of the domain attribute of the cookie
 * @param {Boolean} secure  If true, the secure attribute of the cookie will be
 *     set
 */
Cookie.prototype.remove = function(path, domain, secure) {
  // Delete the properties of the cookie
  for (var prop in this) {
    if (prop.charAt(0) != '$' && typeof this[prop] != 'function')
      delete this[prop];
  }

  // Then, store the cookie with a lifetime of 0
  this.store(0, path, domain, secure);
};

/**
 * This static method attempts to determine whether cookies are enabled.
 * @return {Boolean} True if they appear to be enabled and false otherwise.
 *   A return value of true does not guarantee that cookies actually persist.
 *   Nonpersistent session cookies may still work even if this method
 *   returns false.
 */
Cookie.enabled = function() {
  // Use navigator.cookieEnabled if this browser defines it
  if (navigator.cookieEnabled != undefined) return navigator.cookieEnabled;

  // If we've already cached a value, use that value
  if (Cookie.enabled.cache != undefined) return Cookie.enabled.cache;

  // Otherwise, create a test cookie with a lifetime
  document.cookie = 'testcookie=test; max-age=10000';  // Set cookie

  // Now see if that cookie was saved
  var cookies = document.cookie;
  if (cookies.indexOf('testcookie=test') == -1) {
    // The cookie was not saved
    Cookie.enabled.cache = false;
    return false;
  }
  else {
    // Cookie was saved, so we've got to delete it before returning
    document.cookie = 'testcookie=test; max-age=0';  // Delete cookie
    Cookie.enabled.cache = true;
    return true;
  }
};

///////////////// Build-in object extension ///////////////////
/**
 * Extends the Function object for object inheritance.
 * Makes one class inherit from another class
 * @param {Function} parent  The ancestor class to be inherited
 * @return {Function} The successor class
 */
Function.prototype.inheritsFrom = function(parent) {
  this.prototype = new parent('inheritsFrom');
  this.prototype.constructor = this;
  this.prototype.parent = parent.prototype;
  return this;
};
Function.prototype.borrowFrom = function(target) {
  var f = target.prototype;
  var t = this.prototype;
  for (var m in f) {
    if (typeof f[m] == 'function') {
      t[m] = f[m];
    }
  }
};

//////////////////////////////////////////////////////
/**
 * Some short alias and wrap functions.
 */
function _gelt(tag) {
  return document.getElementsByTagName(tag);
}

var _arr = Util.array.apply;
var _map = Util.map.apply;
var _event = Util.event.add;
var _show = Util.CSS.showElement;
var _hide = Util.CSS.hideElement;
var _err = Util.console.error;
var _log = Util.console.log;

function _gel(id) {
  if (!_gel.form) _gel.form = document['gsg-form'];
  var el = _gel.form[id];
  if (el) return el;
  return Util.checkElemExistAndReturn(id);
}
