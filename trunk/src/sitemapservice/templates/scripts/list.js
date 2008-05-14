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
 * @fileoverview This file contains the DynamicInputList class implementation.
 *     This class manages the list component in the UI.
 *
 * @author chaiying@google.com (Ying Chai)
 */

/**
 * DynamicInputList creates a new empty list element according to the given
 * template, and add it to list container. The list element contains a hidden
 * list item element and an 'add' button. User can add a new empty list item
 * to the list by click the 'add' button. The new empty list item will be a
 * clone node as the hidden list item, with the text input HTML elements in it
 * being empty.
 *
 * TODO: remove the hidden list item, use list item template directly.
 *
 * @param {Object} htmlId  The id set of the related HTML elements, includes
 *   the list container and the list template element.
 * @param {ListSettingComponent} owner  The setting component that manage the
 *   list
 * @constructor
 */
function DynamicInputList(htmlId, owner) {
  /**
   * this.htmlId_.containerId,
   * this.htmlId_.listTplId, ;
   */
  this.htmlId_ = htmlId;

  /**
   * @type {ListSettingComponent}
   */
  this.ownerListSetting_ = owner;

  /**
   * @type {Element}
   */
  this.container_ = Util.checkElemExistAndReturn(this.htmlId_.containerId);

  /**
   * The list items
   * @type {Array.<Element>}
   */
  this.listItems_ = [];


  // initial the template if necessary
  var listTpl = DynamicInputList.Tpl[this.htmlId_.listTplId];
  if (!listTpl) {
    listTpl = Util.checkElemExistAndReturn(this.htmlId_.listTplId);

    // the first direct DIV under the list tpl DIV is the list item tpl DIV
    listTpl.listItemTpl =
        Util.DOM.getFirstChildrenByTagName(listTpl, 'DIV');

    listTpl.addButtonTpl =
        Util.DOM.getFirstChildrenByTagNameAndType(listTpl, 'INPUT', 'button');
    DynamicInputList.Tpl[this.htmlId_.listTplId] = listTpl;
  }

  /**
   * list item template
   * @type {Element}
   */
  this.listItemTpl_ = listTpl.listItemTpl;

  /**
   * events for inputs
   * @type {Object} The property name is the event type, while the value is the
   * array of event handler functions.
   */
  this.events_ = {};

  // container is shared, and addbutton is shared too, but need initialize.
  if (this.container_.addbtn == null) {
    var addButton = listTpl.addButtonTpl.cloneNode(true);

    addButton.value = ADD_BUTTON_VALUE;

    // click 'add' button will trigger the list onchange event
    Util.event.add(addButton, 'click', function(e, target){
      target.ownerContainer_.currentOwnerList_.addItem([]); // add empty line
      TabManager.getInstance().tabHeightChanged();
      // trigger the onChange function
      Util.event.send('change', target);
    });

    // add 'add' button to container
    this.container_.appendChild(addButton);
    this.container_.addbtn = addButton;
    addButton.ownerContainer_ = this.container_;
  }

  /**
   * @type {Element}
   */
  this.addButton_ = this.container_.addbtn;
}

DynamicInputList.prototype.releaseHtml = function() {
  if (this.listItems_) {
    // call the sub items' releaseHtml
    this.clear();
    this.listItems_ = null;
  }
  if (this.container_) {
    if (this.container_.addbtn) {
      this.container_.addbtn.ownerContainer_ = null;
      this.container_.addbtn = null;
      this.container_.currentOwnerList_ = null;
    }
    this.addButton_ = null;
    this.container_ = null;
  }
  if (DynamicInputList.Tpl[this.htmlId_.listTplId]) {
    DynamicInputList.Tpl[this.htmlId_.listTplId] = null;
  }
  if (this.listItemTpl_) {
    this.listItemTpl_ = null;
  }
};

/**
 * The list templates dictionary, records the list templates that have been 
 * used, to reduce the time searching the DOM by HTML id.
 */
DynamicInputList.Tpl = {};

