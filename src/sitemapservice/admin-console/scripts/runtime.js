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
 * @fileoverview
 */

////////////////////////////RuntimeManager////////////////////////////
/**
 * @constructor
 */
function RuntimeManager() {
  this.currentSiteId_ = null;
  /**
   * @type {XMLNode?}
   */
  this.global_ = null;
  /**
   * @type {Object}
   */
  this.sites_ = {};
}

/**
 * Singleton access function.
 */
function _getRuntime() {
  if (_getRuntime.instance_ == null) {
    _getRuntime.instance_ = new RuntimeManager();
  }
  return _getRuntime.instance_;
}

/**
 * Use this function to add a handler when runtime information is refreshed.
 * @param {Object} listener
 */
RuntimeManager.prototype.registOnLoad = function(listener) {
  if (this.onloadListeners_ == null)
    this.onloadListeners_ = new ListenerManager(this);

  this.onloadListeners_.regist(listener);
};

/**
 * Loads the runtime information to the page.
 * @param {Document} dom  The XML document that has the runtime info
 */
RuntimeManager.prototype.setData = function(dom) {
  this.xml_ = dom;
  this.global_ = this.xml_.getElementsByTagName('ApplicationInfo')[0];
  if (!this.global_) {
    return;
  }

  /**
   * @type {Array.<XMLNode>}
   */
  var xmls = this.global_.getElementsByTagName('SiteInfo');
  if (xmls) {
    var me = this;
    _arr(xmls, function(xml){
      var id = xml.getAttribute('site_id');
      me.sites_[id] = xml;
    });
  }

  // always load global(app) runtime.
  this.setApp_();

  this.loadCurSite_();
};

/**
 * Loads runtime information for 'siteIdx' site.
 * @param {String} siteIdx  include GLOBAL_SETTING_ID
 */
RuntimeManager.prototype.loadSite = function(siteIdx) {
  if (this.xml_ == null) { // setData not yet called
    return;
  }

  var siteId = _getSetting().siteId(siteIdx);
  if (siteId == this.currentSiteId_) {
    return;
  }
  this.currentSiteId_ = siteId;

  this.loadCurSite_();
};

/**
 * Show error to user.
 * @param {Object} msg
 */
RuntimeManager.prototype.error_ = function(msg) {
  _gel('rt-err-txt').innerHTML = msg;
  _show(_gel('rt-err-box'));
};

/**
 * Clear error message.
 */
RuntimeManager.prototype.clearErr_ = function() {
  _hide(_gel('rt-err-box'));
};

/**
 * Loads runtime information for current site.
 */
RuntimeManager.prototype.loadCurSite_ = function() {
  if (!this.currentSiteId_ || this.currentSiteId_ == GLOBAL_SETTING_ID) {
    return;
  }

  var xml = this.sites_[this.currentSiteId_];
  if (!xml) {
    // show warning
    this.error_(SITE_NOT_RUNNING);
    _hide(_gel('status-area'));
  } else {
    this.clearErr_();
    _show(_gel('status-area'));
    // show site info
    this.setSite_(xml);
    // show services info
    for (var service in SettingOperator.services.enums)
      this.setService_(SettingOperator.services.enums[service], xml);
    this.onloadListeners_.inform();
  }
};

/**
 * Loads application runtime information.
 */
RuntimeManager.prototype.setApp_ = function() {
  this.set_(RuntimeManager.fields_.global, this.global_);
  var ttlMemSize = 0;
  var ttlDiskSize = 0;
  var len = _getSetting().sitesNum();
  for (var i = 0; i < len; i++) {
    var size = this.avgUrlSize(_getSetting().siteId(i));
    ttlMemSize += _getSetting().curMemLimit(i) * size;
    ttlDiskSize += _getSetting().curDiskLimit(i) * size;
  }
  this.setSpaceInfo_('app-memUsage',
      parseInt(this.global_.getAttribute('memory_used')), ttlMemSize);
  this.setSpaceInfo_('app-diskUsage',
      parseInt(this.global_.getAttribute('disk_used')), ttlDiskSize);
};

/**
 * Loads current site information.
 * @param {Object} xml
 */
RuntimeManager.prototype.setSite_ = function(xml) {
  this.set_(RuntimeManager.fields_.site.general, xml);
  var me = this;
  var avgsize = this.avgUrlSize();
  this.setSpaceInfo_('memUsage', parseInt(xml.getAttribute('memory_used')),
      _getSetting().curMemLimit() * avgsize);
  this.setSpaceInfo_('diskUsage', parseInt(xml.getAttribute('disk_used')),
      _getSetting().curDiskLimit() * avgsize);
};

