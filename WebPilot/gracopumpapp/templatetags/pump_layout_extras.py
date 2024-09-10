from datetime import date
from datetime import date
import re
from time import strftime

from django import template
from django.utils.safestring import mark_safe
from gracopumpapp.models import UserProfile


register = template.Library()


class Templates(object):
    '''Templates for page generation'''
    ROW_TEMPLATE = '''
<div class="row row-generic %ROW_TYPE_CLASS%" id="%FIELD_NAME%_row_outer">
  <div class="col-xs-12">
    <div class="row-inner-container">
      <div class="row-table-row">
        <form class="inline-element">
          <div class="row-left-content">
            <div class="row">
              <div class="col-sm-4"><span class="row-name">%FIELD_NAME_PRETTY%</span></div>
              <div class="col-sm-8">
                <div id="%FIELD_NAME%_read_container" class="read-container">
                  <div class="list-table-cell-container"><div class="list-table-cell-content"><span id="%FIELD_NAME%_value">%FIELD_DEFAULT_VALUE%</span>%FIELD_NAME_UNITS%</div><div class="list-table-cell-spacer"></div></div>
                </div>
                <div id="%FIELD_NAME%_write_container" class="write-container %ROW_TYPE_CLASS%_write">
                  %EDIT_ROW%
                </div>
              </div>
            </div>
          </div>
          <div class="edit-button-container row-right-content %ROW_TYPE_CLASS%_right">
            %EDIT_BUTTON%
          </div>
        </form>
      </div>
    </div>
  </div>
</div>
'''

    ROW_DIVIDER = '''
<div class="row row-generic row-divider">
</div>
'''

    ROW_DISPLAY = '''
<div class="row row-generic" id="%FIELD_NAME%_row_outer">
  <div class="col-xs-12">
    <div class="row-inner-container">
      <div class="row-table-row">
        <form class="inline-element">
          <div class="row-left-content">
            <div class="row">
              <div class="col-sm-4"><span class="row-name">%FIELD_NAME_PRETTY%</span></div>
              <div class="col-sm-8">
                %TEXT%
              </div>
            </div>
          </div>
          <div class="edit-button-container row-right-content">
            &nbsp;
          </div>
        </form>
      </div>
    </div>
  </div>
</div>
'''

    ROW_DESCRIPTION = '''
<div class="row row-generic">
    <p>%TEXT%</p>
</div>
'''

    HIDDEN_VALUE_TEMPLATE = '''
<input type="hidden" %NAME_ATTR% value="%FIELD_DEFAULT_VALUE%" />
'''

    EDIT_BUTTON_TEMPLATE = '''
<button type="button" class="btn btn-default btn-sm pull-right pump-details-button not-for-create cancel-button" id="%FIELD_NAME%_edit_button">
  <i class="fa fa-lg %BUTTON_ICON%"></i>
</button>
<button type="submit" class="btn btn-default btn-sm pull-right pump-details-button ajax_submit not-for-create accept-button" name="%FIELD_NAME%" id="%FIELD_NAME%_submit">
  <i class="fa fa-check fa-lg green"></i>
</button>
'''
    EDIT_BUTTON_ICON_DEFAULT = 'fa-edit'

    RESET_BUTTON_TEMPLATE = '''
<div id="%FIELD_NAME%_reset_container" class="inline-element">
    <input type="hidden" %NAME_ATTR% value="0">
    <button type="submit" class="btn btn-default btn-sm pull-right pump-details-button ajax_submit" name="%FIELD_NAME%" >
      <i class="fa %BUTTON_ICON% fa-lg"></i>
    </button>
</div>
'''

    RESET_BUTTON_ICON_DEFAULT = 'fa-history'

    HYPERLINK_BUTTON_TEMPLATE = '''
<a href="%HYPERLINK_URL%" role="button" class="btn btn-default btn-sm pull-right pump-details-button">
    <i class="fa %BUTTON_ICON% fa-lg"></i>
</a>
'''
    HYPERLINK_BUTTON_ICON_DEFAULT = 'fa-external-link'

    TOGGLE_BUTTON_TEMPLATE = '''
  <button type="submit" id="%FIELD_NAME%_change" class="btn btn-default btn-sm pull-right pump-details-button ajax_submit toggle-button" name="%FIELD_NAME%">
    <i class="fa %BUTTON_ICON% fa-lg"></i>
  </button>
  <input type="hidden" %NAME_ATTR% id="%FIELD_NAME%_edit" value="%TOGGLE_VALUE_DEFAULT%" >
'''
    TOGGLE_BUTTON_ICON_ON = 'fa-toggle-on'
    TOGGLE_BUTTON_ICON_OFF = 'fa-toggle-off'

    TEXT_EDIT_TEMPLATE = '''
      <input type="text" class="text-edit-field %EXTRA_CSS_CLASS%" %NAME_ATTR%  %MAX_LENGTH_ATTR% size="%TEXT_EDIT_SIZE%" id="%FIELD_NAME%_edit" value="" placeholder="%FIELD_NAME_PRETTY%"/>%FIELD_NAME_UNITS_EDIT%
      <input type="hidden" class="" id="%FIELD_NAME%_settings" />
'''
    TEXT_EDIT_SIZE_DEFAULT = '15'

    LOCATION_EDIT_TEMPLATE = '''
      <input type="text" class="text-edit-field" %NAME_ATTR% size=12 id="%FIELD_NAME%_edit" value="" placeholder="%FIELD_NAME_PRETTY%"/>%FIELD_NAME_UNITS_EDIT%
      <input type="submit" class="ok-button" id="%FIELD_NAME%_mark_loc_btn" value="Get location" />
      <input type="hidden" class="" id="%FIELD_NAME%_settings" />
'''

    PASSWORD_EDIT_TEMPLATE = '''
      <p>%PASSWSORD_REQ_MSG%.</p>
      %PASSWORD_OLD%
      <input type="password" %NAME_ATTR% size=12 class="space-below-field" id="%FIELD_NAME%_edit" value="" placeholder="New password" />
      <input type="hidden" class="password_edit" id="%FIELD_NAME%_settings" />
'''
    PASSWORD_OLD_ROW_TEMPLATE = '''
      <input type="password" name="old_password" size=12 class="space-below-field" id="old_password_edit" value="" placeholder="Old password" /><br/>
'''

    TIME_EDIT_TEMPLATE = '''
      <input type="text" name="hours" size=2 id="%FIELD_NAME%_hours_edit" value="" />:
      <input type="text" name="minutes" size=2 id="%FIELD_NAME%_minutes_edit" value="" />:
      <input type="text" name="seconds" size=2 id="%FIELD_NAME%_seconds_edit" value="" />

      <input type="hidden" class="time_edit" id="%FIELD_NAME%_settings" />
'''

    EXPIRATION_DATE_EDIT_TEMPLATE = '''
      <select %NAME_ATTR% name="month" id="%FIELD_NAME%_month_combo" class="detail-dropdown" >%MONTH_DROPDOWN_OPTIONS%</select> / 
      <select %NAME_ATTR% name="year" id="%FIELD_NAME%_year_combo" class="detail-dropdown" >%YEAR_DROPDOWN_OPTIONS%</select>

      <input type="hidden" class="expiration_edit" id="%FIELD_NAME%_settings" />
'''

    DROPDOWN_EDIT_TEMPLATE = '''
        <select %NAME_ATTR% id="%FIELD_NAME%_combo" class="detail-dropdown" >
          %DROPDOWN_OPTIONS%
        </select> %FIELD_NAME_UNITS_EDIT%
        <input type="hidden" class="select_edit" id="%FIELD_NAME%_settings" />
'''
    DROPDOWN_OPTION_TEMPLATE = '''
<option value="%DROPDOWN_VALUE%" %DROPDOWN_SELECTED%>%DROPDOWN_NAME%</option>
'''

    UNITS_TEMPLATE = '''
<span class="%UNIT_NAME%_value"></span>
'''

    DELETE_BUTTON_TEMPLATE = '''
<div class="float-left">
    <button type="button" id="delete-button" class="btn btn-danger" data-toggle="modal" data-target="#deleteModal">Delete this %OBJECT_TYPE%</button>
</div>
'''
    GENERIC_BUTTON_TEMPLATE = '''
<div class="float-%BUTTON_SIDE%">
    <button type="button" id="generic_button_%BUTTON_NAME%" class="btn btn-%BUTTON_STYLE%" data-toggle="modal" data-target="#%MODAL_ID%">%GENERIC_BUTTON_LABEL%</button>
</div>
'''

    MODAL_DELETE_DIALOG_TEMPLATE = '''
    <div id="deleteModal" class="modal fade" role="dialog">
        <div class="modal-dialog">
            <div class="modal-content">
                <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal">&times;</button>
                    <h4 class="modal-title">Deletion confirmation</h4>
                </div>
                <div class="modal-body">
                    <p>Are you sure you want to delete this %OBJECT_TYPE%? This cannot be undone.</p>
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-default" id="modal_delete_button" data-dismiss="modal" onClick="%DELETE_FUNCTION%;return false;">Delete</button>
                    <button type="button" class="btn btn-primary" data-dismiss="modal">Cancel</button>
                </div>
            </div>
        </div>
    </div>
'''

    MODAL_REMOVE_DIALOG_TEMPLATE = '''
    <div id="removeModal" class="modal fade" role="dialog">
        <div class="modal-dialog">
            <div class="modal-content">
                <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal">&times;</button>
                    <h4 class="modal-title">Removal confirmation</h4>
                </div>
                <div class="modal-body">
                    <p>Are you sure you want to remove this %OBJECT_TYPE% from this group? You'll need to re-add this %OBJECT_TYPE% if you change your mind later.</p>
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-default" id="modal_remove_button" data-dismiss="modal" onClick="%FUNCTION%;return false;">Yes</button>
                    <button type="button" class="btn btn-primary" data-dismiss="modal">No</button>
                </div>
            </div>
        </div>
    </div>
'''

    MODAL_GENERIC_DIALOG_TEMPLATE = '''
    <div id="%MODAL_NAME%" class="modal fade" role="dialog">
        <div class="modal-dialog">
            <div class="modal-content">
                <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal">&times;</button>
                    <h4 class="modal-title">%MODAL_TITLE%</h4>
                </div>
                <div class="modal-body">
                    <p>%MODAL_QUESTION%</p>
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-default" id="%MODAL_NAME%_button" data-dismiss="modal" onClick="%FUNCTION%;return false;">%MODAL_AFFIRM_BUTTON%</button>
                    <button type="button" class="btn btn-primary" data-dismiss="modal">%MODAL_CANCEL_BUTTON%</button>
                </div>
            </div>
        </div>
    </div>
'''