/**
 * Removes the invalid class from all the list items.
 * @private
 */
DynamicInputList.prototype.removeInvalidClass_ = function() {
  this.applyToInputs_('removeInvalidClass');
};

/**
 * Adds event handlers for the list inputs.
 * @param {String} type  The event name
 * @param {Funciton} handler  The event handler
 */
DynamicInputList.prototype.addEvent = function(type, handler) {
  if (!this.events_[type])
    this.events_[type] = [];
  this.events_[type].push(handler);
};

/**
 * Adds new list item to the list. *
 * @param {Array.<String>} values  The values for the fields of the item.
 */
DynamicInputList.prototype.addItem = function(values) {

  // create a new list item
  var listItem = new ListItem(this.listItemTpl_.cloneNode(true), this, values);

  // add the new list item to HTML
  // add event function for inputs in list item
  listItem.applyToInputs('addEvent', this.events_);
  this.addButton_.parentNode.insertBefore(listItem.item_, this.addButton_);
  this.listItems_.push(listItem);
};

/**
 * Gets value of the list.
 *
 * @return {Array.<Array.<String>>} Array of the value strings arrays, each for
 *     one list item, and the sub-array contains strings each for one field in
 *     the item. An empty array will return if no item under the list.
 */
DynamicInputList.prototype.getValue = function() {
  return Util.array.apply(this.listItems_, function(item) {
    return item.getValue();
  });
};

/**
 * Sets value of the list.
 * @param {Array.<Array.<String>>} value  The list value set
 * @param {AccessManager} access  The access manager of the list
 */
DynamicInputList.prototype.setValue = function(value, access) {
  var oldOwner = this.container_.currentOwnerList_;
  if (oldOwner && oldOwner != this) {
    oldOwner.clear();
  }
  this.clear();
  for (var i = 0; i < value.length; i++) {
    this.addItem(value[i]);
  }
  // re-set the access since the HTML has regenerated
  this.setAccess(access);
  this.removeInvalidClass_();
  this.container_.currentOwnerList_ = this;
};

/**
 * Prints this object.
 * @return {String} The string represent of this object
 */
DynamicInputList.prototype.toString = function() {
  var values = this.getValue();
  var str = 'List:';
  Util.array.apply(values, function(value) {
    str += 'Item:\n';
    Util.array.apply(value, function(subval) {
      str += 'sub: ' + subval + '\n';
    });
  });
};

/**
 * Set 'this' list edit status according to the argument.
 * @param {AccessManager} access  The access manager of this list
 */
DynamicInputList.prototype.setAccess = function(access) {
  // set to input/text and 'del' input/button
  Util.array.apply(this.listItems_, function(item) {
    item.setAccess(access);
  });
  // set to input/button 'add'
  AccessManager.setAccessToElem(access, this.addButton_);
};

/**
 * Validates the list setting.
 * @return {Boolean} The result of validation, 'true' means the setting is valid
 */
DynamicInputList.prototype.validate = function() {
  var inputs = this.getInputs_();
  var isValid = true;
  Util.array.applyWithBreak(inputs, function(input) {
    if (Util.CSS.isClass(input, INPUT_INVALID_CSS_CLASS)) {
      isValid = false;
      return false; // break
    } else if (!input.ownerField_.validator().check(input)) {
      isValid = false;
      return false; // break
    }
    return true; // continue
  });
  return isValid;
};

/**
 * Gets input elements of this list.
 * @return {Array.<Element>} The input elements of this list
 * @private
 */
DynamicInputList.prototype.getInputs_ = function() {
  var inputs = [];
  Util.array.apply(this.listItems_, function(item) {
    inputs = inputs.concat(item.getInputs());
  });
  return inputs;
};

/**
 * Applies some action to the input elements in the list.
 * @param {String} type  The action type
 * @param {Object} param  The action parameter
 * @private
 */
DynamicInputList.prototype.applyToInputs_ = function(type, param) {
  Util.array.apply(this.listItems_, function(item) {
    item.applyToInputs(type, param);
  });
};

