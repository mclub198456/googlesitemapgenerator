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
  this.globalMsgBox_ = Util.checkElemExistAndReturn('runtimeInfoErrorg');
  this.siteMsgBox_ = Util.checkElemExistAndReturn('runtimeInfoError');
  this.currentSiteId_ = null;
  /**
   * @type {XmlManager?}
   */
  this.global_ = null;
  /**
   * @type {Set of XmlManagers}
   */
  this.sites_ = {};
  this.loadingFailed_ = false;
}

/**
 * Gets the RuntimeManager singleton object.
 * @return {RuntimeManager} The RuntimeManager singleton object
 */
RuntimeManager.getInstance = function() {
  if (RuntimeManager.getInstance.instance_ == null) {
    RuntimeManager.getInstance.instance_ = new RuntimeManager();
  }
  return RuntimeManager.getInstance.instance_;
};

/**
 * Loads the runtime information to the page.
 * @param {Document?} opt_dom  The XML document that has the runtime info
 */
RuntimeManager.prototype.load = function(opt_dom) {
  /**
   * Sets the informations.
   * @param {Object} rtInfos  The informations that should set
   * @param {XmlManager} xml  The XML that has the informations
   */
  function set_(rtInfos, xml) {
    var subxml = xml;
    Util.array.apply(rtInfos, function(rtInfo) {
      if (rtInfo.elem == null) {
        rtInfo.elem = Util.checkElemExistAndReturn(rtInfo.id);
      }
      if (rtInfo.nodeX) {
        subxml = xml.getNodeByXpath(rtInfo.nodeX);
      }
      var value = subxml.getAttributeByXpath(rtInfo.tag);
      if (rtInfo.tag == '@success') {
        value = value == 'true' ? 'successful' : 'failed';
      }
      if (rtInfo.tag == '@last_update' || 
          rtInfo.tag == '@last_ping') {
        if (value == '-1') {
          value = 'N/A';
          rtInfos[0].elem.innerHTML = 'N/A'; // @success
        }
      }
      rtInfo.elem.innerHTML = value;
    });
  }

  // initialize
  if (opt_dom) {
    this.xml_ = new XmlManager(opt_dom);
    this.global_ = this.xml_.getNodeByXpath('ApplicationInfo');
    if (!this.global_) {
      return;
    }

    /**
     * @type {Array.<XmlManager>}
     */
    var xmls = this.global_.getNodesByXpath('SiteInfo');
    if (xmls) {
      var that = this;
      Util.array.apply(xmls, function(xml){
        var id = xml.getAttribute('site_id');
        that.sites_[id] = xml;
      });
    }
  }

  if (this.xml_ == null) {
    return;
  }

  // get current site id
  var siteId = SiteSettings.getInstance().siteId();
  if (siteId == null) {// Global setting has no site id
    // borrow the GLOBAL_SETTING_ID to represent it's the global setting
    siteId = GLOBAL_SETTING_ID;
  }

  // switch to current site
  // If the new site is the current site, don't switch; but if it's reloading,
  // reload the current site.
  if (siteId == this.currentSiteId_ && !opt_dom) {
    return;
  }
  this.currentSiteId_ = siteId;

  // load current site
  if (this.currentSiteId_ == GLOBAL_SETTING_ID) {
    if (this.global_) {
      Util.CSS.hideElement(this.globalMsgBox_);
      set_(RuntimeManager.fields_.global, this.global_);
    } else {
      this.globalMsgBox_.innerHTML = SITE_NOT_RUNNING;
      Util.CSS.showElement(this.globalMsgBox_);
    }
  } else {
    var xml = this.sites_[this.currentSiteId_];
    if (!xml) {
      // show warning
      this.siteMsgBox_.innerHTML = SITE_NOT_RUNNING;
      Util.CSS.showElement(this.siteMsgBox_);
    } else {
      // hide warning
      Util.CSS.hideElement(this.siteMsgBox_);
      // show info
      set_(RuntimeManager.fields_.site.general, xml);
      Util.array.apply(RuntimeManager.fields_.site.services,
                        function(service){set_(service, xml);});
    }
  }

};

////////////////////////////////////
/**
 * Runtime information fields, each has its HTML id and XML xpath.
 * @private
 */
