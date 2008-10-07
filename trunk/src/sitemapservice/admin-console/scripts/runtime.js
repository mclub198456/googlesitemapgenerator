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
 * @fileoverview
 *
 * @author chaiying@google.com (Ying Chai)
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
 * Gets the RuntimeManager singleton object.
 * @return {RuntimeManager} The RuntimeManager singleton object
 */
var _getRuntime = function() {
  if (_getRuntime.instance_ == null) {
    _getRuntime.instance_ = new RuntimeManager();
  }
  return _getRuntime.instance_;
};

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

  this.loadCurSite();
};

/**
 *
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

  this.loadCurSite();
};

RuntimeManager.prototype.error = function(msg) {
  _gel('rt-err-txt').innerHTML = msg;
  _show(_gel('rt-err-box'));
};
RuntimeManager.prototype.clearErr = function() {
  _hide(_gel('rt-err-box'));
};

RuntimeManager.prototype.loadCurSite = function() {
  if (!this.currentSiteId_ || this.currentSiteId_ == GLOBAL_SETTING_ID) {
    return;
  }

  var xml = this.sites_[this.currentSiteId_];
  if (!xml) {
    // show warning
    this.error(SITE_NOT_RUNNING);
    _hide(_gel('status-area'));
  } else {
    this.clearErr();
    _show(_gel('status-area'));
    // show site info
    this.setSite_(xml);
    // show services info
    var me = this;
    _arr(RuntimeManager.fields_.site.services,
         function(service){me.setService_(service, xml);});
    // hack
    var elem = _gel('wf-status');
    if (_getSetting().cursite_().getService(
        SettingOperator.services.enums.WEBSERVERFILTER).isEnabled()) {
      elem.innerHTML = 'Running';
      Util.CSS.changeClass(elem, 'status-disabled', 'status-success');
    } else {
      elem.innerHTML = 'Disabled';
      Util.CSS.changeClass(elem, 'status-success', 'status-disabled');
    }
    this.onloadListeners_.inform();
  }
};

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
RuntimeManager.prototype.setSite_ = function(xml) {
  this.set_(RuntimeManager.fields_.site.general, xml);
  var me = this;
  var avgsize = this.avgUrlSize();
  this.setSpaceInfo_('memUsage', parseInt(xml.getAttribute('memory_used')),
      _getSetting().curMemLimit() * avgsize);
  this.setSpaceInfo_('diskUsage', parseInt(xml.getAttribute('disk_used')),
      _getSetting().curDiskLimit() * avgsize);
};
RuntimeManager.prototype.setService_ = function(rtInfos, xml) {
  this.set_(rtInfos, xml);
};

/**
 * Sets the runtime informations on page.
 * @param {Object} rtInfos  The informations that should set
 * @param {XMLNode} xml  The XML that has the informations
 */
RuntimeManager.prototype.set_ = function(rtInfos, xml) {
  var subxml = xml;
  _arr(rtInfos, function(rtInfo) {
    if (rtInfo.elem == null) {
      rtInfo.elem = _gel(rtInfo.id);
    }
    if (rtInfo.nodeX) {
      subxml = xml.getElementsByTagName(rtInfo.nodeX)[0];
    }
    var value = subxml.getAttribute(rtInfo.tag);
    // special code for some runtime info
    switch (rtInfo.tag) {
      case 'success':
        if (/-s/.test(rtInfo.id)) {
          if (value == 'true') {
            Util.CSS.changeClass(
                rtInfo.elem, 'status-failed', 'status-success');
          } else {
            Util.CSS.changeClass(
                rtInfo.elem, 'status-success', 'status-failed');
          }
        }
        value = value == 'true' ? 'success' : 'failed';
        break;
      case 'last_update':
      case 'last_ping':
        if (value == '-1') {
          value = 'N/A';
          rtInfos[0].elem.innerHTML = 'N/A'; // success
          rtInfos[1].elem.innerHTML = 'Disabled'; // success-s
          Util.CSS.removeClass(rtInfos[1].elem, 'status-success');
          Util.CSS.removeClass(rtInfos[1].elem, 'status-failed');
          Util.CSS.addClass(rtInfos[1].elem, 'status-disabled');
        }
        break;
      case 'memory_used':
      case 'disk_used':
        value = Util.getSpaceString(value);
        break;
    }
    rtInfo.elem.innerHTML = value;
  });
};