/**
 * Loads one service information.
 * @param {Object} service One object defined in SettingOperator.services.enums
 * @param {Object} xml
 */
RuntimeManager.prototype.setService_ = function(service, xml) {
  var rtInfos = RuntimeManager.fields_.site.services[service.xmltag];
  // Set 'status' element.
  var elem = _gel(rtInfos[0].id);
  if (_getSetting().cursite_().getService(service).isEnabled()) {
    RuntimeManager.setStatus_(elem, RuntimeManager.serviceState.RUNNING);
  } else {
    RuntimeManager.setStatus_(elem, RuntimeManager.serviceState.DISABLE);
  }
  this.set_(rtInfos, xml);
};

RuntimeManager.serviceState = {
  DISABLE: {html: STATE_DISABLE, css: 'status-disabled'},
  RUNNING: {html: STATE_RUNNING, css: 'status-success'},
  FAILED: {html: STATE_FAILED, css: 'status-failed'}
};

RuntimeManager.setStatus_ = function(elem, state) {
  elem.innerHTML = state.html;
  if (elem.state)
    Util.CSS.removeClass(elem, elem.state.css);
  Util.CSS.addClass(elem, state.css);
  elem.state = state;
};

/**
 * Sets the runtime informations on page.
 * @param {Object} rtInfos  The informations that should set
 * @param {XMLNode} xml  The XML that has the informations
 */
RuntimeManager.prototype.set_ = function(rtInfos, xml) {
  var serviceStateRuntimeInfo = rtInfos[0];
  if (serviceStateRuntimeInfo.elem == null) {
    serviceStateRuntimeInfo.elem = _gel(serviceStateRuntimeInfo.id);
  }
  var isEnabled = true;
  if (serviceStateRuntimeInfo.elem.state == 
          RuntimeManager.serviceState.DISABLE) {
    isEnabled = false;
  }
  
  var curXml = xml;
  _arr(rtInfos, function(rtInfo) {
    if (rtInfo.elem == null) {
      rtInfo.elem = _gel(rtInfo.id);
    }
    if (rtInfo.nodeX) { // Change current Xml node
      curXml = xml.getElementsByTagName(rtInfo.nodeX)[0];
    }
    var value = '';
    if (rtInfo.tag)
      value = curXml.getAttribute(rtInfo.tag);
    switch (rtInfo.tag) {
      case 'success':
        if (isEnabled && value == 'false') {
          RuntimeManager.setStatus_(rtInfo.elem, 
                                    RuntimeManager.serviceState.FAILED);          
        }
        break;
      case 'last_update':
      case 'last_ping':
        if (value == '-1') {
          rtInfo.elem.innerHTML = 'N/A';
          if (isEnabled && serviceStateRuntimeInfo.elem.state == 
                  RuntimeManager.serviceState.FAILED) {
            // Revert to 'running' state.        
            RuntimeManager.setStatus_(serviceStateRuntimeInfo.elem,
                                      RuntimeManager.serviceState.RUNNING);
          }
        } else {
          rtInfo.elem.innerHTML = value;
        }
        break;
      case 'memory_used':
      case 'disk_used':
        rtInfo.elem.innerHTML = Util.getSpaceString(value);
        break;
      case '':
        break;
      default:
        rtInfo.elem.innerHTML = value;
        break;
    }
  });
};

/**
 * Sets space information which has a percentage bar.
 * @param {Object} id
 * @param {Object} cost
 * @param {Object} ttl
 */
RuntimeManager.prototype.setSpaceInfo_ = function(id, cost, ttl) {
  var used = cost * 100 / ttl;
  var used2 = used.toFixed(3);
  if (used2 == 0 && used > 0) used2 = 0.001;
  var usage = used2 + '%';
  var graph = document.getElementById(id + '-graph');
  if (graph) graph.style.width = usage;
  _gel(id + '-cost').innerHTML = 
      Util.getSpaceString(cost) + (Browser.isIE ? ' ' : '');
  _gel(id + '-ttl').innerHTML = 
      Util.getSpaceString(ttl) + (Browser.isIE ? ' ' : '');
  _gel(id + '-txt').innerHTML = usage;
};

/**
 * Calculate average url size.
 * @param {Object} opt_site
 */