/**
 * Removes all the items in 'this' list.
 */
DynamicInputList.prototype.clear = function() {
  // Notes: don't using applyToArray to remove node  
  while (true) {
    var item = this.listItems_.pop();
    if (!item) {
      break;
    }
    // item node must be the direct child of list container
    item.removeSelf();
  }
};

/**
 * Removes the item from the list.
 * @param {ListItem} listItem  The list item that need to be removed
 */
DynamicInputList.prototype.removeItem = function(listItem) {
  Util.array.remove(this.listItems_, listItem, true);
  listItem.removeSelf();
};
/////////////////////////////ListItem/////////////////////////////
/**
 * @constructor
 * @param {Element} item  The HTML element of the list item
 * @param {DynamicInputList} owner  The list that the item belongs to
 * @param {Array.<String>} value  The value of the list item
 */
function ListItem(item, owner, value) {
  /**
   * @type {Element}
   */
  this.item_ = item;

  /**
   * @type {DynamicInputList}
   */
  this.ownerList_ = owner;

  /**
   * the first button INPUT under the list item DIV is the delete button
   * @type {Element}
   */
  this.delButton_ = Util.DOM.getFirstDescentdantdantByTagNameAndType(
                        this.item_, 'INPUT', 'button');

  this.delButton_.value = DELETE_BUTTON_VALUE;

  var that = this;
  Util.event.add(this.delButton_, 'click', function() {
    // let the list deal with the change
    that.ownerList_.removeItem(that);
    TabManager.getInstance().tabHeightChanged();
  });

  /**
   * @type {Array.<ComplexField>}
   */
  this.fields_ = [];
  var that = this;
  Util.array.apply(
      Util.DOM.getAllDescentdantByTagName(this.item_, 'DIV'), function(div) {
    if (Util.DOM.hasAttribute(div, 'complexField')) {
      switch(div.getAttribute('complexField')) {
      	case 'urlfind':
          that.fields_.push(new FindComplexField(div, that));
      		break;
      	case 'urlreplace':
          that.fields_.push(new ReplaceComplexField(div, that));
      		break;
      	case 'url':
          that.fields_.push(new ComplexField(
              div, that, new ValidateManager('urlInput')));
          break;
      	case 'urlmatch':
          that.fields_.push(new ComplexField(
              div, that, new ValidateManager('urlmatchInput')));
      		break;
      }
    }
  });

  // fill values to the fields of the item
  this.setValue(value);
}

ListItem.prototype.releaseHtml = function() {
  // Haven't add any reference on the elements
  this.item_ = null;
  this.delButton_ = null;
};

/**
 * Release the resources and references of the list item.
 */
ListItem.prototype.removeSelf = function() {
  this.ownerList_ = null;
  this.item_.parentNode.removeChild(this.item_);
  this.releaseHtml();

  // Notes: don't using applyToArray to remove node
  while (true) {
    var field = this.fields_.pop();
    if (!field) {
      break;
    }
    // item node must be the direct child of list container
    field.removeSelf();
  }
};

/**
 * Gets input elements of this list item.
 * @return {Array.<Element>} The input elements of this list item
 */
ListItem.prototype.getInputs = function() {
  var inputs = [];
  Util.array.apply(this.fields_, function(field) {
    inputs = inputs.concat(field.getInputs());
  });
  return inputs;
};

/**
 * Sets the value to the item.
 * @param {Array.<String>} value  The value set for the fields in the list item.
 *     Each value string for one field.
 */
ListItem.prototype.setValue = function(value) {
  if (value.length == 0)
    return;

  var that = this;
  Util.array.applyToMultiple(this.fields_, value, function(field, val) {
    field.setValue(val);
  });
};

/**
 * Applies some action to the input elements in the list item.
 * @param {String} type  The action type
 * @param {Object} param  The action parameter
 */
ListItem.prototype.applyToInputs = function(type, param) {
  Util.array.apply(this.fields_, function(field) {
    field.applyToInputs(type, param);
  });
};