def button_render(context, rendered_in, button_type=None, edit_type=None, *args, **kwargs):
    '''Render the edit buttons'''

    button_default_icon = ''
    rendered = rendered_in

    if 'reset' == button_type:
        rendered = re.sub('%EDIT_BUTTON%', Templates.RESET_BUTTON_TEMPLATE, rendered)
        button_default_icon = Templates.RESET_BUTTON_ICON_DEFAULT
    elif 'hyperlink' == button_type:
        hyperlink_url = '#'
        if 'hyperlink_url' in kwargs:
            if kwargs['hyperlink_url'] in context:
                hyperlink_url = context[kwargs['hyperlink_url']]
            else:
                hyperlink_url = kwargs['hyperlink_url']

        rendered = re.sub('%EDIT_BUTTON%', Templates.HYPERLINK_BUTTON_TEMPLATE, rendered)
        rendered = re.sub('%HYPERLINK_URL%', hyperlink_url, rendered)
        button_default_icon = Templates.HYPERLINK_BUTTON_ICON_DEFAULT
    elif 'toggle' == button_type:
        rendered = re.sub('%EDIT_BUTTON%', Templates.TOGGLE_BUTTON_TEMPLATE, rendered)
        if 'default' in kwargs:
            default_value = str(kwargs['default'])
        else:
            default_value = '0'
        rendered = re.sub('%TOGGLE_VALUE_DEFAULT%', default_value, rendered)
        if default_value == '0':
            button_default_icon = Templates.TOGGLE_BUTTON_ICON_OFF
        else:
            button_default_icon = Templates.TOGGLE_BUTTON_ICON_ON
    elif edit_type:
        rendered = re.sub('%EDIT_BUTTON%', Templates.EDIT_BUTTON_TEMPLATE, rendered)
        button_default_icon = Templates.EDIT_BUTTON_ICON_DEFAULT
    else:
        rendered = re.sub('%EDIT_BUTTON%', '', rendered)

    # The button is an image button
    if 'button_icon' in kwargs:
        button_icon = kwargs['button_icon']
    else:
        button_icon = button_default_icon
    rendered = re.sub('%BUTTON_ICON%', button_icon, rendered)

    return rendered


