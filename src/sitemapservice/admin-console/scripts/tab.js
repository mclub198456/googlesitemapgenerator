// Copyright 2007 Google Inc.
// All Rights Reserved.

/**
 * @fileoverview
 *
 * @author chaiying@google.com
 */

/**
 * Manage a group of tabs, only show one tab each time.
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
 *
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
 * Manage show/hide the tab and change switcher CSS class.
 * @constructor
 * @param {Object} id
 * @param {Object} opt_sid
 */
function Tab(id, opt_sid, opt_handler) {
  this.page_ = _gel(id);
  if (opt_sid) {
    this.switcher_ = _gel(opt_sid);
    if (opt_handler) {
      _event(this.switcher_, 'click', function() {
        opt_handler(id);
      });
    }
  }
}

/**
 *
 */
Tab.prototype.show = function() {
  _show(this.page_);
  if (this.switcher_) {
    Util.CSS.changeClass(this.switcher_, INACTIVETAB_CSS, ACTIVETAB_CSS);
  }
};
Tab.prototype.hide = function() {
  _hide(this.page_);
  if (this.switcher_) {
    Util.CSS.changeClass(this.switcher_, ACTIVETAB_CSS, INACTIVETAB_CSS);
  }
};