/**
 * Sets the access right of the list item.
 * @param {AccessManager} access  The access manager of the list item
 */
ListItem.prototype.setAccess = function(access) {
  // set to input/text
  Util.array.apply(this.fields_, function(field) {
    field.applyToInputs('setAccess', access);
  });
  // set to input/button 'del'
  AccessManager.setAccessToElem(access, this.delButton_);
};

/**
 * Gets value of the list item.
 * @return {Array.<String>} Array of the values for the item, each for one field
 */
ListItem.prototype.getValue = function() {
  return Util.array.apply(this.fields_, function(field) {
    return field.getValue();
  });
};

/**
 * Updates the 'Replace' field according to the 'Find' field value. If the
 * 'Replace' field value is set through the argument, the text input elements
 * values will be set as the argument.
 * @param {String?} opt_repVal  The value of the 'Replace' field
 */
ListItem.prototype.updateReplaceField = function(opt_repVal) {
  var findField = this.fields_[0];
  var repField = this.fields_[1];
  Util.assert(findField instanceof FindComplexField &&
              repField instanceof ReplaceComplexField);

  // get find field value
  var findVal = findField.getValue();

  // generate the replace field according to find field value
  repField.regenerate(findVal, opt_repVal);
};

//////////////////////////// ComplexField ///////////////////
/**
 * @constructor
 * @param {HTMLElement} elem  The field HTML element
 * @param {ListItem} owner  The list item that this field belongs to
 * @param {ValidateManager} validator  The validator of the list
 */
function ComplexField(elem, owner, validator) {
  if (arguments[0] == 'inheritsFrom') {
    // called by inheritsFrom function
    return;
  }
   
  /**
   * @type {ValidateManager}
   */
  this.validator_ = validator;
  
  /**
   * DIV element with 'ComplexField' attribute.
   * @type {HTMLElement}
   */
  this.elem_ = elem;

  /**
   * @type {ListItem}
   */
  this.ownerListItem_ = owner;

  /**
   * @type {Array.<Element>}
   */
  this.inputs_ =
      Util.DOM.getAllDescentdantByTagNameAndType(this.elem_, 'INPUT', 'text');

  var that = this;
  Util.array.apply(this.inputs_, function(input) {
    input.ownerField_ = that;
    var range = that.validator_.range();
    if (range && input.tip) {
      input.tip += ' Range=' + range;
    }
  });

  /**
   * the direct SPANs are the field parts.
   * for replace field it will be reset in regenerate function
   * @type {Array.<Element>}
   */
  this.spans_ = Util.DOM.getAllChildrenByTagName(this.elem_, 'SPAN');

  var name = this.elem_.getAttribute('complexField');
  Util.assert(name != null && name != '', 'lack complexField attribute');
  
  var fieldMode = FieldMode.find(name);
  if (!fieldMode) {
    Util.console.error('field mode cannot find:'+name);
  }

  /**
   * @type {FieldMode}
   */
  this.mode_ = fieldMode;
}

/**
 * Gets The validator of the list field.
 * @return {ValidateManager} The validator of the list field
 */
ComplexField.prototype.validator = function() {
  return this.validator_;
};

ComplexField.prototype.releaseHtml = function() {
  if (this.inputs_) {
    while (true) {
      var input = this.inputs_.pop();
      if (!input) {
        break;
      }
      input.ownerField_ = null;
    }
  }
  if (this.spans_) {
    while (this.spans_.pop());
  }
};
/**
 * Release the resources and references of the field.
 */
ComplexField.prototype.removeSelf = function() {
  this.ownerListItem_ = null;
  this.releaseHtml();
};

/**
 * Gets input elements of this field.
 * @return {Array.<Element>}  The input elements
 */
ComplexField.prototype.getInputs = function() {
  return this.inputs_;
};

/**
 * Applies some action to the input elements in the field.
 * @param {String} type  The action type
 * @param {Object} param  The action parameter
 */
