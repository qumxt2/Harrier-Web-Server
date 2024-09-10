// Globals
var PREVIOUS_JSON;
var LAST_VERSION;

var POLL_PERIOD_SHORT_SEC = 1;
var POLL_PERIOD_LONG_SEC = 5;
var POLL_COUNT_FOR_LONG = 15;
var POLL_PERIOD_MS = 1;
var POLL_TIMER;
var CONNECTION_TIMEOUT = 15000; // ms
var REFRESH_COUNTER = 0;
var SORT_BY = '';

var AJAX_URL = ''; // API location -- must be set by the parent page

if (!BUTTON_LIST) {
  var BUTTON_LIST = {}; // List of edit buttons
}

var UNITS = -1;

var NOTICE_TIMER;
var NOTICE_HIDE_MS = 5000;
var NOTICE_WAITING_FOR_PUMP = false;

var PULSE_CLASS_ONE = 'pulse';
var PULSE_CLASS_TWO = 'pulseAgain';

var LAST_ETAG = '';

// Connect the edit buttons
function connectButtons(button_list) {         
  jQuery.each(button_list, function(index, element) {
      $('#' + element + '_edit_button').click(function(obj) { 
        removePulseAnimation();
        if (!$('#' + element + '_row_outer').hasClass('row-selected')) {
          // This click is activating the row
          
          // Ensure the other rows are deselected
          collapseAccordian();
          
          $('#' + element + '_row_outer').addClass('row-selected');
          $('#' + element + '_read_container').hide();
          $('#' + element + '_write_container').show();
          
          $('#' + element + '_submit').addClass('btn-show'); // for the green checkbox
          $('#' + element + '_edit_button > i').removeClass().addClass('fa fa-lg fa-times red');
        }
        else {
          // This click is deactivating the row
          collapseAccordian();
        }
      });
  });
}

var FLASH_TEMPLATE = '<div id="dynamic-flash" class="alert alert-%LEVEL%" role="alert">%MESSAGE%</div>';

//Hide the given flash message
function hideFlash() {
  $('#dynamic-flash').remove();
}