RuntimeManager.prototype.setSpaceInfo_ = function(id, cost, ttl) {
    var usage = (cost * 100 / ttl).toFixed(5) + '%';
    var graph = document.getElementById(id + '-graph');
    if (graph) graph.style.width = usage;
    _gel(id + '-cost').innerHTML = Util.getSpaceString(cost);
    _gel(id + '-ttl').innerHTML = Util.getSpaceString(ttl);
    _gel(id + '-txt').innerHTML = usage;
};

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
 * Runtime information fields, each has its HTML id and XML xpath.
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
    // for services
    services: [
      // webserver filter
      [{
        id: 'webServerFilterServiceInfoUrlsCount',
        tag: 'urls_count',
        nodeX: 'WebServerFilterInfo'
      }],
      // log parser
      [{
        id: 'logParserServiceInfoSuccess',
        tag: 'success',
        nodeX: 'LogParserInfo'
      }, {
        id: 'logParserServiceInfoSuccess-s',
        tag: 'success'
      },
      {
        id: 'logParserServiceInfoLastUpdate',
        tag: 'last_update'
      },
      {
        id: 'logParserServiceInfoUrlsCount',
        tag: 'urls_count'
      }],
      // file scanner
      [{
        id: 'fileScannerServiceInfoSuccess',
        tag: 'success',
        nodeX: 'FileScannerInfo'
      }, {
        id: 'fileScannerServiceInfoSuccess-s',
        tag: 'success'
      },
      {
        id: 'fileScannerServiceInfoLastUpdate',
        tag: 'last_update'
      },
      {
        id: 'fileScannerServiceInfoUrlsCount',
        tag: 'urls_count'
      }],
      // web sitemap
      [{
        id: 'webSitemapServiceInfoSuccess',
        tag: 'success',
        nodeX: 'WebSitemapServiceInfo'
      }, {
        id: 'webSitemapServiceInfoSuccess-s',
        tag: 'success'
      },
      {
        id: 'webSitemapServiceInfoLastUpdate',
        tag: 'last_update'
      },
      {
        id: 'webSitemapServiceInfoUrlsCount',
        tag: 'urls_count'
      },
      {
        id: 'webSitemapServiceInfoUrlsCount-s',
        tag: 'urls_count'
      }],
      // news sitemap
      [{
        id: 'newsSitemapServiceInfoSuccess',
        tag: 'success',
        nodeX: 'NewsSitemapServiceInfo'
      }, {
        id: 'newsSitemapServiceInfoSuccess-s',
        tag: 'success'
      },
      {
        id: 'newsSitemapServiceInfoLastUpdate',
        tag: 'last_update'
      },
      {
        id: 'newsSitemapServiceInfoUrlsCount-s',
        tag: 'urls_count'
      },
      {
        id: 'newsSitemapServiceInfoUrlsCount',
        tag: 'urls_count'
      }],
      // mobile sitemap
      [{
        id: 'mobileSitemapServiceInfoSuccess',
        tag: 'success',
        nodeX: 'MobileSitemapServiceInfo'
      }, {
        id: 'mobileSitemapServiceInfoSuccess-s',
        tag: 'success'
      },
      {
        id: 'mobileSitemapServiceInfoLastUpdate',
        tag: 'last_update'
      },
      {
        id: 'mobileSitemapServiceInfoUrlsCount-s',
        tag: 'urls_count'
      },
      {
        id: 'mobileSitemapServiceInfoUrlsCount',
        tag: 'urls_count'
      }],
      // code search sitemap
      [{
        id: 'codesearchSitemapServiceInfoSuccess',
        tag: 'success',
        nodeX: 'CodeSearchSitemapServiceInfo'
      }, {
        id: 'codesearchSitemapServiceInfoSuccess-s',
        tag: 'success'
      },
      {
        id: 'codesearchSitemapServiceInfoLastUpdate',
        tag: 'last_update'
      },
      {
        id: 'codesearchSitemapServiceInfoUrlsCount-s',
        tag: 'urls_count'
      },
      {
        id: 'codesearchSitemapServiceInfoUrlsCount',
        tag: 'urls_count'
      }],
      // blog search sitemap
      [{
        id: 'blogsearchSitemapServiceInfoSuccess',
        tag: 'success',
        nodeX: 'BlogSearchPingServiceInfo'
      }, {
        id: 'blogsearchSitemapServiceInfoSuccess-s',
        tag: 'success'
      },
      {
        id: 'blogsearchSitemapServiceInfoLastPing',
        tag: 'last_ping'
      },
      {
        id: 'blogsearchSitemapServiceInfoLastUrl',
        tag: 'last_url'
      }]
    ]
  }
};