ComplexField.prototype.applyToInputs = function(type, param) {
  for (var i = 0; i < this.inputs_.length; i++) {
    var input = this.inputs_[i];
    switch(type) {
    	case 'addEvent':
        var events = param;

        for (var type in events) {
          Util.array.apply(events[type], function(handler) {
            Util.event.add(input, type, handler);
          });
        }
    		break;

    	case 'setAccess':
        var access = param;
        AccessManager.setAccessToElem(access, input);
    		break;

    	case 'removeInvalidClass':
        Util.CSS.removeClass(input, INPUT_INVALID_CSS_CLASS);
    		break;
    }
  }
};

/**
 * Set the value to field.
 * @param {String} value  The value of the field
 */
ComplexField.prototype.setValue = function(value) {
  this.setValueToHTML_(value);
};

/**
 * Sets the field value to the HTML element. It will split the value and set
 * it to each inputs. The 'SPAN' elements have been set statically in the 
 * template in HTML page.
 * @param {String} value  The value of the field
 * @private
 */
ComplexField.prototype.setValueToHTML_ = function(value) {
  // get the pattern, and split the value,
  // get the values for text INPUTs, and fill the INPUTs
  var vals = value.match(this.mode_.pattern);
  if (vals == null) {
    Util.console.error('invalid value for ComplexField');
  }

  // clear the value, then set it
  Util.array.applyToMultiple(
      this.inputs_, this.mode_.position, function(input, idx) {
    input.value = vals[idx];
  });
};

/**
 * Gets value of the field.
 * @return {String} The value of the field
 */
ComplexField.prototype.getValue = function() {
  return this.getValueFromHTML_();
};

/**
 * Gets value of the field from HTML.
 * @return {String} The value of the field
 * @private
 */
ComplexField.prototype.getValueFromHTML_ = function() {
  // the direct text INPUTs and SPANs are the field parts, put their values
  // together according to the sequence to get the field value

  // assume the sequence is <span><input><span><input>...
  var values = [];
  Util.array.applyToMultiple(this.spans_, this.inputs_, function(span, input) {
    values.push(span.innerHTML);
    values.push(input.value);
  });
  if (this.spans_.length > this.inputs_.length) {
    values.push(this.spans_[this.inputs_.length].innerHTML);
  }

  return values.join('');
};

/////////////////////// FindComplexField//////////////
/**
 * @constructor
 * @param {HTMLElement} elem  The field HTML element
 * @param {ListItem} owner  The list item that this field belongs to
 */
function FindComplexField(elem, owner) {
  this.parent.constructor.call(this, elem, owner, 
                               new ValidateManager('urlfindInput'));
  var that = this;
  Util.array.apply(this.inputs_, function(input) {
    Util.event.add(input, 'change', function(){
      that.ownerListItem_.updateReplaceField();
    });
  });
}
FindComplexField.inheritsFrom(ComplexField);

/**
 * Sets the field value to HTML element.
 * 
 * For 'Replace' field, it will also set the 'SPAN' elements 'innerHTML' 
 * attribute according to the 'Find' field's value;
 * 
 * In the list template, the 'Find' field must be defined before the 'Replace' 
 * field to ensure the 'set' function of 'Find' field will be called before that
 * of 'Replace' field.
 * 
 * @param {String} value  The value of the field
 * @private
 */
FindComplexField.prototype.setValueToHTML_ = function(value) {
  this.parent.setValueToHTML_.call(this, value);
  this.ownerListItem_.updateReplaceField();
};

/////////////////////// ReplaceComplexField//////////////
/**
 * @constructor
 * @param {HTMLElement} elem  The field HTML element
 * @param {ListItem} owner  The list item that this field belongs to
 */