//Show the given flash message
function showFlash(message, safe) {
  var out_text = FLASH_TEMPLATE;
  var level_default = 'warning'
  var message_filtered;
  
  if (safe === true) {
    message_filtered = message;
  }
  else {
    message_filtered = message.replace(/['<"]/g, '');
  }

  // Only update if changed
  if ($('#dynamic-flash').val() != message_filtered) {
   hideFlash();
   out_text = out_text.replace(/%MESSAGE%/, message_filtered);
   out_text = out_text.replace(/%LEVEL%/, level_default);
 
   $(out_text).insertBefore('#main-content');    
  }
}

//Save the current label on the given button
function saveLabel(obj) {
$(obj).attr('oldLabel', $(obj).html());
}

//Restore the button's label to what it had been, if one was saved
function restoreLabel(obj) {
var attr = $(obj).attr('oldLabel');
if (typeof attr !== typeof undefined && attr !== false) {
 $(obj).html(attr);
 $(obj).removeAttr('oldLabel');
}
}

//Unselect all of the rows
function collapseAccordian() {
  removePulseAnimation();
  $('.write-container').hide();
  $('.read-container').show(); 
  $('.row-generic').removeClass('row-selected');
  
  $('.edit-button-container > .cancel-button > i').removeClass().addClass('fa fa-lg fa-edit')
  $('.edit-button-container > .accept-button').removeClass('btn-show');
}

// Remove the pulse animation so it doesn't fire when an element is made visible
function removePulseAnimation() {
  $('.' + PULSE_CLASS_ONE).removeClass(PULSE_CLASS_ONE);
  $('.' + PULSE_CLASS_TWO).removeClass(PULSE_CLASS_TWO);
}

// Tiger-stripe the visible rows
function tigerStripe() {
  var visible_counter = 0;
  $('.row-striped').each(function(index){
    if ($(this).is(":visible")) {
      if (visible_counter % 2 == 0){
        $(this).removeClass('row-even');
        $(this).addClass('row-odd');
      } else {
        $(this).removeClass('row-odd');
        $(this).addClass('row-even');
      }
      visible_counter++;
    }
  });
}

// Change the input field only if it does not have focus and none of its siblings
// have focus (e.g., so seconds don't change when minutes are being edited)
function changeIfNotFocused(field_name, new_value) {
  var obj = $('#' + field_name);
  var is_neighbor_focused = false;
  $(obj).siblings("input").each(function() {
    if ($(this).is(':focus')) {
      is_neighbor_focused = true;
    }
  });
  
  if (!$(obj).is(':focus') && !is_neighbor_focused) {
    if ($(obj).is(':checkbox')) {
      $(obj).prop('checked', new_value);
    } else {
      $(obj).val(new_value);
    }
  }  
}

// Toggle controls
function toggleUpdate(data, field_name, pretty_values, toggle_true_values) {
  var toggle_icon = '';
  var ready_value;
  var visible_value;
  var index = data[field_name];
  
  if (typeof(index) == 'boolean') {
    index = index ? 1 : 0;
  }
  
  if (toggle_true_values === undefined) {
    toggle_true_values = [1];
  }
  
  // Annoyingly, IE8 doesn't support indexOf()
  if ($.inArray(index, toggle_true_values) > -1) {
    visible_value = pretty_values[index];
    toggle_icon = '<i class="fa fa-toggle-on fa-lg"></i>';
    ready_value = 0;
  } 
  else {
    if (index == false) {
      index = 0;
    }
    visible_value = pretty_values[index];
    toggle_icon = '<i class="fa fa-toggle-off fa-lg"></i>';
    ready_value = 1;
  }

  $('#' + field_name + '_value').html(visible_value);
  $('#' + field_name + '_edit').val(ready_value);
  $('#' + field_name + '_change').html(toggle_icon);
}

// Time-formatted
function timeFieldUpdate (field_name, data) {
  var time_raw = data[field_name]
  var time_text = timeText(time_raw)
  var time_array = toHHMMSS(time_raw);
  $('#' + field_name + '_value').html(time_text);
  changeIfNotFocused(field_name + '_hours_edit', time_array[0]);
  changeIfNotFocused(field_name + '_minutes_edit', time_array[1]);
  changeIfNotFocused(field_name + '_seconds_edit', time_array[2]);
}
  

// Pulse the field value if it has changed in the latest data pull
function pulseIf(value_name, prev_json, new_json) {
  var field_name = '#' + value_name + '_value';
  if (prev_json && new_json[value_name] != prev_json[value_name]) {
    pulseText($(field_name));
  }
}

// Swap identical-but-differently-named CSS classes to ensure that pulse animation can
// happen again on the same element
function pulseText(obj) {  
  if ($(obj).hasClass(PULSE_CLASS_ONE)) {
    $(obj).removeClass(PULSE_CLASS_ONE);
    $(obj).addClass(PULSE_CLASS_TWO);
  } else {
    $(obj).addClass(PULSE_CLASS_ONE);
    $(obj).removeClass(PULSE_CLASS_TWO);
  }
}


////
// Time formatting helper -- from http://stackoverflow.com/questions/6312993/javascript-seconds-to-time-string-with-format-hhmmsshttp://stackoverflow.com/questions/6312993/javascript-seconds-to-time-string-with-format-hhmmss
function toHHMMSS(sec_num) {
    var hours   = Math.floor(sec_num / 3600);
    var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
    var seconds = sec_num - (hours * 3600) - (minutes * 60);

    if (hours   < 10) {hours   = "0"+hours;}
    if (minutes < 10) {minutes = "0"+minutes;}
    if (seconds < 10) {seconds = "0"+seconds;}

    var time = [hours, minutes, seconds];
    return time;
}

function timeText(sec_num) {
    var hms = toHHMMSS(sec_num);
    var hours   = hms[0];
    var minutes = hms[1];
    var seconds = hms[2];

    var time    = hours+':'+minutes+':'+seconds;
    return time;
}

////
// CSRF handling - From the django CSRF docs
function getCookie(name) {
    var cookieValue = null;
    if (document.cookie && document.cookie != '') {
        var cookies = document.cookie.split(';');
        for (var i = 0; i < cookies.length; i++) {
            var cookie = jQuery.trim(cookies[i]);
            // Does this cookie string begin with the name we want?
            if (cookie.substring(0, name.length + 1) == (name + '=')) {
                cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                break;
            }
        }
    }
    return cookieValue;
}
var csrftoken = getCookie('csrftoken');
function csrfSafeMethod(method) {
    // these HTTP methods do not require CSRF protection
    return (/^(GET|HEAD|OPTIONS|TRACE)$/.test(method));
}
$.ajaxSetup({
    beforeSend: function(xhr, settings) {
        if (!csrfSafeMethod(settings.type) && !this.crossDomain) {
            xhr.setRequestHeader("X-CSRFToken", csrftoken);
        }
    }
});

////
// For automatic notice handling
function showNotice(notice_text) {
  clearTimeout(NOTICE_TIMER);
  
  $('#status-text').html(notice_text);
  $('#status-footer').fadeIn();
  
  // Wait indefinitely for a pump reply
  if (!NOTICE_WAITING_FOR_PUMP) {
    NOTICE_TIMER = setTimeout(function(){
      hideNotice();
    }, NOTICE_HIDE_MS);  
  }
}

function hideNotice() {
  $('#status-footer').fadeOut();
}

// Find new input values based on the siblings of whichever
// "submit" button was clicked
function extractInputValue(obj) {
  var new_value = -1;
  var attr_name = $(obj).attr('name');
  if ($('#' + attr_name + '_settings').length > 0) {
    obj = $('#' + attr_name + '_settings');
  }
  
  if ($(obj).hasClass('time_edit')) {
    var hours = $(obj).siblings("input[name$='hours']").val();
    var minutes = $(obj).siblings("input[name$='minutes']").val();
    var seconds = $(obj).siblings("input[name$='seconds']").val()
    new_value = hours*3600 + minutes*60 + seconds*1;      
  } else if ($(obj).hasClass('select_edit')) {
    new_value = $(obj).siblings("select[name='new_value']").val();
  } else if ($(obj).hasClass('check_edit')) {
    new_value = $(obj).siblings("input[name='new_value']").prop('checked');
  } else {  
    new_value = $(obj).siblings("input[name='new_value']").val();
  }
  return new_value;
}

////
// Handle AJAX form submissions
function submitChange(obj, urlOverride) {
  NOTICE_WAITING_FOR_PUMP = true
  showNotice('Sending...');
  NOTICE_WAITING_FOR_PUMP = false
  
  var attr_name = $(obj).attr('name');
  var new_value = extractInputValue(obj);
     
  var submit_data = {attr_name: attr_name, 
                     new_value: new_value,
                     units: UNITS};
  
  if ($(obj).attr('name') == 'password') {
    var old_password_field = $('#old_password_edit');
    // Admins don't need to supply an old password, so they don't see the field
    if (old_password_field) {
      submit_data['old_password'] = old_password_field.val();
    }
    // Clear the password fields after they are submitted
    $('#password_edit').val('');
    $('#old_password_edit').val('');
  }   
  
  var url = AJAX_URL; 
  if (urlOverride) {
    url = urlOverride;
  }
  
  $.ajax({
    url: url,
    data: submit_data,
    timeout: CONNECTION_TIMEOUT,
    error: function(data) { 
        NOTICE_WAITING_FOR_PUMP = false;
        if (data.responseJSON['message']) {
          showNotice(data.responseJSON['message']);
        } else {
          showNotice('Error sending command.');
        } 
      },
    success: function(data) { 
        NOTICE_WAITING_FOR_PUMP = data['data']['waiting_for_pump'];
        REFRESH_COUNTER = 0;
        POLL_PERIOD_MS = 1;
        poll();
        showNotice(data['message']);
      },
    type: 'POST',
    });
  
  // Collapse all of the accordians
  collapseAccordian();
}

////
//Handle simple submissions
function submitSimple(obj, attr_name, new_value, urlOverride) {
  NOTICE_WAITING_FOR_PUMP = true
  showNotice('Sending...');
  NOTICE_WAITING_FOR_PUMP = false
  
  var submit_data = {attr_name: attr_name, 
                    new_value: new_value,
                    units: UNITS};
    
  var url = AJAX_URL; 
  if (urlOverride) {
    url = urlOverride;
  }
  
  $.ajax({
    url: url,
    data: submit_data,
    timeout: CONNECTION_TIMEOUT,
    error: function(data) { 
        NOTICE_WAITING_FOR_PUMP = false;
        if (data.responseJSON && data.responseJSON.message) {
          showNotice(data.responseJSON.message);
        } else {
          showNotice('Error sending command.');
        } 
        if (typeof simpleSubmitError == 'function') {
          simpleSubmitError(data, obj);
        }
    },
    success: function(data) { 
      if (data.data) {
        NOTICE_WAITING_FOR_PUMP = data.data.waiting_for_pump;
      }
      REFRESH_COUNTER = 0;
      POLL_PERIOD_MS = 1;
      if (typeof simpleSubmitSuccess == 'function') {
        simpleSubmitSuccess(data, obj);
      }
      showNotice(data.message);
    },
    type: 'POST',
  });
}

// For creation of new objects
function submitCreate(obj, urlOverride) {
  // Prevent double-clicks
  if ($(obj).hasClass('disabled')) {
    return;
  }
  $(obj).addClass('disabled');

  NOTICE_WAITING_FOR_PUMP = true
  showNotice('Sending...');
  NOTICE_WAITING_FOR_PUMP = false  
  
  // Loop through every input on the page
  var submit_data = {};
  var attr_name = '';
  var attr_value = '';
  $('.ajax_field').each(function(){
    attr_name = $(this).attr("name");
    attr_value = extractInputValue(this);
    submit_data[attr_name] = attr_value;
  });
    
  $('.ajax_supplemental').each(function(){  
    attr_name = $(this).attr("name");
    attr_value = $(this).val();
    submit_data[attr_name] = attr_value;
  });
    
  var url = AJAX_URL; 
  if (urlOverride) {
    url = urlOverride;
  }
  
  $.ajax({
    url: url,
    data: submit_data,
    timeout: CONNECTION_TIMEOUT,
    error: function(data) { 
        NOTICE_WAITING_FOR_PUMP = false;
        if (data.status == 500) {
          showNotice('Error: server error');
        } else if (data.status == 403) {
          showNotice('Error: CSRF token invalid');
        } else if (data.responseJSON['message']) {
          showNotice(data.responseJSON['message']);
        } else {
          showNotice('Error: creation failed.');
        }
        $(obj).removeClass('disabled'); 
      },
    success: function(data) { 
        showNotice(data['message']);
        if (typeof successfulCreate == 'function') {
          successfulCreate(data);
        }
      },
    type: 'PUT',
    });
}

function submitDelete(obj) {
  if (!obj) {
    obj = $('#delete-button');
  }
  
  // Prevent double-clicks
  if ($(obj).hasClass('disabled')) {
    return;
  }
  $(obj).addClass('disabled');

  NOTICE_WAITING_FOR_PUMP = true
  showNotice('Deleting...');
  NOTICE_WAITING_FOR_PUMP = false  
      
  var url = AJAX_URL; 
  
  $.ajax({
    url: url,
    timeout: CONNECTION_TIMEOUT,
    error: function(data) { 
        NOTICE_WAITING_FOR_PUMP = false;
        if (data.status == 500) {
          showNotice('Error: server error');
        } else if (data.responseJSON['message']) {
          showNotice(data.responseJSON['message']);
        } else {
          showNotice('Error: delete failed.');
        }
        $(obj).removeClass('disabled'); 
      },
    success: function(data) { 
        showNotice(data['message']);
        if (typeof successfulDelete == 'function') {
          successfulDelete(data);
        }
      },
    type: 'DELETE',
    });
    
}


// Operate the toggle switch when not being updated by AJAX data (e.g., on a user-creation page)
function toggleClick(obj) {
  var valueObj = $(obj).siblings("input[name$='new_value']")
  var iconObj = $(obj).children("i");
  if (valueObj.val() == '0') {
    valueObj.val('1');    
    iconObj.removeClass("fa-toggle-off");
    iconObj.addClass("fa-toggle-on");
  } else {
    valueObj.val('0');
    iconObj.removeClass("fa-toggle-on");
    iconObj.addClass("fa-toggle-off");
  }
}

function updatePollCounter() {
  
  REFRESH_COUNTER++;
  
  // Poll more frequently after an action, because we're probably expecting an update
  if (REFRESH_COUNTER >= POLL_COUNT_FOR_LONG){        
    POLL_PERIOD_MS = POLL_PERIOD_LONG_SEC*1000;
  } else {
    POLL_PERIOD_MS = POLL_PERIOD_SHORT_SEC*1000;
  }
  
}

// AJAX data polling (inbound)
function poll(){
   clearTimeout(POLL_TIMER);
   POLL_TIMER = setTimeout(function(){
      $.ajax({ 
      url: AJAX_URL + SORT_BY,
      timeout: CONNECTION_TIMEOUT, 
      beforeSend: function(request) {
        request.setRequestHeader('If-None-Match', LAST_ETAG);
      },
      success: function(data, textStatus, xhr) {
        try {
          if (xhr.status != 304) {
            LAST_ETAG = xhr.getResponseHeader('ETag');
            populateData(data);
          }
          updatePollCounter();
        }
        catch(err) {
          showNotice('Exception: ' + err);
          console.log("Exception caught: " + err);
          POLL_PERIOD_MS = POLL_PERIOD_SHORT_SEC*1000;  
        }
      }, 
      dataType: "json", 
      error: function(data, textStatus, errorThrown) {
        if (data.status == 401) {
          // Logged out and need to log in again
          showNotice('Please log in again');
          window.location.href = "/";
        } else if (data.status == 404) {
          // Possibly a deleted pump
          window.location.href = "/";
        } else {
          if (data.responseJSON && data.responseJSON.message && data.responseJSON.message.length > 0) {
            textStatus = data.responseJSON.message;
          }
          else if (textStatus == 'error') {
            textStatus = 'network problem';
          }
          else {
            textStatus = 'Bad request';
          }
          showNotice('Error: ' + textStatus);
          POLL_PERIOD_MS = POLL_PERIOD_LONG_SEC*1000;
        }
      },
      complete: poll });
  }, POLL_PERIOD_MS);
}

// onLoad setup common to all pages
function pageUniversalSetup(){

  // Indicate that the javascript has loaded
  $('#loading_mask').html('<p>Loading data...</p>');

  connectButtons(BUTTON_LIST);
  
  // Page-specific setup
  if (typeof pageSetup == 'function') {
    pageSetup();
  }

  $('.ajax_submit').attr('onClick', 'submitChange(this); return false;');
  
  // Note that polling setup (i.e., a call to poll()) is not done here 
  // since not all pages have AJAX updates
}

$(window).load(pageUniversalSetup);

// Populate the page with the AJAX data
function populateData(data) {
  // Allow cached redraws on resize
  if (!data && PREVIOUS_JSON) {
    data = PREVIOUS_JSON;
  }

  // Make sure we're getting back something that looks plausible
  if (!('version' in data) ||
      !('data' in data))
  {
    showNotice('No valid data received');
    return;
  }
  
  // We want to be able to force a reload of the page if a newer version becomes available
  if (LAST_VERSION && LAST_VERSION != data.version) {
    location.reload(true);
  }

  if ('flash' in data) {
    if ('safeFlash' in data) {
      showFlash(data.flash, true);  
    }
    else
    {
      showFlash(data.flash);
    }
    
  }
  else {
    hideFlash();
  }
  
  $('#loading_mask').hide();
  $('.details-container').show();  
  
  pageSpecificPopulate(data.data);
  
  PREVIOUS_JSON = data;
  LAST_VERSION = data.version;
}
