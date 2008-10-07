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
 * If mouse is over the component, show the tip.
 * If mouse is out of the component and not on the tip, hide the tip.
 * If mouse is out of the tip, hide it.
 *
 * TODO:
 * If mouse click the tip, hide it.
 * If mouse is in input, show the tip (add short key to trigger).
 * If mouse click the label, show the tip.
 * Add latency to show/hide action.
 *
 * @author chaiying@google.com (Ying Chai)
 */
var Tips = {
  elem: '',
  isPopped: false,
  isMouseOnTip: false,
  popDiv: null,
  timer: null,
  X_OFFSET: 15,  // Pixels to the right of the mouse pointer
  Y_OFFSET: 15,  // Pixels below the mouse pointer
  SHOW_DELAY: 500,    // Milliseconds after mouseover
  HIDE_DELAY: 200,
  DELAY_BETWEEN_CLICK: 50,
  DELAY_ON_CLICK: 0
};

/**
 * Adds tip-related event handlers for the target element.
 * @param {HTMLElement} target  The element that has a tip
 */
Tips.schedule = function(target) {
  _event(target, 'mouseover', Tips.eventHandlers.onMouseOverComponent);
  _event(target, 'mouseout', Tips.eventHandlers.onMouseOutComponent);
};

Tips.scheduleIcon = function(icon) {
  icon.bHelpTooltips = true;
  _event(icon, 'mouseover', Tips.eventHandlers.onMouseOverComponent);
  _event(icon, 'click', Tips.eventHandlers.onClickIcon);
};

function GetAbsolutePosition(object) {
  var p = {x: 0, y: 0};

  p.x = object.offsetLeft;
  p.y = object.offsetTop;

  if (object.offsetParent) {
    var parent = GetAbsolutePosition(object.offsetParent);
    p.x += parent.x;
    p.y += parent.y;
  }

  p.w = object.offsetWidth;
  p.h = object.offsetHeight;

  return p;
}

Tips.tipShow = function(e, target, opt_time) {
    // if the tips is showing and is the same elem, just ignore
    if (Tips.isPopped) {
      // click
      if (opt_time == 0) {
        Tips.hidePopup(0);
        opt_time = Tips.DELAY_BETWEEN_CLICK;
      } else if (Tips.popDiv.elem == target) {
        return;
      }
    }
    var event = e || window.event;

    // the position of the mouse
    var x = event.clientX + Geometry.getHorizontalScroll();
    var y = event.clientY + Geometry.getVerticalScroll();

    // adjust to the position of showing the tips
    x += Tips.X_OFFSET;
    y += Tips.Y_OFFSET;

    if (target.bHelpTooltips) {
      var p = GetAbsolutePosition(target);
      // extend 2px for UX
      Tips.activeRegionLeft_ = p.x - 2;
      Tips.activeRegionTop_ = p.y - 2;
    }

    Tips.elem = target;
    Tips.showPopup(x, y, opt_time);
};

/**
 * The event handlers for Tips.
 */
Tips.eventHandlers = {
  /**
   * If user moves the pointer over the element and keeps it there for
   * SHOW_DELAY milliseconds, show the tip.
   * @param {Event} e  The event that occurs
   * @param {Element} target  The element the event belongs to
   */
  onMouseOverComponent: function(e, target) {Tips.tipShow(e, target);},

  /**
   * If user moves the pointer out of the element and keeps it out for
   * HIDE_DELAY milliseconds, hide the tip.
   * @param {Event} e  The event that occurs
   * @param {Element} target  The element the event belongs to
   */
  onMouseOutComponent: function(e, target) {
    Tips.hidePopup();
  },

  /**
   * If user positions the pointer over the help icon and clicks, show the tip.
   * @param {Event} e  The event that occurs
   * @param {Element} target  The element the event belongs to
   */
  onClickIcon: function(e, target) {
    Tips.tipShow(e, target, Tips.DELAY_ON_CLICK);
  }
};

/**
 * Shows the tip box.
 * @param {Number} x  The horizen postion of the box
 * @param {Number} y  The vertical postion of the box
 */