def edit_render(context, rendered_in, edit_type=None, *args, **kwargs):
    '''Render the various types of edit fields'''

    rendered = rendered_in

    if 'text' == edit_type:
        rendered = re.sub('%EDIT_ROW%', Templates.TEXT_EDIT_TEMPLATE, rendered)
        if 'size' in kwargs:

            rendered = re.sub('%TEXT_EDIT_SIZE%', str(kwargs['size']), rendered)
            rendered = re.sub('%MAX_LENGTH_ATTR%', 'maxlength="%s"' % str(kwargs['size']), rendered)
        else:
            rendered = re.sub('%TEXT_EDIT_SIZE%', Templates.TEXT_EDIT_SIZE_DEFAULT, rendered)
            rendered = re.sub('%MAX_LENGTH_ATTR%', '', rendered)
    elif 'location' == edit_type:
        rendered = re.sub('%EDIT_ROW%', Templates.LOCATION_EDIT_TEMPLATE, rendered)
    elif 'password' == edit_type:
        rendered = re.sub('%EDIT_ROW%', Templates.PASSWORD_EDIT_TEMPLATE, rendered)
        rendered = re.sub('%PASSWSORD_REQ_MSG%', UserProfile.PASSWORD_REQUIREMENT_MESSAGE, rendered)
        # Admins don't need to enter the old password
        if 'is_admin' in context and context['is_admin']:
            rendered = re.sub('%PASSWORD_OLD%', '', rendered)
        else:
            if 'mode' in kwargs and kwargs['mode'] == 'create':
                rendered = re.sub('%PASSWORD_OLD%', '', rendered)
            else:
                rendered = re.sub('%PASSWORD_OLD%', Templates.PASSWORD_OLD_ROW_TEMPLATE, rendered)
    elif 'time' == edit_type:
        rendered = re.sub('%EDIT_ROW%', Templates.TIME_EDIT_TEMPLATE, rendered)
    elif 'dropdown' == edit_type:
        # generate list of options from (key,name) tuples

        if 'dropdown_options_name' in kwargs and kwargs['dropdown_options_name'] in context:
            option_list = ''
            # List options are in context
            dropdown_options = context[kwargs['dropdown_options_name']]
            selected_value = kwargs.get('dropdown_selected', None)
            for dropdown_option in dropdown_options:
                if type(dropdown_option) == str or type(dropdown_option) == int:
                    option_value = str(dropdown_option)
                    option_name = option_value
                else:
                    option_value = str(dropdown_option[0])
                    option_name = str(dropdown_option[1])
                option_entry = Templates.DROPDOWN_OPTION_TEMPLATE
                option_entry = re.sub('%DROPDOWN_VALUE%', option_value, option_entry)
                option_entry = re.sub('%DROPDOWN_NAME%', option_name, option_entry)
                if option_value == str(selected_value):
                    option_entry = re.sub('%DROPDOWN_SELECTED%', 'selected="selected"', option_entry)
                else:
                    option_entry = re.sub('%DROPDOWN_SELECTED%', '', option_entry)
                option_list += option_entry
            rendered = re.sub('%EDIT_ROW%', Templates.DROPDOWN_EDIT_TEMPLATE, rendered)
            rendered = re.sub('%DROPDOWN_OPTIONS%', option_list, rendered)
        else:
            rendered = re.sub('%EDIT_ROW%', '', rendered)
    elif 'expiration_date' == edit_type:
        # Fill month options
        month_options = ''
        for month in range(1, 13):
            option_entry = Templates.DROPDOWN_OPTION_TEMPLATE
            option_entry = re.sub('%DROPDOWN_VALUE%', str(month), option_entry)
            option_entry = re.sub('%DROPDOWN_NAME%', date(2000, month, 15).strftime('%b -  %m'), option_entry)
            month_options += option_entry

        # Fill year options
        year_options = ''
        current_year = date.today().year
        for year_offset in range(0, 26):
            year = '%u' % (current_year + year_offset)
            option_entry = Templates.DROPDOWN_OPTION_TEMPLATE
            option_entry = re.sub('%DROPDOWN_VALUE%', year, option_entry)
            option_entry = re.sub('%DROPDOWN_NAME%', year, option_entry)
            year_options += option_entry

        rendered = re.sub('%EDIT_ROW%', Templates.EXPIRATION_DATE_EDIT_TEMPLATE, rendered)
        rendered = re.sub('%MONTH_DROPDOWN_OPTIONS%', month_options, rendered)
        rendered = re.sub('%YEAR_DROPDOWN_OPTIONS%', year_options, rendered)
    else:
        rendered = re.sub('%EDIT_ROW%', '', rendered)

    return rendered


