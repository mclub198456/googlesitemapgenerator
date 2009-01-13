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
 * @fileoverview file for Ajax utility functions.
 */

/////////////////////////////AjaxUtil_requester///////////////////////////

/**
 * The requester object organizes the parameters of the request to the server.
 * @constructor
 */
function AjaxUtil_requester() {
  /**
   * @type {String?} url the url of post target
   */
  this.url = null;
  /**
   * @type {String?} type the content-type of http request header
   */
  this.type = null;
  /**
   * @type {Object?} content the content to post
   */
  this.content = null;
  /**
   * @type {Number?}
   */
  this.timeout = null;
  /**
   * @type {Boolean?}
   */
  this.wait = null;
}

/////////////////////////////AjaxUtil_responser///////////////////////////

/**
 * The responser object deals with the response message from the server.
 * It also store the session information, such as the XMLHttpRequest object and
 * the AjaxUtil_requester object of this session.
 * @constructor
 * @param {AjaxUtil_requester} requester  The request object
 */
function AjaxUtil_responser(requester) {
  /**
   * Keeps the reference to the request parameters set.
   * @type {AjaxUtil_requester}
   */
  this.requester = requester;

  /**
   * This is a reference to the XMLHttpRequest object that does the
   * communication.
   *
   * It will be initialized in AjaxUtil.accessServer, and be used in
   * AjaxUtil.getResponseContent.
   *
   * @type {XMLHttpRequest?}
   */
  this.xhr = null;

  /**
   * Call back function when time for waiting server response is out of the
   * limit.
   * @type {Function?}
   */
  this.onTimeout = null;

  /**
   * @type {Function?}
   */
  this.onResponseSuccess = null;
  /**
   * @type {Function?}
   */
  this.onResponseFail = null;
  /**
   * @type {Function?}
   */
  this.onResponse = null;
  /**
   * @type {Function?}
   */
  this.onProgress = null;
}

/////////////////////////////AjaxUtil///////////////////////////

/**
 * Namespace for ajax functions.
 */
var AjaxUtil = {};

/**
 * Gets XMLHttpRequest object.
 * @return {XMLHttpRequest?} The XMLHttpRequest object.
 * @supported IE and Firefox
 */
AjaxUtil.getRequestObject = function() {
  var xhr = null;
  if (window.XMLHttpRequest) {
    xhr = new XMLHttpRequest();
  } else {
    if (window.ActiveXObject) {
      try {
        xhr = new ActiveXObject('Microsoft.XMLHTTP');
      } catch (e) {
      }
    }
  }
  if (!xhr) {
    _err('Fail to create an XMLHttpRequest');
  }
  return xhr;
};

/**
 * Retreives the xml file and parses it to DOM.
 * @param {String} filename  The xml file name
 * @return {Document} The XML document
 * @supported IE only
 */
AjaxUtil.loadXml = function(filename) {
  var xmlDoc = Util.XML.createNewXmlDocument();

  /**
   * @type {String}
   */
  xmlDoc.async = 'false';
  xmlDoc.load(filename);
  return xmlDoc;
};

/**
 * Parses response content and return suitable format.
 * @param {AjaxUtil_responser} responser  The response object
 * @return {Document|String|Boolean|Number|Object|null} The return value will be
 *     different according to the 'Content-Type' response header.
 */
AjaxUtil.getResponseContent = function(responser) {
  // even if responser.xhr.status != 200, server may also send some error msg.
  // so we have to parse the response content.
  if (!responser.xhr || responser.xhr.readyState != 4) {
    return null;
  }

  // Check the content type returned by the server
  switch (responser.xhr.getResponseHeader('Content-Type')) {
    case 'text/xml; charset=utf-8':
    case 'text/xml':
      // server may return empty string (text/html) if load xml file failed
      if (responser.xhr.responseText == '') {
        return null;
      }
      // If it is an XML document, use the parsed Document object.
      return responser.xhr.responseXML;

    case 'text/json':
    case 'text/javascript':
    case 'application/javascript':
    case 'application/x-javascript':
      // If the response is JavaScript code, or a JSON-encoded value,
      // call eval() on the text to 'parse' it to a JavaScript value.
      // Note: only do this if the JavaScript code is from a trusted server!
      return eval(responser.xhr.responseText);

    default:
      // Otherwise, treat the response as plain text and return as a string.
      return responser.xhr.responseText;
  }
};