function ReplaceComplexField(elem, owner) {
  this.parent.constructor.call(this, elem, owner,
                               new ValidateManager('urlreplaceInput'));

  // <span>/</span>
  // <input type='text' class='url' pattern='urlreplaceInput'
  // transmark='tip:urlReplace'>
  this.spanElemTpl_ = document.createElement('SPAN');
  Util.CSS.addClass(this.spanElemTpl_, TEXT_LABEL_CSS_CLASS);
  Util.CSS.addClass(this.spanElemTpl_, VMIDDLE_CSS_CLASS);

  this.inputElemTpl_ = document.createElement('INPUT');
  this.inputElemTpl_.type = 'text';
  this.inputElemTpl_.setAttribute('pattern', 'urlreplaceInput');
  Util.CSS.addClass(this.inputElemTpl_, INPUT_URLREPLACE_CSS_CLASS);

  // do transmark by hand
  this.inputElemTpl_.setAttribute(TRANSMARK_ATTRNAME, 'tip:urlReplace');
  TransMarkSet.transLanguageForInputElem(this.inputElemTpl_);
}
ReplaceComplexField.inheritsFrom(ComplexField);

/**
 * Sets the 'Replace' field value.
 * @param {String} value  The value of the field
 */
ReplaceComplexField.prototype.setValue = function(value) {
  this.setValueToHTML_(value);
};

/**
 * Sets the field value to HTML element.
 * 
 * For 'Replace' field, the 'SPAN' elements have been set before according to 
 * the 'Find' field's value;
 * 
 * In the list template, the 'Find' field must be defined before the 'Replace' 
 * field to ensure the 'set' function of 'Find' field will be called before that
 * of 'Replace' field.
 * 
 * @param {String} value  The value of the field
 * @private
 */
ReplaceComplexField.prototype.setValueToHTML_ = function(value) {
  // the pattern to extract the inputs values
  var pat = /\[[^\]]*\]/g; 
  
  // use the pattern to match the value string
  var values = value.match(pat);
  if (!values)
    Util.console.error('invalid value for ComplexField');

  // set input elements
  Util.array.applyToMultiple(this.inputs_, values, function(input, val) {
    // remove the '[' and ']' in the results.
    input.value = val.replace(/[\[\]]/g, '');
  });
};

/**
 * Generates 'Replace' field dynamically according to the 'Find' field value,
 * with the inputs elements being empty.
 *
 * @param {String} findValue  The value of the 'Find' field
 * @param {String} opt_repVal  The value of the 'Replace' field
 */
ReplaceComplexField.prototype.regenerate = function(findValue, opt_repVal) {
  // split the findvalue, each span value
  var spanVals = ReplaceComplexField.splitFindFieldValueForLabels_(findValue);

  // take the span values to generate the replace fields
  // add to replace field
  this.inputs_ = [];
  this.spans_ = [];
  Util.DOM.removeAllChildren(this.elem_);
  for (var i = 0; i < spanVals.length - 1; i++) {
    this.addSpan_(spanVals[i] ? spanVals[i] : '');
    this.addInput_();
  }
  this.addSpan_(spanVals[i] ? spanVals[i] : '');

  if (opt_repVal) {
    // get the replace inputs values according to replace field value
    this.setValueToHTML_(repVal);
  }
};

ReplaceComplexField.prototype.addSpan_ = function(text){
  var newSpanElem = this.spanElemTpl_.cloneNode(true);
  newSpanElem.innerHTML = text;
  this.spans_.push(newSpanElem);
  this.elem_.appendChild(newSpanElem);
};

ReplaceComplexField.prototype.addInput_ = function() {
  var newInputElem = this.inputElemTpl_.cloneNode(true);
  newInputElem.ownerField_ = this;
  this.inputs_.push(newInputElem);
  this.elem_.appendChild(newInputElem);
};

ReplaceComplexField.prototype.releaseHtml = function() {
  ReplaceComplexField.prototype.parent.releaseHtml();
  this.spanElemTpl_ = null;
  this.inputElemTpl_ = null;
};

/**
 * Takes the 'Find' field value, split it and extract user-unchanged values for
 * 'Replace' field.
 *
 * @param {String} findValue  The value of the 'Find' field
 * @return {Array.<String>}  The value strings for the 'SPAN' elements in the
 *     'Replace' field.
 * @private
 */