RuntimeManager.fields_ = {
  global: [
    {
      id: 'memoryUsage',
      tag: '@memory_used'
    },
    {
      id: 'diskUsage',
      tag: '@disk_used'
    },
    {
      id: 'startTime',
      tag: '@start_time'
    }
  ],
  //  the context is /ApplicationInfo/SiteInfo[*]
  site: {
    general: [
      {
        id: 'urlInDatabase',
        tag: '@url_in_database'
      },
      {
        id: 'urlInTempfile',
        tag: '@url_in_tempfile'
      },
      {
        id: 'urlInMemory',
        tag: '@url_in_memory'
      },
      {
        id: 'hostName',
        tag: '@host_name'
      },
      {
        id: 'memoryUsageForSite',
        tag: '@memory_used'
      },
      {
        id: 'diskUsageForSite',
        tag: '@disk_used'
      }
    ],
    // for services
    services: [
      // webserver filter
      [{
        id: 'webServerFilterServiceInfoUrlsCount',
        tag: '@urls_count',
        nodeX: 'WebServerFilterInfo'
      }],
      // log parser
      [{
        id: 'logParserServiceInfoSuccess',
        tag: '@success',
        nodeX: 'LogParserInfo'
      },
      {
        id: 'logParserServiceInfoLastUpdate',
        tag: '@last_update'
      },
      {
        id: 'logParserServiceInfoUrlsCount',
        tag: '@urls_count'
      }],
      // file scanner
      [{
        id: 'fileScannerServiceInfoSuccess',
        tag: '@success',
        nodeX: 'FileScannerInfo'
      },
      {
        id: 'fileScannerServiceInfoLastUpdate',
        tag: '@last_update'
      },
      {
        id: 'fileScannerServiceInfoUrlsCount',
        tag: '@urls_count'
      }],
      // web sitemap
      [{
        id: 'webSitemapServiceInfoSuccess',
        tag: '@success',
        nodeX: 'WebSitemapServiceInfo'
      },
      {
        id: 'webSitemapServiceInfoLastUpdate',
        tag: '@last_update'
      },
      {
        id: 'webSitemapServiceInfoUrlsCount',
        tag: '@urls_count'
      }],
      // news sitemap
      [{
        id: 'newsSitemapServiceInfoSuccess',
        tag: '@success',
        nodeX: 'NewsSitemapServiceInfo'
      },
      {
        id: 'newsSitemapServiceInfoLastUpdate',
        tag: '@last_update'
      },
      {
        id: 'newsSitemapServiceInfoUrlsCount',
        tag: '@urls_count'
      }],
      // video sitemap
      [{
        id: 'videoSitemapServiceInfoSuccess',
        tag: '@success',
        nodeX: 'VideoSitemapServiceInfo'
      },
      {
        id: 'videoSitemapServiceInfoLastUpdate',
        tag: '@last_update'
      },
      {
        id: 'videoSitemapServiceInfoUrlsCount',
        tag: '@urls_count'
      }],
      // mobile sitemap
      [{
        id: 'mobileSitemapServiceInfoSuccess',
        tag: '@success',
        nodeX: 'MobileSitemapServiceInfo'
      },
      {
        id: 'mobileSitemapServiceInfoLastUpdate',
        tag: '@last_update'
      },
      {
        id: 'mobileSitemapServiceInfoUrlsCount',
        tag: '@urls_count'
      }],
      // code search sitemap
      [{
        id: 'codeSearchSitemapServiceInfoSuccess',
        tag: '@success',
        nodeX: 'CodeSearchSitemapServiceInfo'
      },
      {
        id: 'codeSearchSitemapServiceInfoLastUpdate',
        tag: '@last_update'
      },
      {
        id: 'codeSearchSitemapServiceInfoUrlsCount',
        tag: '@urls_count'
      }],
      // blog search sitemap
      [{
        id: 'blogSearchPingServiceInfoSuccess',
        tag: '@success',
        nodeX: 'BlogSearchPingServiceInfo'
      },
      {
        id: 'blogSearchPingServiceInfoLastPing',
        tag: '@last_ping'
      },
      {
        id: 'blogSearchPingServiceInfoLastUrl',
        tag: '@last_url'
      }]
    ]
  }
};