/**
 * Sends the 'GET' request to http server.
 * @param {AjaxUtil_requester} requester  The request object
 * @param {AjaxUtil_responser} responser  The response object
 */
AjaxUtil.makeRequest = function(requester, responser) {
  requester.httpmethod = 'GET';
  AjaxUtil.accessServer(requester, responser);
};

/**
 * Sends the 'POST' request to http server.
 * @param {AjaxUtil_requester} requester  The request object
 * @param {AjaxUtil_responser} responser  The response object
 */
AjaxUtil.postContent = function(requester, responser) {
  requester.httpmethod = 'POST';
  AjaxUtil.accessServer(requester, responser);
};

/**
 * This function communicates to server by http protocal. It can support
 * 'GET'/'POST' methods, sync/async functionality, time out functionality, and
 * progress steps callback.
 * @param {AjaxUtil_requester} requester  The request object
 * @param {AjaxUtil_responser} responser  The response object
 */
AjaxUtil.accessServer = function(requester, responser) {
  // send GET request to get xml document
  var xhr = AjaxUtil.getRequestObject();
  if (xhr) {
    // default value
    if (requester.wait == null)
      requester.wait = false;

    responser.xhr = xhr;

    // set timeout
    var waitTimer = null;
    if (requester.timeout) {
      waitTimer = window.setTimeout(function(){
        xhr.abort();
        if (responser.onTimeout) {
          responser.onTimeout();
        }
      }, requester.timeout);
    }

    // response function
    var onResponse = function() {
      if (xhr.status == 200) {
        // We got the server's response. Display the response text.
        if (responser.onResponseSuccess) {
          responser.onResponseSuccess();
        }
      } else {
        // Something went wrong. Display error code and error message.
        if (responser.onResponseFail)
          responser.onResponseFail();
      }
      if (responser.onResponse)
        responser.onResponse();
    };

    // Set asyncronized response
    if (!requester.wait) {
      var progressStep = 0;
      xhr.onreadystatechange = function(){
        if (xhr.readyState == 4) {
          if (waitTimer != null)
            window.clearTimeout(waitTimer);
          onResponse();
        } else if (responser.onProgress) {
          responser.onProgress(++progressStep);
        }
      };
    }

    // send request, may cause exception if remote server is down
    if (requester.httpmethod.toUpperCase() == 'GET') {
      xhr.open('GET', requester.url, !requester.wait);
      AjaxUtil.send(xhr, null);
    } else if (requester.httpmethod.toUpperCase() == 'POST') {
      // send request
      xhr.open('POST', requester.url, !requester.wait);
      if (requester.type != null)
        xhr.setRequestHeader('Content-Type', requester.type);

      if (requester.content == null)
        AjaxUtil.send(xhr, '');
      else
        AjaxUtil.send(xhr, AjaxUtil.encodeFormData(requester.content));
    }

    // Do syncronized response
    if (requester.wait) {
      onResponse();
    }
  }
};

/**
 * Sends content to server.
 * @param {Object} xhr  The Ajax object
 * @param {Object} post  The post content
 */
AjaxUtil.send = function(xhr, post) {
  try {
    xhr.send(post);
  } catch (e) {
    _err(SERVER_NOT_REACHABLE);
  }
};

/**
 * Encodes the property name/value pairs of an object as if they were from
 * an HTML form, using application/x-www-form-urlencoded format
 *
 * @param {Object} data  The data object
 * @return {string}
 */
AjaxUtil.encodeFormData = function(data){
  var pairs = [];
  var regexp = /%20/g; // A regular expression to match an encoded space
  for (var name in data) {
    var value = data[name].toString();
    // Create a name/value pair, but encode name and value first
    // The global function encodeURIComponent does almost what we want,
    // but it encodes spaces as %20 instead of as '+'. We have to
    // fix that with String.replace()
    var pair = encodeURIComponent(name).replace(regexp, '+') + '=' +
        encodeURIComponent(value).replace(regexp, '+');
    pairs.push(pair);
  }

  // Concatenate all the name/value pairs, separating them with &
  return pairs.join('&');
};