def pretty_name_render(field_name, rendered_in, pretty_name=None, *args, **kwargs):
    '''Render the field name with spaces and proper capitalization'''

    rendered = rendered_in

    if pretty_name:
        rendered = re.sub('%FIELD_NAME_PRETTY%', pretty_name, rendered)
    else:
        rendered = re.sub('%FIELD_NAME_PRETTY%', ' '.join(field_name.split('_')).capitalize(), rendered)

    return rendered


def units_render(rendered_in, *args, **kwargs):
    '''Render units for field values'''

    rendered = rendered_in

    if 'units' in kwargs:
        # Support ratio units
        units = kwargs['units'].split(',')
        units_text = ' '  # Space for sep
        units_text += '/'.join([re.sub('%UNIT_NAME%', unit, Templates.UNITS_TEMPLATE) for unit in units])
        rendered = re.sub('%FIELD_NAME_UNITS%', units_text, rendered)

        # Allow the units to be hidden in edit mode
        if 'units_in_edit' in kwargs:
            if kwargs['units_in_edit']:
                rendered = re.sub('%FIELD_NAME_UNITS_EDIT%', units_text, rendered)
            else:
                rendered = re.sub('%FIELD_NAME_UNITS_EDIT%', '', rendered)
        else:
            rendered = re.sub('%FIELD_NAME_UNITS_EDIT%', units_text, rendered)
    else:
        rendered = re.sub('%FIELD_NAME_UNITS%', '', rendered)
        rendered = re.sub('%FIELD_NAME_UNITS_EDIT%', '', rendered)

    return rendered


