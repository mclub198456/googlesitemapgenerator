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
 * @fileoverview Defines some classes to manage a group of page tabs, user will
 * only see one tab each time, can switch them by click some navigation bar.
 */

/**
 * A tab set.
 * @constructor
 * @param {Object} tabIdSet
 * @param {Object} opt_switcherIdSet
 */
function TabSet(tabIdSet, opt_hasSwitcher, opt_swtHandler) {
  this.tabs_ = {};
  var tabs = this.tabs_;
  if (opt_hasSwitcher) {
    _arr(tabIdSet, function(id){
      tabs[id] = new Tab(id, id + '-s', opt_swtHandler);
    });
  } else {
    _arr(tabIdSet, function(id){
      tabs[id] = new Tab(id);
    });
  }
  this.curTabId_ = null;
}

/**
 * Show the tab with the id.
 * @param {Object} id
 */
TabSet.prototype.show = function(id) {
  if (this.curTabId_ == id)
    return;

  var oldId = this.curTabId_;
  this.curTabId_ = id;

  // hide old one
  if (oldId != null) {
    this.tabs_[oldId].hide();
  }

  // show new one
  this.tabs_[id].show();
  return oldId;
};

TabSet.prototype.curId = function() {
  return this.curTabId_;
};

/**
 * A tab.
 * @constructor
 * @param {Object} id
 * @param {Object} opt_sid
 */
function Tab(id, opt_sid, opt_handler) {
  this.page_ = _gel(id);
  if (opt_sid) {
    this.switcher_ = _gel(opt_sid);
    if (opt_handler) {
      _event(this.switcher_.getElementsByTagName('A')[0], 'click', function() {
        opt_handler(id);
      });
    }
  }
}

/**
 * Show this tab.
 */
Tab.prototype.show = function() {
  _show(this.page_);
  if (this.switcher_) {
    Util.CSS.changeClass(this.switcher_, INACTIVETAB_CSS, ACTIVETAB_CSS);
  }
};

/**
 * Hide this tab.
 */
Tab.prototype.hide = function() {
  _hide(this.page_);
  if (this.switcher_) {
    Util.CSS.changeClass(this.switcher_, ACTIVETAB_CSS, INACTIVETAB_CSS);
  }
};