Tips.showPopup = function(x, y, opt_delay) {
  if (!Tips.popDiv) {
    var div = document.createElement('DIV');
    Util.CSS.addClass(div, 'popup');
    Tips.popDiv = div;
    document.body.appendChild(Tips.popDiv);
  }

  // close old hide/show event
  if (Tips.timer)
    window.clearTimeout(Tips.timer);

  // schedule to show tips
  function show_() {
    // records which is the tooltips owner
    Tips.popDiv.elem = Tips.elem;
    // fill box with the tooltips
    Tips.popDiv.innerHTML = Tips.elem.tip;

    // move tooltips box to right position
    Tips.moveTo_(Tips.popDiv, x, y);

    // show the box
    Util.CSS.removeClass(Tips.popDiv, HIDDEN_CSS);

    if (Tips.elem.bHelpTooltips) {
      // remove onmouseout handler, onmouseoutacitveregion will be responsible
      // for closing the tooltips.
      Tips.elem.onmouseout = null;

      var w = Tips.popDiv.clientWidth || Tips.popDiv.offsetWidth;
      var h = Tips.popDiv.clientHeight || Tips.popDiv.offsetHeight;
      var left = Tips.activeRegionLeft_;
      var top = Tips.activeRegionTop_;
      var right = x + w;
      var bottom = y + h;
      // If user moves the pointer out of the active region and keeps it out for
      // y milliseconds, hide the tip.
      _event(document, 'mousemove', function(e) {
        var event = e || window.event;
        // position of the current mouse
        var x = event.clientX + Geometry.getHorizontalScroll();
        var y = event.clientY + Geometry.getVerticalScroll();

        if (x < left || x > right || y < top || y > bottom) {
          Tips.hidePopup();
        }
      });
    } else {
      Util.event.clear(document, 'mousemove');
    }

    Tips.isPopped = true;
  }
  if (opt_delay == 0) {
    show_();
  } else {
    Tips.timer = window.setTimeout(show_,
        opt_delay != null ? opt_delay : Tips.SHOW_DELAY);
  }
};

/**
 * Hides the tip box.
 */
Tips.hidePopup = function(opt_delay) {
  // close old hide/show event
  if (Tips.timer)
    window.clearTimeout(Tips.timer);

  if (Tips.isMouseOnTip) {
    return;
  }

  function hide_(){
    Util.CSS.addClass(Tips.popDiv, HIDDEN_CSS);
    Util.event.clear(document, 'mousemove');
    Tips.isPopped = false;
  }

  if (opt_delay == 0) {
    hide_();
  } else {
    Tips.timer = window.setTimeout(hide_,
        opt_delay != null ? opt_delay : Tips.HIDE_DELAY);
  }
};

/**
 * Moves the element to the position.
 * @param {Object} elem  The element to be moved
 * @param {Object} left  The left side destination position of the element
 * @param {Object} top  The top side destination position of the element
 * @private
 */
Tips.moveTo_ = function(elem, left, top) {
  elem.style.left = left;
  elem.style.top = top;
};

/**
 * Copy from Geometry lib.
 * Initializes the Geometry lib.
 */
function Geometry() {
  if (window.innerWidth) {
    //All but IE
    Geometry.getViewportWidth = function(){
      return window.innerWidth;
    };
    Geometry.getViewportHeight = function(){
      return window.innerHeight;
    };
    Geometry.getHorizontalScroll = function(){
      return window.pageXOffset;
    };
    Geometry.getVerticalScroll = function(){
      return window.pageYOffset;
    };
  } else if (document.documentElement &&
             document.documentElement.clientWidth) {
    // IE6 w/ doctype
    Geometry.getViewportWidth = function(){
      return document.documentElement.clientWidth;
    };
    Geometry.getViewportHeight = function(){
      return document.documentElement.clientHeight;
    };
    Geometry.getHorizontalScroll = function(){
      return document.documentElement.scrollLeft;
    };
    Geometry.getVerticalScroll = function(){
      return document.documentElement.scrollTop;
    };
  } else if (document.body.clientWidth) {
    // IE4,5,6(w/o doctype)
    Geometry.getViewportWidth = function(){
      return document.body.clientWidth;
    };
    Geometry.getViewportHeight = function(){
      return document.body.clientHeight;
    };
    Geometry.getHorizontalScroll = function(){
      return document.body.scrollLeft;
    };
    Geometry.getVerticalScroll = function(){
      return document.body.scrollTop;
    };
  }
}

// Initializes the Geometry lib on loading page.
_event(window, 'load', Geometry);