ReplaceComplexField.splitFindFieldValueForLabels_ = function(findValue) {
  var spanVals = [];
  var reg = /((?:[^\[]*)\[)(?:[^\]]*)/g;
  var vals = reg.exec(findValue);
  while (vals) {
    spanVals.push(vals[1]);
    vals = reg.exec(findValue);
  }
  vals = /\][^\[\]]*$/.exec(findValue);
  if (vals)
    spanVals.push(vals[0]);
  return spanVals;
};

////////////////////////// FieldMode /////////////////////
/**
 * @constructor
 * @param {String} name  The name of the field mode
 * @param {RegExp?} pattern  The pattern of the field mode, which used to 
 *   extract the value of inputs from the field value
 * @param {Array.<Number>?} position  The postion of the inputs value in the 
 *   pattern-match result
 */
function FieldMode(name, pattern, position) {
  this.name = name;
  this.pattern = pattern;
  this.position = position;
}

/**
 * Prints this object.
 * @return {String} The string represent of this object
 */
FieldMode.prototype.toString = function() {
  return this.name + ', ' + this.pattern + ', ' + this.position;
};

/**
 * It defines the look-up table for the 'complexField' attribute of the field
 * HTML element. The table item is an object called FieldMode, which has
 * three member, 'name', 'pattern', and 'position'; The attribute value is
 * corresponding to the 'name' member, and the 'pattern' member defines the
 * RegExp that is used to check and extract values of the inputs elements from 
 * the field value; The 'position' member is an array that is used
 * to pick up the inputs values from the match result of the 'pattern'.
 *
 * @enum {FieldMode}
 * @private
 */
FieldMode.modes_ = {
  /**
   * A = [\w-]  B = [\w- ./?%&=] = [A ./?%&=]
   * C = [\w- ./?%&=\*] = [B\*]
   */

  /**
   * pattern of value:
   * url = http://(A+\.)*A+(/B*)?=
   * it can also be 'http://' when it is just added
   *
   * UI:
   * http://{INPUTElement}
   */
  URL: new FieldMode(
    'url',
    /^http\:\/\/((?:[\w-]+\.)*[\w-]+(?:\/[\w- \.\/\?\%\&\=]*)?\=)?$/,
    [1]
  ),

  /**
   * pattern of value:
   * urlmatch = /C*
   * it can also be '/' when it is just added
   *
   * UI:
   * /{INPUTElement}
   */
  URLMATCH: new FieldMode(
    'urlmatch',
    /^\/([\w- \.\/\?\%\&\=\*]*)?$/,
    [1]
  ),

  /**
   * pattern of value:
   * urlfind = /(C*\[C*\]C*)+
   * it can also be '/' when it is just added
   *
   * UI:
   * /{INPUTElement}
   */
  URLFIND: new FieldMode(
    'urlfind',
    // It's too long, has to use RegExp contructor to split it into multi-lines.
    new RegExp('^\\/((?:[\\w- \\.\\/\\?\\%\\&\\=\\*]*' +
               '\\[[\\w- \\.\\/\\?\\%\\&\\=\\*]*\\]' +
               '[\\w- \\.\\/\\?\\%\\&\\=\\*]*)+)?$'),
    [1]
  ),

  /**
   * pattern of value:
   * urlreplace = /(C*\[B*\]C*)+
   * it can also be '/' or '' when it is just added
   *
   * UI:
   * /abc[{INPUTElement}]xyz[{INPUTElement}]
   */
  URLREP: new FieldMode(
    'urlreplace',
    null, // it's too complicated, need a function to do it
    null // generated dynamically
  )
};

/**
 * Gets the field mode by name.
 * @param {String} name  The FieldMode name
 * @return {FieldMode} The FieldMode object
 */
FieldMode.find = function(name) {
  var fieldMode = null;
  Util.object.apply(FieldMode.modes_, function(mode) {
    if (mode.name == name) {
      fieldMode = mode;
    }
  });
  return fieldMode;
};