RuntimeManager.prototype.avgUrlSize = function(opt_site) {
  // Default URL size in Bytes.
  var defSize = 100;

  // Get current site's xml data.
  var xml = this.sites_[opt_site != null ? opt_site : this.currentSiteId_];
  if (!xml) {
    return defSize;
  }

  // Get number of URLs in database and temp file.
  var urlsInDB = parseInt(xml.getAttribute('url_in_database'));
  var urlsInTemp = parseInt(xml.getAttribute('url_in_tempfile'));

  var space;
  var urls = urlsInDB + urlsInTemp;
  if (urls == 0) {
    // If no URLs in disk, use number of URLs in memory to estimate.
    urls = parseInt(xml.getAttribute('url_in_memory'));
    if (urls == 0) {
      return defSize;
    }
    // Get memory costs.
    space = parseInt(xml.getAttribute('memory_used'));
  } else {
    // Get disk costs.
    space = parseInt(xml.getAttribute('disk_used'));
  }
  // Return the average URL size.
  return space / urls;
};

////////////////////////////////////
/**
 * The map for runtime information fields, which groups the HTML id and 
 * XML xpath for each field.
 * @private
 */
RuntimeManager.fields_ = {
  global: [
    {
      id: 'startTime',
      tag: 'start_time'
    }
  ],
  //  the context is /ApplicationInfo/SiteInfo[*]
  site: {
    general: [
      {
        id: 'urlInDatabase',
        tag: 'url_in_database'
      },
      {
        id: 'urlInTempfile',
        tag: 'url_in_tempfile'
      },
      {
        id: 'urlInMemory',
        tag: 'url_in_memory'
      },
      {
        id: 'hostName',
        tag: 'host_name'
      },
      {
        id: 'memoryUsageForSite',
        tag: 'memory_used'
      },
      {
        id: 'diskUsageForSite',
        tag: 'disk_used'
      }
    ],
    // For services. Use xml tag name for the service property name.
    services: {
      'webServerFilter':
      [{
        id: 'wf-status',
        tag: '',
        nodeX: 'WebServerFilterInfo'
      },
      {
        id: 'webServerFilterInfoUrls',
        tag: 'urls_count'
      }],
      'logParser':
      [{
        id: 'logParserInfoStat',
        tag: 'success',
        nodeX: 'LogParserInfo'
      },
      {
        id: 'logParserInfoTime',
        tag: 'last_update'
      },
      {
        id: 'logParserInfoUrls',
        tag: 'urls_count'
      }],
      'fileScanner':
      [{
        id: 'fileScannerInfoStat',
        tag: 'success',
        nodeX: 'FileScannerInfo'
      },
      {
        id: 'fileScannerInfoTime',
        tag: 'last_update'
      },
      {
        id: 'fileScannerInfoUrls',
        tag: 'urls_count'
      }],
      'webSitemap':
      [{
        id: 'webSitemapInfoStat',
        tag: 'success',
        nodeX: 'WebSitemapServiceInfo'
      },
      {
        id: 'webSitemapInfoTime',
        tag: 'last_update'
      },
      {
        id: 'webSitemapInfoUrls',
        tag: 'urls_count'
      },
      {
        id: 'webSitemapInfoUrls-s',
        tag: 'urls_count'
      }],
      'newsSitemap':
      [{
        id: 'newsSitemapInfoStat',
        tag: 'success',
        nodeX: 'NewsSitemapServiceInfo'
      },
      {
        id: 'newsSitemapInfoTime',
        tag: 'last_update'
      },
      {
        id: 'newsSitemapInfoUrls-s',
        tag: 'urls_count'
      },
      {
        id: 'newsSitemapInfoUrls',
        tag: 'urls_count'
      }],
      'mobileSitemap':
      [{
        id: 'mobileSitemapInfoStat',
        tag: 'success',
        nodeX: 'MobileSitemapServiceInfo'
      },
      {
        id: 'mobileSitemapInfoTime',
        tag: 'last_update'
      },
      {
        id: 'mobileSitemapInfoUrls-s',
        tag: 'urls_count'
      },
      {
        id: 'mobileSitemapInfoUrls',
        tag: 'urls_count'
      }],
      'codeSearchSitemap':
      [{
        id: 'codesearchSitemapInfoStat',
        tag: 'success',
        nodeX: 'CodeSearchSitemapServiceInfo'
      },
      {
        id: 'codesearchSitemapInfoTime',
        tag: 'last_update'
      },
      {
        id: 'codesearchSitemapInfoUrls-s',
        tag: 'urls_count'
      },
      {
        id: 'codesearchSitemapInfoUrls',
        tag: 'urls_count'
      }],
      'blogSearchPing':
      [{
        id: 'blogsearchSitemapInfoStat',
        tag: 'success',
        nodeX: 'BlogSearchPingServiceInfo'
      },
      {
        id: 'blogsearchSitemapServiceInfoLastPing',
        tag: 'last_ping'
      },
      {
        id: 'blogsearchSitemapServiceInfoLastUrl',
        tag: 'last_url'
      }]
    }
  }
};