def default_value_render(rendered_in, *args, **kwargs):
    '''Render the default value for the field'''

    rendered = rendered_in

    if 'default_value' in kwargs:
        default_value = str(kwargs['default_value'])
    else:
        default_value = ''

    rendered = re.sub('%FIELD_DEFAULT_VALUE%', default_value, rendered)

    return rendered


def cleanup(rendered_in, *args, **kwargs):
    rendered = rendered_in

    extra_class = kwargs.get('extra_class', '')
    rendered = re.sub('%EXTRA_CSS_CLASS%', extra_class, rendered)

    rendered = re.sub('%NAME_ATTR%', 'name="new_value"', rendered)

    return rendered


@register.simple_tag(takes_context=True)
def row_standard(context, field_name, pretty_name=None, button_type=None, edit_type=None, *args, **kwargs):

    rendered = Templates.ROW_TEMPLATE

    rendered = button_render(context, rendered, button_type, edit_type, *args, **kwargs)
    rendered = edit_render(context, rendered, edit_type, *args, **kwargs)
    rendered = pretty_name_render(field_name, rendered, pretty_name, *args, **kwargs)
    rendered = units_render(rendered, *args, **kwargs)
    rendered = default_value_render(rendered, *args, **kwargs)
    rendered = cleanup(rendered, *args, **kwargs)

    rendered = re.sub('%FIELD_NAME%', field_name, rendered)

    if edit_type:
        rendered = re.sub('%ROW_TYPE_CLASS%', 'edit_row_%s' % edit_type, rendered)
    else:
        rendered = re.sub('%ROW_TYPE_CLASS%', '', rendered)

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def row_divider(context, *args, **kwargs):
    rendered = Templates.ROW_DIVIDER

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def row_description(context, text, *args, **kwargs):
    rendered = Templates.ROW_DESCRIPTION

    rendered = re.sub('%TEXT%', text, rendered)

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def row_display(context, field_name, text, pretty_name=None, *args, **kwargs):
    rendered = Templates.ROW_DISPLAY

    rendered = pretty_name_render(field_name, rendered, pretty_name)
    rendered = re.sub('%FIELD_NAME%', field_name, rendered)
    rendered = re.sub('%TEXT%', text, rendered)

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def delete_button(context, object_type_name, *args, **kwargs):
    '''Generate a button to delete the current object'''
    rendered = Templates.DELETE_BUTTON_TEMPLATE

    rendered = re.sub('%OBJECT_TYPE%', object_type_name, rendered)

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def generic_button(context, button_label, modal_id, *args, **kwargs):
    '''Generate a button to delete the current object'''
    rendered = Templates.GENERIC_BUTTON_TEMPLATE

    side = 'left'
    if 'side' in kwargs:
        side = kwargs['side']

    style = 'danger'
    if 'style' in kwargs:
        style = kwargs['style']

    rendered = re.sub('%BUTTON_NAME%', modal_id, rendered)
    rendered = re.sub('%BUTTON_SIDE%', side, rendered)
    rendered = re.sub('%BUTTON_STYLE%', style, rendered)
    rendered = re.sub('%MODAL_ID%', modal_id, rendered)
    rendered = re.sub('%GENERIC_BUTTON_LABEL%', button_label, rendered)

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def modal_delete_dialog(context, object_type_name, delete_function='submitDelete()', *args, **kwargs):
    rendered = Templates.MODAL_DELETE_DIALOG_TEMPLATE

    rendered = re.sub('%OBJECT_TYPE%', object_type_name, rendered)
    rendered = re.sub('%DELETE_FUNCTION%', delete_function, rendered)

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def modal_remove_dialog(context, object_type_name, function, *args, **kwargs):
    rendered = Templates.MODAL_REMOVE_DIALOG_TEMPLATE

    rendered = re.sub('%OBJECT_TYPE%', object_type_name, rendered)
    rendered = re.sub('%FUNCTION%', function, rendered)

    return mark_safe(rendered)


@register.simple_tag(takes_context=True)
def modal_generic_dialog(context, name, title, question, affirm_button, cancel_button, function):
    rendered = Templates.MODAL_GENERIC_DIALOG_TEMPLATE

    rendered = re.sub('%MODAL_NAME%', name, rendered)
    rendered = re.sub('%MODAL_TITLE%', title, rendered)
    rendered = re.sub('%MODAL_QUESTION%', question, rendered)
    rendered = re.sub('%MODAL_AFFIRM_BUTTON%', affirm_button, rendered)
    rendered = re.sub('%MODAL_CANCEL_BUTTON%', cancel_button, rendered)
    rendered = re.sub('%FUNCTION%', function, rendered)

    return mark_safe(rendered)
