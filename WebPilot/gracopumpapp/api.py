from datetime import datetime
import ipaddress
import re
import traceback

from dateutil.relativedelta import relativedelta
from django.contrib import messages
from django.contrib.auth import authenticate, login, logout, \
    update_session_auth_hash
from django.contrib.auth.decorators import login_required
from django.contrib.auth.models import User
from django.forms.models import model_to_dict
from django.http.response import HttpResponse, Http404
from django.urls.base import reverse
from django.utils import timezone
from django.views.decorators.csrf import ensure_csrf_cookie
from gracopumpapp.aeris import aeris_get_status, aeris_clear_registration
from gracopumpapp.decorators import really_defeat_caching
from gracopumpapp.models import Pump, Log, History, Customer, UserProfile, \
    create_graco_user, UnitsOfMeasure, Notification, get_none_customer, \
    TermsOfService, Invitation, AlarmCustomization, Subscription, \
    Plan, sanitize, AlarmPreference, SitePreference, PointCustomization
from gracopumpapp.payments import Payments
from gracopumpapp.pumps import PumpSet, PrettyPump, alarm_bitfield_to_list, \
    Bounds, AlarmList
from gracopumpapp.render_helpers import respond_not_logged_in, respond_not_authorized, \
    respond_error, respond, log_command, get_pump_if_authorized, \
    get_customer_if_authorized
import pytz
from recurly.errors import NotFoundError, ValidationError


LOW = 0
HIGH = 1

'''
Helpers
'''


def soft_delete_pump(pump):
    '''
    Deactivate (similar to a soft-delete) a pump, such as when the pump's group
    is deleted or a group manager or admin "deletes" the pump.

    This method takes care of housekeeping that previously had been scattered
    in other places in the code.
    '''
    if not isinstance(pump, Pump):
        raise TypeError('Invalid pump object')

    pump.is_active = False
    pump.suspended = False

    old_customer = pump.customer
    pump.customer = Customer.objects.get(pk=get_none_customer())

    pump.pretty_name = ''

    pump.save()

    try:
        if old_customer.has_valid_subscription():
            Payments.recalculate(old_customer.get_current_subscription())
    except Exception as e:
        print('Exception %s recomputing payments while deleting pump %s' % (repr(e), pump.unique_id))

    try:
        PumpSet.set_pump(pump.unique_id, 'SetPumpName', pump.pretty_name, retain=True)
    except Exception as e:
        print('Exception %s publishing blank name while deleting pump %s' % (repr(e), pump.unique_id))

    try:
        # Force the deleted pump to disconnect to ensure everything stays in sync
        PumpSet.disconnect([pump, ])
    except Exception as e:
        print('Exception %s force-disconnecting pump while deleting pump %s' % (repr(e), pump.unique_id))


'''
API 1.0
'''


@ensure_csrf_cookie
def csrf_api_1_0(request):
    '''Easy way to get the CSRF token from a cold start'''
    return respond(request)


def login_api_1_0(request):
    '''Log in'''
    message = ''
    username = None
    not_active = False
    bad_pw = False

    if 'username' in request.POST and 'password' in request.POST:
        username = request.POST['username']
        password = request.POST['password']

        # For legacy username support (before we logged in with email addresses)
        user = authenticate(username=username, password=password)

        if user is None:
            # Try using the username as an email address, but only if there are no
            # email address collisions
            if User.objects.filter(email__iexact=username).count() == 1:
                user_possible = User.objects.filter(email__iexact=username).first()
                user = authenticate(username=user_possible.username, password=password)

        # We got a user and authenticated using either the current or legacy methods
        if user is not None:
            if user.is_active:
                login(request, user)
                timezone.activate(pytz.timezone(user.userprofile.time_zone))
            else:
                not_active = True
        else:
            bad_pw = True

    if request.user.is_authenticated():
        message = 'Logged in'
        status = 200
    else:
        attribute = ''
        new_value = ''
        log_message = ''

        if username:
            attribute = 'username'
            new_value += re.sub(UserProfile.USERNAME_REGEX, '', username)

        if bad_pw:
            log_message = 'Wrong u or p (API)'
        elif not_active:
            log_message = 'Not active (API)'

        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_LOGIN,
                                       action=Log.ACTION_LOGIN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_USER,
                                       attribute=sanitize(attribute),
                                       new_value=sanitize(new_value),
                                       )
        log_entry.save()

        message = 'Bad username or password'
        status = 401

    return respond(request, status, message)


@login_required
def logout_api_1_0(request):
    '''Log out the current user'''
    status = 400
    message = 'Could not log out'
    try:
        if request.method == 'POST':
            logout(request)
            message = 'Logged out'
            status = 200
    except:
        pass
    return respond(request, status, message)


@really_defeat_caching
@login_required
def admin_api_1_0(request):
    '''The admin api'''
    if not request.user.is_authenticated():
        return respond_not_logged_in(request)

    if not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    if request.method == 'POST':
        if request.POST['attr_name'] == 'force_pump_disconnect':
            return admin_helper_force_pump_disconnect(request)
        if request.POST['attr_name'] == 'force_payments_sync':
            return admin_helper_force_payments_sync(request)

    return respond_error(request, status=400, message='Bad command or target')


def admin_helper_force_pump_disconnect(request):
    '''Force pump disconnects'''
    response_dict = {}

    try:
        pumps = Pump.objects.filter(is_active=True).all()
        if PumpSet.disconnect(pumps):
            status = 200
            message = 'Disconnections successful'
        else:
            status = 400
            message = 'Disconnections failed'
    except:
        status = 500
        message = 'Error during disconnection'

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.user_actor = request.user
    log_entry.success = Log.STATUS_SUCCESS if status == 200 else Log.STATUS_FAIL
    log_entry.message = 'Disconnect all pumps'
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.action = Log.ACTION_NA
    log_entry.save()

    return respond(request, status, message, response_dict)


def admin_helper_force_payments_sync(request):
    '''Force synchronization of the local DB with the authoritative Recurly DB'''
    response_dict = {}

    try:
        Payments.sync_plans()
        status = 200
        message = 'Sync successful'
    except Exception as e:
        status = 500
        message = 'Error during sync'
        print('%s: %s' % (message, repr(e)))

    return respond(request, status, message, response_dict)


@really_defeat_caching
def events_api_1_0(request):
    if not request.user.is_authenticated() and request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    if request.method == 'GET':
        tz = pytz.timezone(request.user.userprofile.time_zone)

        all_events_raw = Log.fetch_filtered(request, paginate=True)

        all_events_rev = sorted(list(all_events_raw), key=lambda x: int(x.pk))
        all_events_list = []
        for evt in all_events_rev:
            evt.convert_old_version_to_new()
            evt_dict = model_to_dict(evt)
            evt_dict['timestamp'] = evt.timestamp.astimezone(tz).strftime('%Y-%m-%d %X %Z')
            evt_dict['username'] = ''
            if evt.user_actor:
                evt_dict['username'] = evt.user_actor.userprofile.get_username()
            evt_dict['message'] = re.sub('<', '&gt;', evt_dict['message'])
            all_events_list.append(evt_dict)
        response_dict = {'events': all_events_list}
        return respond(request, response_dict=response_dict)
    else:
        return respond_error(request, status=400, message='Invalid option')


# Anonymous access permissible
@really_defeat_caching
def registration_api_1_0(request, activation_id=None):
    response = None

    try:
        if not activation_id and request.method == 'PUT':
            response = registration_helper_create(request)

        else:
            response = respond_error(request, status=400, message='Bad command or target')

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='registration',
                                       )
        log_entry.save()
        response = respond_error(request, status=500, message='Server error')

    return response


def registration_helper_create(request):
    """
    Create an account and make it pending for activation
    """
    status = 200
    message = 'OK'
    response_dict = {}
    pump_id = 'n/a'
    user = None
    association_key = '?'
    pump = None
    invitation = None

    invited = 'invitation_code' in request.PUT

    required_fields_new = ['email', 'first_name', 'last_name', 'password', 'association_key', 'customer_name', 'time_zone', 'display_units']
    required_fields_invited = ['email', 'first_name', 'last_name', 'password', 'invitation_code', 'time_zone', 'display_units']

    if invited:
        if not set(required_fields_invited).issubset(request.PUT):
            status = 400
            message = 'Missing required parameter'
            return respond(request, status, message, response_dict=response_dict)
    else:
        if not set(required_fields_new).issubset(request.PUT):
            status = 400
            message = 'Missing required parameter'
            return respond(request, status, message, response_dict=response_dict)

    # The username is also the email address
    email = sanitize(re.sub(UserProfile.USERNAME_REGEX, '', request.PUT['email']))
    password = request.PUT['password']
    first_name = sanitize(request.PUT['first_name'])
    last_name = sanitize(request.PUT['last_name'])
    time_zone = sanitize(request.PUT['time_zone'])
    display_units = sanitize(request.PUT['display_units'])

    if not invited:
        association_key = sanitize(re.sub(Pump.ASSOCIATION_KEY_FILTER, '', request.PUT['association_key']))
        customer_name = sanitize(request.PUT['customer_name'])
        pump = Pump.check_pump_for_user_registration(association_key)
    else:
        invitation_code = sanitize(request.PUT['invitation_code'])

    if not re.match(UserProfile.EMAIL_REGEX, email):
        status = 400
        message = 'Email address invalid'
    elif User.objects.filter(email__iexact=email).exists() or User.objects.filter(username__iexact=email).exists():
        status = 400
        message = 'Email address already registered'
    elif not invited and pump is None:
        status = 400
        message = 'Pump not valid or already registered'
    elif len(re.sub(r'[-\s.,]', '', first_name)) < 1:
        status = 400
        message = 'First name too short'
    elif len(re.sub(r'[-\s.,]', '', last_name)) < 1:
        status = 400
        message = 'Last name too short'
    elif not UserProfile.check_password_requirements(password):
        status = 400
        message = UserProfile.check_password_requirements(password, return_reason=True)
    elif not invited and len(re.sub(r'[-\s.,]', '', customer_name)) < 1:
        status = 400
        message = 'Group name too short'
    elif not invited and Customer.objects.filter(organization_name__iexact=customer_name).exists():
        status = 400
        message = 'Group already exists. Please ask the group manager to invite you.'
    elif invited and not Invitation.is_valid(invitation_code):
        status = 400
        message = 'Invalid invitation code'
    else:
        try:
            user = create_graco_user(email, password=password, time_zone=time_zone, display_units=display_units)
            user.first_name = first_name
            user.last_name = last_name
            user.userprofile.save()
            user.save()

            if invited:
                invitation = Invitation.is_valid(invitation_code)
                success = invitation.use(user)
                if not success:
                    raise ValueError('Invalid invitation')
                customer = invitation.customer_invited
            else:
                customer = Customer.objects.create(organization_name=customer_name)
                customer.manager = user
                customer.save()

                pump.customer = customer
                pump.activation_key = ''
                pump_id = pump.unique_id
                pump.save()

            user.userprofile.customers.add(customer)

            nc = Customer.objects.get(pk=get_none_customer())
            if nc in user.userprofile.customers.all():
                user.userprofile.customers.remove(nc)

            response_dict['user_id'] = user.pk

            # Send the activation email
            email_success = user.userprofile.send_email_verification(UserProfile.EMAIL_USER_ACTIVATION)

            if email_success:
                status = 201
                message = 'User created; activation sent'
            else:
                status = 400
                message = 'Error sending activation email; contact support'
        except:
            status = 400
            message = 'Unknown error while creating user'

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    if invited:
        if invitation:
            log_entry.message = ('Registration; email: %s, invitation: %d' % (email, invitation.id))[:512]
        else:
            log_entry.message = ('Registration; email: %s, invitation_code: %s' % (email, invitation_code))[:512]
    else:
        log_entry.message = ('Registration; email: %s, key: %s, pump: %s' % (email, association_key, pump_id))[:512]
    log_entry.target_type = Log.TARGET_USER
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.action = Log.ACTION_CREATE

    if status >= 200 and status < 300:
        log_entry.success = Log.STATUS_SUCCESS

    if user is not None:
        log_entry.target_id = user.pk

    log_entry.save()

    return respond(request, status, message, response_dict=response_dict)


@really_defeat_caching
def notifications_api_1_0(request, user_id=None, pump_id=None, notification_id=None):
    # Do this instead of using the @login_required decorator so that we can send a failure status code
    response = None

    try:
        if not request.user.is_authenticated():
            response = respond_not_logged_in(request)
        # Handle list of notifications (all, by pump, or by user)
        elif request.method == 'GET':
            response = notifications_helper_list(request, user_id, pump_id)
        # Handle notification creation
        elif not user_id and not pump_id and request.method == 'PUT':
            response = notifications_helper_create(request)
        # Handle notification deletion
        elif notification_id and request.method == 'DELETE':
            response = notifications_helper_delete(request, notification_id)
        elif notification_id and request.method == 'POST':
            response = notifications_helper_change(request, notification_id)
        else:
            response = respond_error(request, status=400, message='Bad command or target')
    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='notifications',
                                       )
        log_entry.save()
        response = respond_error(request, status=500, message='Server error')

    return response


def notifications_helper_list(request, user_id=None, pump_id=None):
    '''
    Return a list of all notifications for the specified user and/or pump, or all notifications if no params

    If only one of user_id or pump_id are given, the other parameter is assumed to
    include all pumps or users, respectively.
    '''
    status = 200
    message = 'OK'
    response_dict = {}

    user_obj = None
    pump_obj = None

    # Only admins can see notifications not owned by themselves
    if user_id is not None:
        user_obj = User.objects.get(pk=user_id)
        if not user_obj or (user_obj != request.user and not request.user.userprofile.is_admin()):
            return respond_not_authorized(request)
    else:
        if not request.user.userprofile.is_admin():
            user_obj = request.user

    # All notifications need a pump
    if pump_id is not None:
        pump_id = sanitize(pump_id)
        pump_obj = get_pump_if_authorized(request, pump_id)
        if not pump_obj:
            return respond_error(request, message='Bad target')

    try:
        if pump_obj and user_obj:
            notifications = Notification.objects.filter(is_enabled=True, pump=pump_obj, user=user_obj).order_by('pump', 'user', 'pk')
        elif pump_obj:
            notifications = Notification.objects.filter(is_enabled=True, pump=pump_obj).order_by('user', 'pk')
        elif user_obj:
            notifications = Notification.objects.filter(is_enabled=True, user=user_obj).order_by('pump', 'pk')
        else:
            notifications = Notification.objects.filter(is_enabled=True).order_by('pump', 'user', 'pk')

        response_dict['notifications'] = []

        for notif in notifications:
            # Don't list notifications for groups that have expired
            if not request.user.userprofile.is_admin() and not notif.pump.has_valid_subscription():
                continue

            mini_dict = model_to_dict(notif, fields='id, user, period, time_next_due, time_last_sent, subject_text, criteria')
            mini_dict['pump_id'] = notif.pump.unique_id
            mini_dict['user_name'] = notif.user.userprofile.get_username()
            mini_dict['user_id'] = notif.user.id

            if notif.time_next_due:
                tz = pytz.timezone(request.user.userprofile.time_zone)
                mini_dict['time_next_due_fmt'] = notif.time_next_due.astimezone(tz).strftime('%Y/%m/%d')
            else:
                mini_dict['time_next_due_fmt'] = 'Not set'

            if len(notif.pump.pretty_name) > 0:
                mini_dict['pump_name'] = notif.pump.pretty_name
            else:
                mini_dict['pump_name'] = notif.pump.unique_id

            response_dict['notifications'].append(mini_dict)

    except Exception as e:
        print('Exception fetching notifications: %s' % (repr(e),))
        status = 400
        message = 'Error fetching reminders'

    return respond(request, status=status, message=message, response_dict=response_dict)


def notifications_helper_create(request):
    '''Create a notification for the given pump and user'''
    status = 200
    message = 'OK'
    response_dict = {}
    notif_obj = None

    period_final = None
    subject_final = None
    subject_other_filtered = None

    required_fields = ['user_id', 'pump_id', 'period', 'subject', 'subject_other', 'period_other']

    if status == 200:
        if not set(required_fields).issubset(request.PUT):
            status = 400
            message = 'Missing required parameter'

    if status == 200:
        try:
            user_id = sanitize(request.PUT['user_id'])
            pump_id = sanitize(request.PUT['pump_id'])
            period = sanitize(request.PUT['period'])
            period_other = sanitize(request.PUT['period_other'])
            subject_text_filtered = sanitize(re.sub(Notification.SUBJECT_REGEX, '', request.PUT['subject']))
            subject_other_filtered = sanitize(re.sub(Notification.SUBJECT_REGEX, '', request.PUT['subject_other']))

            user = User.objects.get(pk=user_id)
            pump = get_pump_if_authorized(request, pump_id)

        except:
            status = 400
            message = 'Invalid parameter'

    if status == 200:
        # Only admins can create notifications for people other than themselves
        if not user or (user != request.user and not request.user.userprofile.is_admin()):
            status = 401
            message = 'Not authorized'

        if not user.userprofile.enable_notifications:
            status = 400
            message = 'Cannot create with reminders disabled'

        elif len(user.email) < 1 or not re.match(UserProfile.EMAIL_REGEX, user.email):
            status = 400
            message = 'Email address invalid'

        elif not user.userprofile.email_confirmed:
            status = 400
            message = 'Email address not confirmed'

        elif not pump:
            status = 400
            message = 'Bad target'

    if status == 200:
        if not request.user.userprofile.is_admin() and not pump.customer.has_valid_subscription():
            status = 401
            message = "Subscription expired"

    if status == 200:
        if subject_text_filtered == Notification.SUBJECT_OTHER_VALUE:
            if len(subject_other_filtered) < 1:
                status = 400
                message = 'Custom subject too short'
            elif len(subject_other_filtered) > Notification.SUBJECT_MAX_LENGTH:
                status = 400
                message = 'Custom subject too long'
            else:
                subject_final = subject_other_filtered
        else:
            if subject_text_filtered not in Notification.SUBJECT_TYPES:
                status = 400
                message = 'Invalid subject'
            else:
                subject_final = subject_text_filtered

    if status == 200:
        try:
            if period == Notification.PERIOD_OTHER_VALUE:
                period_other_int = int(period_other)
                if period_other_int < Notification.PERIOD_CUSTOM_MIN or period_other_int > Notification.PERIOD_MAX:
                    status = 400
                    message = 'Invalid custom period'
                else:
                    period_final = period_other_int
            else:
                # Conversion and error checking is handled in the creation helper
                period_final = period

        except:
            status = 400
            message = 'Error creating reminder'

    if status == 200:
        try:
            criteria = Notification.CRITERIA_TYPES.index('Periodic')

            notif_obj = Notification.creation_helper(user=user,
                                                     pump=pump,
                                                     period=period_final,
                                                     subject_text=subject_final,
                                                     criteria=criteria)

            response_dict['notification_id'] = notif_obj.pk
            status = 201
            message = 'Reminder created'

        except:
            status = 400
            message = 'Error creating reminder'

    notif_id = notif_obj.id if notif_obj else ''
    attr_name = 'pump_id' if notif_obj else ''
    new_value = notif_obj.pump.unique_id if notif_obj else ''
    log_command(request, Log.TARGET_NOTIF, notif_id, attribute=attr_name, new_value=new_value, action=Log.ACTION_CREATE, status=status)

    return respond(request, status=status, message=message, response_dict=response_dict)


def notifications_helper_change(request, notification_id):
    '''Change an existing notification'''
    status = 200
    message = 'OK'
    response_dict = {'waiting_for_pump': False}

    notif_obj = Notification.objects.get(pk=notification_id)

    if not notif_obj:
        return respond_error(request, status=400, message='Bad command or target')

    if notif_obj.user != request.user and not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    try:
        (status, message) = handle_notification_change(request, notif_obj)
    except Exception:
        status = 500
        message = "Error while processing reminder change"

    return respond(request, status, message, response_dict=response_dict)


def handle_notification_change(request, notif_obj):
    '''Handle changes to notifications'''
    status = 400
    message = 'Invalid command'

    new_value = sanitize(request.POST['new_value'])
    attr_name = sanitize(request.POST['attr_name'])

    if not notif_obj:
        status = 410
        message = "Invalid reminder"

    elif not request.user.userprofile.is_admin() and not notif_obj.pump.customer.has_valid_subscription():
        status = 401
        message = "Subscription expired"

        '''
        Only a VERY limited subset of notification fields can be changed. In general, it
        is preferable to delete an existing notification and create a new one instead of
        modifying an existing notification.
        '''

    elif 'subject_text' == attr_name:
        subject_text = re.sub(Notification.SUBJECT_REGEX, '', new_value)
        if len(subject_text) > 0 and len(subject_text) < Notification.SUBJECT_MAX_LENGTH:
            notif_obj.subject_text = subject_text
            notif_obj.save()
            status = 200
            message = 'Subject changed'

    notif_id = 'Invalid' if notif_obj is None else notif_obj.pk

    log_command(request, Log.TARGET_NOTIF, notif_id, attr_name, new_value, status=status)

    return (status, message)


def notifications_helper_delete(request, notification_id):
    '''Delete the specified notification'''
    notif_obj = Notification.objects.get(pk=notification_id)

    if not notif_obj:
        return respond_error(request, status=400, message='Bad command or target')

    if notif_obj.user != request.user and not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    '''
    Note that users should still be able to remove notifications even if the pump's
    subscription is expired.
    '''

    notif_id = notif_obj.id
    notif_obj.delete()

    log_command(request, Log.TARGET_NOTIF, notif_id, action=Log.ACTION_DELETE, status=200)

    return respond(request, status=200, message='Reminder deleted')


@really_defeat_caching
def users_api_1_0(request, user_id=None, customer_id=None):
    response = None

    try:
        # Do this instead of using the @login_required decorator so that we can send a failure status code
        if not request.user.is_authenticated():
            response = respond_not_logged_in(request)

        # Handle list of users
        elif not user_id and request.method == 'GET':
            response = user_helper_list(request, customer_id)

        # Handle user creation
        elif not user_id and request.method == 'PUT':
            response = user_helper_create(request)

        # Handle data query
        elif user_id and request.method == 'GET':
            response = user_helper_fetch(request, user_id)

        # Handle user settings change
        elif user_id and request.method == 'POST':
            response = user_helper_change(request, user_id)

        # Handle user deletion
        elif user_id and request.method == 'DELETE':
            response = user_helper_delete(request, user_id)

        else:
            response = respond_error(request, status=400, message='Bad command or target')

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='users',
                                       )
        log_entry.save()
        response = respond_error(request, status=500, message='Server error')

    return response


def user_helper_list(request, customer_id=None):
    '''Get list of all users'''
    user_list = []

    if not request.user.userprofile.is_admin() and customer_id is None:
        return respond_not_authorized(request)

    customer = None
    if customer_id is not None:
        customer = get_customer_if_authorized(request, customer_id)
        if customer is None:
            return respond_not_authorized(request)

    user_profiles = None
    if request.user.userprofile.is_admin() and customer_id is None:
        user_profiles = UserProfile.objects.all()
    elif customer:
        user_profiles = customer.userprofile_set.all()

    user_profiles = user_profiles.order_by('user__email', 'id')

    for user_profile in user_profiles:
        user_values = {'user_id': user_profile.user.pk,
                       'username': user_profile.get_username(),
                       'is_admin': user_profile.is_admin(), }
        user_list.append(user_values)

    flash_message = None
    if customer:
        if not request.user.userprofile.is_admin() and not customer.has_valid_subscription():
            flash_message = Subscription.SUBSCRIPTION_EXPIRED_MESSAGE

    response_dict = {'users': user_list}
    return respond(request, response_dict=response_dict, flash_message=flash_message)


def user_helper_create(request):
    '''Create a new user'''
    status = 200
    message = 'OK'
    response_dict = {}
    user = None

    # Only admins can create users
    if not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    if 'email' not in request.PUT or 'first_name' not in request.PUT:
        status = 400
        message = 'Missing required parameter'

    else:
        first_name = sanitize(request.PUT['first_name'])
        email = sanitize(request.PUT['email'])

        if 'last_name' in request.PUT:
            last_name = sanitize(request.PUT['last_name'])
        else:
            last_name = ''

        if 'customer' in request.PUT:
            customer = Customer.objects.filter(pk=int(request.PUT['customer'])).first()
        else:
            customer = None

        if 'show_disconnected_pumps' in request.PUT:
            show_disconnected_pumps = (request.PUT['show_disconnected_pumps'] == '1')
        else:
            show_disconnected_pumps = True

        if 'enable_notifications' in request.PUT:
            enable_notifications = (request.PUT['enable_notifications'] == '1')
        else:
            enable_notifications = True

        if 'enable_alarm_alerts' in request.PUT:
            enable_alarm_alerts = (request.PUT['enable_alarm_alerts'] == '1')
        else:
            enable_alarm_alerts = True

        if 'is_admin' in request.PUT:
            is_admin = (request.PUT['is_admin'] == '1')
        else:
            is_admin = False

        if 'time_zone' in request.PUT:
            time_zone = sanitize(request.PUT['time_zone'])
        else:
            time_zone = None

        if 'display_units' in request.PUT:
            display_units = sanitize(request.PUT['display_units'])
        else:
            display_units = None

        if User.objects.filter(email__iexact=email).count():
            status = 400
            message = 'Email address already exists'
        elif len(re.sub(r'[-\s.,]', '', first_name)) < 1:
            status = 400
            message = 'First name too short'
        elif len(email) < 1:
            status = 400
            message = 'Email address required'
        elif not re.match(UserProfile.EMAIL_REGEX, email):
            status = 400
            message = 'Email address invalid'
        else:
            try:
                user = create_graco_user(email, customer, is_admin, time_zone, show_disconnected_pumps, display_units, enable_notifications)
                user.first_name = first_name
                user.last_name = last_name
                user.enable_alarm_alerts = enable_alarm_alerts
                user.save()
                response_dict['user_id'] = user.pk
                user.userprofile.send_email_verification(UserProfile.EMAIL_USER_ADMIN_CREATED)
                status = 201
                message = 'User created; activation sent'
            except:
                status = 400
                message = 'Unknown error while creating user'

    #
    # Logging
    #

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.message = ''
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.user_actor = request.user
    log_entry.target_type = Log.TARGET_USER
    log_entry.action = Log.ACTION_CREATE

    if user is not None:
        log_entry.attribute = 'email'
        log_entry.new_value = sanitize(email)
        log_entry.target_id = user.pk

    if status >= 200 and status < 300:
        log_entry.success = 1

    log_entry.save()

    return respond(request, status, message, response_dict=response_dict)


def user_helper_delete(request, user_id):
    '''Hard-delete a user'''
    status = 200
    message = 'User deleted'

    user = User.objects.get(pk=user_id)

    # Only admins can delete users
    if not user or not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    user_profile = user.userprofile
    username = user_profile.get_username()
    user.delete()
    user_profile.delete()

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.action = Log.ACTION_DELETE
    log_entry.target_type = Log.TARGET_USER
    log_entry.target_id = user_id
    log_entry.success = 1
    log_entry.message = sanitize('Username %s' % (username,))
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.user_actor = request.user
    log_entry.save()

    return respond(request, status, message)


def user_helper_fetch(request, user_id):
    status = 200
    message = 'OK'

    user = User.objects.get(pk=user_id)

    # Only admins can access user pages other than their own
    if not user or (user != request.user and not request.user.userprofile.is_admin()):
        return respond_not_authorized(request)

    response_dict = {'username': user.userprofile.get_username(),
                     'email': user.email,
                     'time_zone': user.userprofile.time_zone,
                     'first_name': user.first_name,
                     'last_name': user.last_name,
                     'display_units': UnitsOfMeasure.TEXT_SYSTEM[user.userprofile.display_units],
                     'display_units_id': user.userprofile.display_units,
                     'show_disconnected_pumps': user.userprofile.show_inactive_pumps,
                     'enable_notifications': user.userprofile.enable_notifications,
                     'enable_alarm_alerts': user.userprofile.enable_alarm_alerts,
                     }

    user_customers = user.userprofile.customers.all()
    user_customers_list = []
    for customer in user_customers:
        customer_values = {'unique_id': customer.pk,
                           'name': customer.organization_name, }
        user_customers_list.append(customer_values)
    response_dict['user_customers'] = user_customers_list

    alarm_prefs = {}
    for alarm_id in AlarmPreference.ALARMS_CONFIGURABLE:
        key_name = '%s%d' % (AlarmPreference.QUALIFIER, alarm_id)
        # Unless it's explicitly disabled, assume it's enabled
        if user.alarmpreference_set.filter(alarm_id=alarm_id, send_email=False).exists():
            key_value = 0
        else:
            key_value = 1
        alarm_prefs[key_name] = key_value
    response_dict['alarm_prefs'] = alarm_prefs

    # ONly admins should be able to see all customers
    if request.user.userprofile.is_admin():
        response_dict['is_admin'] = user.userprofile.is_admin()
        response_dict['is_distributor'] = user.userprofile.is_distributor()
        response_dict['is_active'] = user.is_active
        response_dict['email_confirmed'] = 'Yes' if user.userprofile.email_confirmed else 'No'

    return respond(request, status, message, response_dict)


def user_helper_change(request, user_id):
    status = 200
    message = 'OK'
    response_dict = {'waiting_for_pump': False}

    user = User.objects.get(pk=user_id)

    # Only admins can access user pages other than their own
    if not user or (user != request.user and not request.user.userprofile.is_admin()):
        return respond_not_authorized(request)

    try:
        (status, message) = handle_user_settings_change(request, user)
    except Exception:
        status = 500
        message = "Error while processing user change"

    return respond(request, status, message, response_dict=response_dict)


def handle_user_settings_change(request, user):
    '''Handle changes to user settings'''
    log_message = ''

    status = 400
    message = 'Invalid command'

    new_value = sanitize(request.POST['new_value'])
    attr_name = sanitize(request.POST['attr_name'])

    if not user:
        status = 410
        message = "Invalid user"

        '''
        Attributes
        '''

    elif 'email' == attr_name:
        if re.match(UserProfile.EMAIL_REGEX, new_value):
            email_filtered = re.sub(UserProfile.USERNAME_REGEX, '', new_value)
            if User.objects.filter(email__iexact=email_filtered).exists():
                status = 400
                message = 'Email address already exists'
            else:
                user.email = email_filtered

                # Convert to new-style username if needed
                if user.userprofile.is_old_style_username():
                    user.username = UserProfile.generate_username()

                user.userprofile.email_confirmed = False
                user.userprofile.save()
                user.save()
                user.userprofile.send_email_verification(UserProfile.EMAIL_CHANGE_VERIFICATION)
                status = 200
                message = 'Email updated'
        else:
            status = 400
            message = 'Email address invalid'

    elif 'time_zone' == attr_name:
        if new_value in pytz.all_timezones:
            user.userprofile.time_zone = new_value
            # timezone.activate(pytz.timezone(new_value))
            user.userprofile.save()
            status = 200
            message = 'Preference updated'
        else:
            message = 'Invalid time zone'

    elif 'is_active' == attr_name:
        user.is_active = (new_value == '1')
        user.save()
        status = 200
        message = 'User updated'

    elif 'show_disconnected_pumps' == attr_name:
        user.userprofile.show_inactive_pumps = (new_value == '1')
        user.userprofile.save()
        status = 200
        message = 'Preference updated'

    elif 'show_unassigned_pumps' == attr_name:
        user.userprofile.show_unassigned_pumps = (new_value == '1')
        user.userprofile.save()
        status = 200
        message = 'Preference updated'

    elif 'enable_notifications' == attr_name:
        user.userprofile.enable_notifications = (new_value == '1')
        user.userprofile.save()

        # Disable or re-enable all notifications associated with the user if they disable notifications
        notifications = Notification.objects.filter(user=user)
        for notif in notifications:
            notif.is_enabled = user.userprofile.enable_notifications
            notif.save()

        status = 200
        message = 'Preference updated'

    elif 'enable_alarm_alerts' == attr_name:
        user.userprofile.enable_alarm_alerts = (new_value == '1')
        user.userprofile.save()

        status = 200
        message = 'Preference updated'

    elif 'display_units' == attr_name:
        if int(new_value) < len(UnitsOfMeasure.TEXT_SYSTEM):
            user.userprofile.display_units = int(new_value)
            user.userprofile.save()
            status = 200
            message = 'Preference updated'
        else:
            message = 'Invalid units'

    elif 'tos_agreed' == attr_name:
        new_tos_id = int(new_value)
        latest_tos = TermsOfService.objects.last()
        if latest_tos and new_tos_id == latest_tos.id:
            user.userprofile.tos_agreed = latest_tos
            user.userprofile.tos_agreement_date = timezone.now()
            user.userprofile.save()
            status = 200
            message = 'TOS accepted'
        else:
            status = 400
            message = 'Error accepting TOS'

    elif 'is_admin' == attr_name and request.user.userprofile.is_admin():
        if request.user == user:
            message = "Can't change own admin state"
        else:
            user.userprofile.make_admin(new_value == '1')
            status = 200
            message = 'Admin state changed'

    elif 'is_distributor' == attr_name and request.user.userprofile.is_admin():
        user.userprofile.make_distributor(new_value == '1')
        status = 200
        message = 'Distributor state changed'

    elif 'first_name' == attr_name:
        if len(re.sub(r'[-\s.,]', '', new_value)) < 1:
            status = 400
            message = 'First name too short'
        else:
            user.first_name = new_value
            user.save()
            status = 200
            message = 'First name updated'

    elif 'last_name' == attr_name:
        user.last_name = new_value
        user.save()
        status = 200
        message = 'Last name updated'

    elif 'password' == attr_name:
        # Admins can change any password without knowing the old one; users can change only their own password
        if request.user.userprofile.is_admin() or ('old_password' in request.POST and user.check_password(request.POST['old_password'])):
            # Use the raw new value, not the filtered one, since the password should be able to contain any character
            new_password = request.POST['new_value']
            if not UserProfile.check_password_requirements(new_password):
                message = UserProfile.check_password_requirements(new_password, return_reason=True)
            else:
                user.set_password(request.POST['new_value'])
                user.save()
                if request.user == user:
                    update_session_auth_hash(request, user)
                status = 200
                message = 'Password changed'
        else:
            status = 401
            message = 'Error: Old password wrong'

        '''
        Verbs
        '''

    elif 'invitation_accept' == attr_name:
        '''Accept an invitation to join a customer'''
        invitation = Invitation.is_valid(new_value)
        if invitation: 
            if not request.user.is_active:
                status = 400
                message = 'Inactive user cannot accept invitation'
            elif not request.user.userprofile.email_confirmed:
                status = 400
                message = 'Cannot accept invitation with unconfirmed email address'  
            elif invitation.use(request.user):
                status = 200
                message = 'Successfully joined group'
                log_message = 'Group %d' % (invitation.customer_invited.id,)
                messages.success(request, message='Successfully joined group "%s"' % (sanitize(invitation.customer_invited.organization_name),))
            else:
                status = 400
                message = 'Invalid or expired invitation code'
        else:
            status = 400
            message = 'Invalid or expired invitation code'
        pass

    elif re.match(r'%s' % (AlarmPreference.QUALIFIER,), attr_name):
        '''An alarm preference change'''
        parse = re.match('%s([-0-9]+)' % (AlarmPreference.QUALIFIER,), attr_name)
        if parse:
            alarm_id = int(parse.group(1))
            if int(alarm_id) in AlarmPreference.ALARMS_CONFIGURABLE:
                send_email = int(new_value)
                pref = user.alarmpreference_set.filter(alarm_id=alarm_id).first()
                if pref:
                    pref.send_email = send_email
                    pref.save()
                else:
                    AlarmPreference.objects.create(user=user, alarm_id=alarm_id, send_email=send_email)
                status = 200
                message = 'Updated alarm preference'
            else:
                status = 400
                message = 'Invalid alarm'

    '''
    Logging
    '''

    user_id = 'Invalid' if user is None else user.pk
    log_command(request, Log.TARGET_USER, user_id, attr_name, new_value, status=status, message=log_message)

    return (status, message)


@really_defeat_caching
def payments_api_1_0(request, payment_id=None):
    response = None

    try:
        # Do this instead of using the @login_required decorator so that we can send a failure status code
        if not request.user.is_authenticated():
            response = respond_not_logged_in(request)

        # Handle list of payments
        elif payment_id is None and request.method == 'GET':
            response = payments_helper_list(request)

        # Handle payment creation
        elif request.method == 'PUT':
            response = payments_helper_create(request)

        # Handle payment details
        elif payment_id is not None and request.method == 'GET':
            response = payments_helper_fetch(request, payment_id)

        # Handle payment change
        elif payment_id is not None and request.method == 'POST':
            response = payments_helper_change(request, payment_id)

        # Handle payment deletion
        elif payment_id is not None and request.method == 'DELETE':
            response = payments_helper_delete(request, payment_id)

        else:
            response = respond_error(request, status=400, message='Bad command or target')

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='payments',
                                       )
        log_entry.save()

        print('***Exception data:')
        traceback.print_stack()
        traceback.print_exc()
        print('***Request data:')
        print(repr(request))

        response = respond_error(request, status=500, message='Server error')

    return response


def payments_helper_list(request):
    status = 501
    message = 'Not implemented'
    response_dict = {}

    return respond(request, status, message, response_dict=response_dict)


def payments_helper_create(request):
    '''Create a new payment (or, in a violation of convention, modify it)'''
    status = 200
    message = 'OK'
    log_message = ''
    response_dict = {}
    customer = None
    virtual_user = None
    plan = None
    admin_override = False

    if 'token_id' not in request.PUT or 'customer_id' not in request.PUT:
        status = 400
        message = 'Missing required parameter'

    else:
        token_id_sanitized = sanitize(request.PUT['token_id'])
        customer_id_sanitized = sanitize(request.PUT['customer_id'])

        if 'plan_code' in request.PUT:
            plan_code_sanitized = sanitize(request.PUT['plan_code'])
            try:
                if request.user.userprofile.is_admin():
                    # Admins can select any available plan
                    plan = Plan.objects.get(code=plan_code_sanitized, is_available=True)
                    admin_override = True
            except:
                plan = None

            if plan is None:
                # Normal users are constrained to "selectable" plans
                try:
                    plan = Plan.objects.get(code=plan_code_sanitized, is_available=True, user_selectable=True)
                except:
                    plan = None

        customer = get_customer_if_authorized(request, customer_id_sanitized, require_manager=True)

    if customer is None:
        status = 401
        message = 'Not authorized'
    elif customer.manager != request.user and not request.user.userprofile.is_admin():
        status = 400
        message = 'Only the account manager can submit a payment'
    elif plan is None:
        status = 400
        message = 'Invalid plan specified'
    else:
        try:
            # Hook to enable admins to masquerade as the account manager
            if request.user.userprofile.is_admin():
                virtual_user = customer.manager
            else:
                virtual_user = request.user

            if customer.has_valid_subscription():
                subscription = Payments.update_subscription(token_id_sanitized, plan, customer, virtual_user)
                message = 'Subscription updated'
                log_message = message
            else:
                subscription = Payments.new_subscription(token_id_sanitized, plan, customer, virtual_user)
                message = 'Subscription created'
                log_message = message

            if subscription:
                subscription.admin_overrode_plan_selection = admin_override
                subscription.save()

            status = 200
            messages.success(request, message)

        except ValidationError as e:
            status = 400
            message = None
            log_message = 'Error making subscription: %s' % repr(e)

            p1 = e.response_doc.find('transaction_error')
            if p1 is not None:
                p2 = p1.find('customer_message')
                if p2 is not None:
                    message = p2.text

            if message is None:
                message = e.error

        except Exception as e:
            status = 400
            message = 'Error creating subscription'
            log_message = 'Error making subscription: %s' % repr(e)

    #
    # Logging
    #

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.action = Log.ACTION_CREATE
    log_entry.target_type = Log.TARGET_CUSTOMER
    log_entry.user_actor = request.user  # Keep this the actual user, not the virtual user
    log_entry.message = log_message
    log_entry.origin_ip = request.META['REMOTE_ADDR']

    if customer is not None:
        log_entry.target_id = customer.pk

    if status >= 200 and status < 300:
        log_entry.success = 1

    log_entry.save()

    return respond(request, status, message, response_dict=response_dict)


def payments_helper_delete(request, payment_id):
    status = 501
    message = 'Not implemented'
    response_dict = {}

    return respond(request, status, message, response_dict=response_dict)


def payments_helper_fetch(request, payment_id):
    status = 501
    message = 'Not implemented'
    response_dict = {}

    return respond(request, status, message, response_dict=response_dict)


def payments_helper_change(request, payment_id):
    status = 200
    message = None
    response_dict = {'waiting_for_pump': False}

    try:
        (status, message) = handle_payments_change(request, payment_id)
    except Exception as e:
        print('Exception during payment change: %s' % repr(e))
        status = 500
        message = "Error while processing payment change"

    return respond(request, status, message, response_dict=response_dict)


def handle_payments_change(request, payment_id):
    '''Handle changes to payments'''
    status = 400
    message = 'Invalid command'

    new_value = sanitize(request.POST['new_value'])
    attr_name = sanitize(request.POST['attr_name'])

    if 'subscription' == attr_name and 'cancel' == new_value:
        '''
        Treat the payment ID as a subscription and cancel it.
        '''
        try:
            subscription = Subscription.objects.get(recurly_uuid=payment_id)
        except Subscription.DoesNotExist:
            subscription = None
        except Subscription.MultipleObjectsReturned:
            subscription = None

        if not subscription:
            status = 400
            message = 'Invalid subscription'
        else:
            # Verify permissions
            if subscription.customer.manager != request.user and not request.user.userprofile.is_admin():
                status = 401
                message = 'Only the group manager can cancel a subscription'
            elif subscription.account.user != request.user and not request.user.userprofile.is_admin():
                status = 401
                message = 'Only the user who made the subscription can cancel it'
            elif subscription.status not in Subscription.ACTIVE_STATUSES:
                status = 400
                message = 'Cannot cancel an inactive subscription'
            else:
                # Attempt to cancel. Final updates made in webhook
                subscription.terminate()
                status = 200
                message = 'Subscription canceled'
                messages.success(request, message)

    log_command(request, Log.TARGET_PAYMENT, payment_id, attr_name, new_value, status=status)

    return (status, message)


@really_defeat_caching
def customers_api_1_0(request, customer_id=None):
    response = None

    try:
        # Do this instead of using the @login_required decorator so that we can send a failure status code
        if not request.user.is_authenticated():
            response = respond_not_logged_in(request)

        # Handle list of customers
        elif not customer_id and request.method == 'GET':
            response = customer_helper_list(request)

        # Handle customer creation
        elif not customer_id and request.method == 'PUT':
            response = customer_helper_create(request)

        # Handle data query
        elif customer_id and request.method == 'GET':
            response = customer_helper_fetch(request, customer_id)

        # Handle customer settings change
        elif customer_id and request.method == 'POST':
            response = customer_helper_change(request, customer_id)

        # Handle customer deletion
        elif customer_id and request.method == 'DELETE':
            response = customer_helper_delete(request, customer_id)

        else:
            response = respond_error(request, status=400, message='Bad command or target')

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='customers',
                                       )
        log_entry.save()

        print('***Exception data:')
        traceback.print_stack()
        traceback.print_exc()
        print('***Request data:')
        print(repr(request))

        response = respond_error(request, status=500, message='Server error')

    return response


def customer_helper_list(request):
    '''List customers. Admins can see all customers, but non-admins see only customers for which they are members'''
    customer_dict = []

    none_id = get_none_customer()
    none_customer_values = None

    if request.user.userprofile.is_admin():
        customers = Customer.objects.order_by('organization_name')
    else:
        ''' 
        Users in the None group shouldn't be able to see others in the None group,
        but otherwise they should be able to see all of the stats about their groups.
        '''
        customers = request.user.userprofile.customers.exclude(id=none_id).order_by('organization_name')

    for customer in customers:
        pump_count = Pump.objects.filter(customer=customer, is_active=True).count()
        user_count = customer.userprofile_set.count()
        customer_values = {'unique_id': customer.pk,
                           'name': customer.organization_name,
                           'pump_count': pump_count,
                           'user_count': user_count, }

        '''Put the None group at the end of the list'''
        if customer.id == none_id:
            none_customer_values = customer_values
        else:
            customer_dict.append(customer_values)

    if none_customer_values:
        customer_dict.append(none_customer_values)

    response_dict = {'customers': customer_dict}
    return respond(request, response_dict=response_dict)


def customer_helper_create(request):
    '''Create a new customer'''
    status = 200
    message = 'OK'
    response_dict = {}
    customer = None
    new_org_name = 'n/a'
    override_subscription = False

    # Only admins can create customers
    if not request.user.userprofile.is_admin() and not request.user.userprofile.is_distributor():
        return respond_not_authorized(request)

    if 'organization_name' not in request.PUT:
        status = 400
        message = 'Missing required parameter'

    else:
        new_org_name = sanitize(request.PUT['organization_name'])

        if len(re.sub(r'\s', '', new_org_name)) < 1:
            status = 400
            message = 'Group name too short'
        elif Customer.objects.filter(organization_name__iexact=new_org_name).first():
            status = 400
            message = 'Group name already exists'
        elif 'manager' in request.PUT and len(request.PUT['manager']) > 0 and request.user.userprofile.is_admin():
            # Make sure the specified user actually exists
            manager_user_query = User.objects.filter(email__iexact=request.PUT['manager'])
            if manager_user_query.count() != 1:
                status = 400
                message = 'Manager user invalid'
        elif 'override_subscription' in request.PUT and request.user.userprofile.is_admin():
            override_subscription = (request.PUT['override_subscription'] == '1')

        # If we didn't fail out above...
        if status == 200:
            try:
                if request.user.userprofile.is_distributor():
                    '''Distributors can only make groups with themselves as managers'''
                    manager = request.user
                else:
                    '''Admins can make groups with anybody as a manager, or no manager'''
                    if 'manager' in request.PUT and len(request.PUT['manager']) > 0:
                        manager = User.objects.filter(email__iexact=request.PUT['manager']).first()
                    else:
                        manager = None
                customer = Customer.objects.create(organization_name=new_org_name,
                                                   manager=manager,
                                                   override_subscription=override_subscription)
                if manager:
                    manager.userprofile.customers.add(customer)

                    nc = Customer.objects.get(pk=get_none_customer())
                    if nc in manager.userprofile.customers.all():
                        manager.userprofile.customers.remove(nc)

                    manager.userprofile.save()
                response_dict['customer_id'] = customer.pk
                message = 'Customer created'
                status = 201
            except:
                status = 400
                message = 'Unknown error while creating customer'

    #
    # Logging
    #

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.action = Log.ACTION_CREATE
    log_entry.target_type = Log.TARGET_CUSTOMER
    log_entry.user_actor = request.user
    log_entry.attribute = 'organization_name'
    log_entry.new_value = sanitize(new_org_name)
    log_entry.origin_ip = request.META['REMOTE_ADDR']

    if customer is not None:
        log_entry.target_id = customer.pk

    if status >= 200 and status < 300:
        log_entry.success = 1

    log_entry.save()

    return respond(request, status, message, response_dict=response_dict)


def customer_helper_delete(request, customer_id):
    '''Customer deletes are "hard" deletes'''
    status = 200
    message = 'OK'

    # Only admins can delete customers
    customer = Customer.objects.get(pk=customer_id)
    if not customer or not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    # Can't delete the None customer
    if customer.id == get_none_customer():
        return respond_not_authorized(request)

    customer_id = customer.pk
    customer_name = customer.organization_name

    # Customers cannot be deleted if there are still active users associated with them
    userprofiles = customer.userprofile_set.all()
    if userprofiles.count() > 0:
        status = 400
        message = 'Group still has users; reassign them first'
    else:
        # Disown any pumps still owned by the customer
        pumps = Pump.objects.filter(customer=customer)
        none_customer = Customer.objects.get(pk=get_none_customer())
        for pump in pumps:
            pump.customer = none_customer
            pump.is_active = True
            pump.suspended = False
            pump.save()

        customer.delete()

        message = 'Group deleted'

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.message = sanitize('Group name: %s' % customer_name)
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.user_actor = request.user
    log_entry.action = Log.ACTION_DELETE
    log_entry.target_type = Log.TARGET_CUSTOMER
    log_entry.target_id = customer_id

    if status >= 200 and status < 300:
        log_entry.success = 1

    log_entry.save()

    return respond(request, status, message)


def customer_helper_fetch(request, customer_id):
    status = 200
    message = 'OK'
    flash_message = None

    customer = get_customer_if_authorized(request, customer_id)

    if not customer:
        return respond_not_authorized(request)

    # Users in the None group shouldn't be able to see who else is in the None group
    if customer.id == get_none_customer() and not request.user.userprofile.is_admin():
        return respond_not_authorized(request)
        

    response_dict = {'customer_id': customer.pk,
                     'organization_name': customer.organization_name,
                     'user_id_name_list': customer.get_user_id_name_list(),
                     'pump_id_name_list': customer.get_pump_id_name_list(),
                     'show_subscription_link': (not customer.subscription_overridden()),
                     }

    if customer.manager is not None:
        response_dict['manager_id'] = customer.manager.id
        response_dict['manager_name'] = customer.manager.userprofile.get_username()

    if request.user.userprofile.is_admin():
        response_dict['override_subscription'] = customer.override_subscription

    if not request.user.userprofile.is_admin() and not customer.has_valid_subscription():
        flash_message = Subscription.SUBSCRIPTION_EXPIRED_MESSAGE

    return respond(request, status, message, response_dict=response_dict, flash_message=flash_message)


def customer_helper_change(request, customer_id):
    status = 200
    message = None
    response_dict = {'waiting_for_pump': False}

    # Only admins and a customer's manager can change a customer's data
    customer = get_customer_if_authorized(request, customer_id, require_manager=True)

    if not customer:
        return respond_not_authorized(request)

    try:
        (status, message) = handle_customer_settings_change(request, customer)
    except:
        status = 500
        message = "Error while processing group change"

    return respond(request, status, message, response_dict=response_dict)


def handle_customer_settings_change(request, customer):
    '''Handle changes to customer settings'''
    status = 400
    message = 'Invalid command'

    new_value = sanitize(request.POST['new_value'])
    attr_name = sanitize(request.POST['attr_name'])

    if not customer:
        status = 410
        message = "Invalid group"

    elif 'organization_name' == attr_name:
        if customer.organization_name == 'None':
            status = 401
            message = 'Cannot edit this account'
        elif len(re.sub(r'\s', '', new_value)) < 1:
            status = 400
            message = 'Name too short'
        else:
            customer.organization_name = new_value
            customer.save()
            status = 200
            message = 'Name updated'

    elif 'manager_name' == attr_name:
        new_manager_query = User.objects.filter(email__iexact=new_value)
        if new_manager_query.count() != 1 and len(new_value) != 0:
            status = 400
            message = 'Invalid manager'
        elif len(new_value) == 0 and not request.user.userprofile.is_admin():
            # Only admins can make a group have no manager at all. This prevents
            # users from accidentally removing themselves as manager of their group
            # and leaving their group managerless
            status = 400
            message = 'Cannot set blank manager'
        elif request.user != customer.manager and not request.user.userprofile.is_admin():
            status = 400
            message = 'Insufficient privileges'
        elif len(new_value) > 0 and customer not in new_manager_query.first().userprofile.customers.all():
            status = 400
            message = 'Manager must already be in group'
        elif customer == Customer.objects.get(pk=get_none_customer()):
            status = 400
            message = 'The "None" group cannot have a manager'
        else:
            manager = None
            if len(new_value) > 0:
                manager = User.objects.get(email=new_value)
            customer.manager = manager
            customer.save()
            status = 200
            message = 'Manager updated'

    elif 'override_subscription' == attr_name:
        if not request.user.userprofile.is_admin():
            status = 400
            message = 'Insufficient privileges'
        else:
            customer.override_subscription = (new_value == '1')
            customer.save()
            status = 200
            message = 'Subscription requirement updated'

    elif 'user_add' == attr_name:
        '''
        Add the given user to this customer (immediately)

        For admins only. Non-admins should 'invite' rather than 'add'
        '''
        user_query = User.objects.filter(email__iexact=new_value)
        if user_query.count() == 0:
            status = 400
            message = 'Invalid user'
        elif user_query.count() > 1:
            status = 400
            message = 'Ambiguous match'
        elif user_query.count() == 1:
            user = user_query.first()

            if customer in user.userprofile.customers.all():
                status = 400
                message = 'User already in this group'

            # Remember, admins only
            elif request.user.userprofile.is_admin():
                user.userprofile.customers.add(customer)

                # Yeah, yeah: I should really just get rid of the "none" customer. It made sense in the beginning, but
                # now it would be simpler to just use the empty set or a literal None instead of having a customer
                # named "None". My apologies to you, future maintainer of this code.
                none_customer = Customer.objects.get(pk=get_none_customer())
                if none_customer in user.userprofile.customers.all():
                    user.userprofile.customers.remove(none_customer)

                status = 200
                message = 'User added to group'

            else:
                status = 401
                message = 'Not authorized'

    elif 'user_invite' == attr_name:
        '''Invite the given email address to this customer'''

        if not re.match(UserProfile.EMAIL_REGEX, new_value):
            status = 400
            message = 'Email address invalid'
        elif customer == Customer.objects.get(pk=get_none_customer()):
            status = 400
            message = 'Cannot invite to None group'
        else:
            # Admins or managers only
            if request.user.userprofile.is_admin() or customer.manager == request.user:
                invitation = Invitation.new(request.user, new_value, customer)
                if invitation.send():
                    status = 200
                    message = 'Invitation sent'
                    messages.success(request, message='Invitation sent')
                else:
                    invitation.state = Invitation.INVITATION_STATE_INACTIVE
                    invitation.save()
                    status = 400
                    message = 'Error sending invitation'
            else:
                status = 401
                message = 'Not authorized'

    elif 'user_remove' == attr_name:
        '''Remove the user from membership in this group'''
        user_query = User.objects.filter(pk=int(new_value))
        if user_query.count() == 0:
            status = 400
            message = 'Invalid user'
        elif user_query.count() > 1:
            status = 400
            message = 'Ambiguous match'
        elif user_query.count() == 1:
            user = user_query.first()

            if not request.user.userprofile.is_admin() and request.user != customer.manager:
                status = 401
                message = 'Not authorized'
            elif user == request.user and not request.user.userprofile.is_admin():
                '''
                Only admins can remove themselves from groups. This guards against normal 
                users accidentally removing themselves from a group they control
                '''
                status = 400
                message = 'Cannot remove yourself from a group'
            else:
                user.userprofile.customers.remove(customer)

                '''
                User can't manage a group if they aren't a member of the group
                '''
                if user == customer.manager:
                    customer.manager = None
                    customer.save()

                '''
                If the user is no longer a member of anything, make them a member of the None group
                '''
                if user.userprofile.customers.all().count() == 0:
                    none_customer = Customer.objects.get(pk=get_none_customer())
                    user.userprofile.customers.add(none_customer)

                '''
                If the user had any reminders set up for pumps in that group, they should
                no longer receive those reminders, since they can no longer access
                those pumps.
                '''
                pumps_in_group = customer.pump_set.all()
                notifications = Notification.objects.filter(user=user, pump__in=pumps_in_group)
                notifications.delete()

                status = 200
                message = 'Group membership updated'

    customer_id = 'Invalid' if customer is None else customer.pk

    log_command(request, Log.TARGET_CUSTOMER, customer_id, attr_name, new_value, status=status)

    return (status, message)


def mqtt_auth_api_1_0(request):
    response = None

    try:
        if request.method == 'GET':
            response = mqtt_auth_helper(request)
        else:
            response = respond_error(request, status=400, message='Bad command or target')

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='mqtt_auth',
                                       )
        log_entry.save()
        response = respond_error(request, status=500, message='Server error')

    return response


def ip_in_allowed_subnets(ip_addr, allowed_subnets):
    '''Check if the given IP address (as a string) is in the given list of subnets'''
    match = False
    ip_numerical = ipaddress.ip_address(ip_addr)

    for subnet in allowed_subnets:
        if ip_numerical in ipaddress.ip_network(subnet):
            match = True
            break

    return match


def mqtt_auth_helper(request):
    """Return a MQTT username and password for the specified pump."""
    response_dict = {}
    status = 500
    success = False
    pump_obj = None

    # Allow requests only from the Aeris network
    allowed_subnets = ['127.0.0.1',  # for testing
                       '216.3.13.0/24',
                       '216.84.179.0/26',
                       '217.163.63.0/24',
                       '207.212.62.0/24',
                       '66.209.78.0/24',
                       '205.201.50.0/24',
                       '205.201.51.0/24'
                       ]
    allowed_forwarders = ['45.79.140.182',
                          '198.58.118.79']
    if not ip_in_allowed_subnets(request.META['REMOTE_ADDR'], allowed_subnets):
        # Special case for forwards
        if ip_in_allowed_subnets(request.META['REMOTE_ADDR'], allowed_forwarders) and ip_in_allowed_subnets(request.META['HTTP_X_REAL_IP'], allowed_subnets):
            pass
        else:
            raise Http404('')

    try:
        if request.GET['magic_number'] == Pump.MAGIC_NUMBER and 'pump_id' in request.GET:
            pump_id = re.sub(r'[^0-9]', '', request.GET['pump_id'])

            pump_obj = Pump.auth_req(pump_id)

        if pump_obj:
            response_dict['username'] = pump_obj.mqtt_auth.username
            response_dict['password'] = pump_obj.mqtt_auth.pw_clear

        if pump_obj:
            status = 200
            success = True
        else:
            status = 401
            message = 'Not authorized'
    except:
        status = 500
        message = 'Server error'

    if success:
        return HttpResponse("u:%s\r\np:%s\r\n" % (response_dict['username'], response_dict['password']), content_type="text/plain")
    else:
        return respond_error(request, status=status, message=message)


@really_defeat_caching
def pumps_api_1_0(request, pump_id=None, customer_id=None):
    response = None

    try:
        # Do this instead of using the @login_required decorator so that we can send a failure status code
        if not request.user.is_authenticated():
            response = respond_not_logged_in(request)

        # Handle list of pumps
        elif not pump_id and request.method == 'GET':
            response = pump_helper_list(request, customer_id)

        # Handle pump data query
        elif pump_id and request.method == 'GET':
            response = pump_helper_fetch(request, pump_id)

        # Handle pump settings change
        elif pump_id and request.method == 'POST':
            response = pump_helper_change(request, pump_id)

        # Handle pump deletion
        elif pump_id and request.method == 'DELETE':
            response = pump_helper_delete(request, pump_id)

        # Handle pump activations, NOT pump creations
        elif request.method == 'PUT' and customer_id is not None:
            response = pump_helper_associate(request, customer_id)

        else:
            response = respond_error(request, status=400, message='Bad command or target')

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='pumps',
                                       )
        log_entry.save()

        print('***Exception data:')
        traceback.print_stack()
        traceback.print_exc()
        print('***Request data:')
        print(repr(request))
        print(repr(request.PUT))

        response = respond_error(request, status=500, message='Server error')

    return response


def pump_helper_list(request, customer_id=None):
    '''Get a list of all pumps the user has rights to'''
    pump_list = []
    prefs_dict = {}
    pumps = None
    customer = None
    flash_message = None

    is_admin = request.user.userprofile.is_admin()

    if customer_id is not None:
        customer = get_customer_if_authorized(request, customer_id)
        if customer is None:
            return respond_error(request, message='Not authorized')

    '''Admins see all of the pumps; users see only their company's pumps'''
    if is_admin:
        pumps = Pump.objects.filter(is_active=True)
    else:
        customers = request.user.userprofile.customers.exclude(id=get_none_customer())
        pumps = Pump.objects.filter(customer__in=customers, is_active=True)

    if pumps and customer:
        pumps = pumps.filter(customer=customer)
        if not request.user.userprofile.is_admin() and not customer.has_valid_subscription():
            flash_message = Subscription.SUBSCRIPTION_EXPIRED_MESSAGE

    '''
    Unassigned pump filtering

    (Note that inactive pump filtering is done client-side to facilitate client-side metadata display)
    '''
    if pumps:
        if not request.user.userprofile.show_unassigned_pumps:
            pumps = pumps.exclude(customer_id=get_none_customer())

    '''Allow custom filtering'''
    filter_phrase = None
    if 'filter_key' in request.GET and 'filter_criteria' in request.GET:
        filter_key = request.GET['filter_key']
        filter_crit = request.GET['filter_criteria']
        if filter_key == 'match':
            filter_phrase = {'pretty_name__icontains': filter_crit}

    if pumps and filter_phrase:
        pumps = pumps.filter(**filter_phrase)

    '''Allow custom sorting'''
    sort_phrases = []
    if 'sort' in request.GET:
        sort_key = request.GET['sort']

        direction = ''
        if 'direction' in request.GET:
            if request.GET['direction'] == 'reverse':
                direction = '-'

        if sort_key == 'pump_name':
            sort_phrases.append(direction + 'pretty_name')
        elif sort_key == 'group_name':
            sort_phrases.append(direction + 'customer__organization_name')
        elif sort_key == 'status':
            sort_phrases.append(direction + 'connection')
            sort_phrases.append(direction + 'status')
    else:
        sort_phrases.append('customer__organization_name')

    if pumps:
        sort_phrases.append('unique_id')
        pumps = pumps.order_by(*sort_phrases)

    '''Check for valid subscription'''
    if pumps:
        for pump in pumps:
            mini_dict = model_to_dict(pump, fields='customer,connection,status,pretty_name,unique_id')
            mini_dict['customer_name'] = pump.customer.organization_name

            # Check for valid subscription
            if not is_admin:
                if not pump.has_valid_subscription():
                    mini_dict['status'] = Pump.INVALID_SUB_STATUS
                    mini_dict['connection'] = False
                    mini_dict['group_subscription_valid'] = 0
                else:
                    mini_dict['group_subscription_valid'] = 1

            pump_list.append(mini_dict)

    prefs_dict['show_inactive_pumps'] = request.user.userprofile.show_inactive_pumps
    prefs_dict['show_unassigned_pumps'] = request.user.userprofile.show_unassigned_pumps

    response_dict = {'pumps': pump_list, 'prefs': prefs_dict}
    return respond(request, response_dict=response_dict, flash_message=flash_message)


def pump_helper_delete(request, pump_id):
    '''Soft-delete a pump by setting it inactive'''
    pump = get_pump_if_authorized(request, pump_id, access='delete')
    if not pump:
        return respond_error(request, message='Bad target')

    soft_delete_pump(pump)
    message = 'Pump deleted'

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.message = 'Pump %s' % (pump.pretty_name,)
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.success = 1
    log_entry.action = Log.ACTION_DELETE
    log_entry.target_type = Log.TARGET_PUMP
    log_entry.target_id = pump.unique_id
    log_entry.user_actor = request.user
    log_entry.save()

    return respond(request, message=message)


def pump_helper_fetch(request, pump_id):
    '''Get data for existing pump'''
    flash_message = None
    pump_dict = {}
    pump_parsed = {}
    prefs_dict = {}
    status = 200
    flash_safe = False

    pump = get_pump_if_authorized(request, pump_id)

    if not pump:
        status = 404
        messages.error(request, 'Pump not found', 'danger')
        return respond_error(request, status=404, message='Pump not found')

    # If we found the pump
    if status == 200:
        pump_parsed = parse_pump(pump, request)
        pump_dict = model_to_dict(pump, exclude='mqtt_auth')
        pump_dict['timestamp'] = int(pump.timestamp.timestamp())

        is_manager = (request.user == pump.customer.manager)

        prefs_dict['display_units'] = request.user.userprofile.display_units

        if request.user.userprofile.is_admin() or pump.has_valid_subscription():
            pass
        elif pump.suspended and pump.customer.has_valid_subscription():
            pump_dict = {}
            pump_parsed = {'can_unsuspend': pump_parsed['can_unsuspend']}
            flash_message = 'Pump data unavailable because this pump has been suspended.'
            if not is_manager:
                flash_message += ' Ask the manager of this group to end the suspension.'
            else:
                flash_message += ' To resume command of this pump, end the suspension using the button below.'
        else:
            pump_dict = {}
            pump_parsed = {'can_unsuspend': pump_parsed['can_unsuspend']}
            flash_message = 'Pump data unavailable due to expired subscription.'
            if not is_manager:
                flash_message += ' Please contact the manager for the group this pump is in and ask them to renew.'
            else:
                payment_url = reverse('customer_subscription', args=[pump.customer.id])
                flash_message += ' To resume command of this pump, renew the subscription for this group on its <a href="%s">subscription page</a>' % (payment_url,)
                flash_safe = True

    response_dict = {'pump_obj': pump_dict,
                     'pump_parsed': pump_parsed,
                     'prefs': prefs_dict}
    return respond(request, status=status, response_dict=response_dict, flash_message=flash_message, flash_safe=flash_safe)


def pump_helper_change(request, pump_id):
    '''Change settings for existing pump'''
    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        return respond_error(request, message='Bad target')

    waiting_for_pump = False

    try:
        (status, message, waiting_for_pump) = handle_pump_change(request, pump)

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='pump_change',
                                       )
        log_entry.save()

        print('***Exception data:')
        traceback.print_stack()
        traceback.print_exc()
        print('***Request data:')
        print(repr(request))

        status = 500
        message = 'Error while processing pump change'

    response_dict = {'waiting_for_pump': waiting_for_pump}
    return respond(request, status=status, message=message, response_dict=response_dict)


def handle_pump_change(request, pump):
    '''Handle commands sent for updating the pump or the pump's settings'''
    status = 400
    message = 'Invalid command'
    waiting_for_pump = False

    pump_id = pump.unique_id

    pump_id_filtered = sanitize(re.sub(Pump.regex_for_clean, '', pump_id))
    new_value = sanitize(request.POST['new_value'])
    attr_name = sanitize(request.POST['attr_name'])

    scale_factor_tank_param = pump.get_tank_parameter_scale_factor()
    
    units = None
    if 'units' in request.POST:
        try:
            if int(request.POST['units']) < len(UnitsOfMeasure.TEXT_SYSTEM):
                units = int(request.POST['units'])
        except:
            units = None

    if not pump:
        status = 410
        message = "Pump not found"

    elif not request.user.userprofile.is_admin() and not \
        pump.customer.has_valid_subscription() and \
            ('customer' != attr_name or int(new_value) != get_none_customer()):
        # Allow removal of pumps from a group even without subscription

        status = 401
        message = "Subscription expired"

    elif 'pretty_name' == attr_name:
        old_value = pump.pretty_name
        pump.pretty_name = new_value

        # Publish the new name so that the pump can display it locally
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetPumpName', old_value, new_value, retain=True)

        # Override waiting_for_pump since the pump will take no action when its name is changed
        waiting_for_pump = False

        if status >= 200 and status < 300:
            pump.save()
            message = 'Name updated'

    elif 'location_name' == attr_name:
        pump.location_name = new_value
        pump.save()
        status = 200
        message = 'Location updated'
        
    elif 'notes_field' == attr_name:
        pump.notes_field = new_value
        pump.save()
        status = 200
        message = 'Note updated'

    elif 'customer' == attr_name:
        success = False
        current_customer = None
        if pump.customer:
            current_customer = pump.customer
        new_customer = Customer.objects.get(pk=int(new_value))

        # Check if the user requesting the change is authorized to make the change
        if current_customer is None or current_customer.is_none_customer():
            if request.user.userprofile.is_admin() or request.user == new_customer.manager:
                pump.customer = new_customer
                success = True
        elif request.user.userprofile.is_admin():
            pump.customer = new_customer
            success = True
        elif current_customer and current_customer.manager == request.user:
            if new_customer is None or new_customer.is_none_customer() or new_customer.manager == request.user:
                pump.customer = new_customer
                success = True

        if success:
            pump.is_active = True
            pump.suspended = False
            pump.save()

            # Update the pricing and subscription costs for the new and old customers
            if current_customer and current_customer.has_valid_subscription():
                Payments.recalculate(current_customer.get_current_subscription())
            if new_customer and new_customer.has_valid_subscription():
                Payments.recalculate(new_customer.get_current_subscription())

            status = 200
            message = 'Group updated'
        else:
            status = 400
            message = 'Invalid group assignment'

    elif 'suspended' == attr_name:
        success = False

        if request.user.userprofile.is_admin() or request.user == pump.customer.manager:
            suspend = (new_value == 'true')
            pump.suspended = suspend
            pump.save()

            if pump.customer.has_valid_subscription():
                Payments.recalculate(pump.customer.get_current_subscription())

            success = True

        if success:
            pump.save()
            status = 200
            message = 'Suspension state updated'
        else:
            status = 400
            message = 'Not allowed'

    elif 'location_source' == attr_name:
        pump.location_source = new_value
        pump.save()
        status = 200
        message = 'Location source updated'

    elif 'location_marked' == attr_name:
        regex_lat_long = r'[^-0-9.,\s]+'
        if re.search(regex_lat_long, new_value):
            status = 400
            message = 'Invalid coordinates'
        else:
            pump.location_marked = new_value
            pump.save()
            status = 200
            message = 'Marked location updated'

    elif 'flow_rate' == attr_name:
        if units is not None:
            old_value = pump.flow_rate
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetFlowRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'

    elif 'totalizer_resetable' in attr_name:
        old_value = pump.totalizer_resetable
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'ResetTotalizer', old_value, new_value)

    elif 'alarms_status' == attr_name:
        old_value = pump.alarms_status
        new_value = 0  # Force clear

        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'ClearAlarmStatus', old_value, new_value)

    elif 'status' == attr_name:
        old_value = pump.status
        lower_bound = Bounds['Status'][LOW]
        upper_bound = Bounds['Status'][HIGH]
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetPumpStatus', old_value, new_value, lower_bound, upper_bound)

    elif 'metering_on_cycles' == attr_name:
        old_value = pump.metering_on_cycles
        lower_bound = Bounds['Cycles'][LOW]
        upper_bound = Bounds['Cycles'][HIGH]
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetOnCycles', old_value, new_value, lower_bound, upper_bound)

    elif 'metering_on_timeout' == attr_name:
        old_value = pump.metering_on_timeout
        lower_bound = Bounds['Time'][LOW]
        upper_bound = Bounds['Time'][HIGH]
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetPumpOnTimeout', old_value, new_value, lower_bound, upper_bound)

    elif 'metering_on_time' == attr_name:
        old_value = pump.metering_on_time
        lower_bound = Bounds['Time'][LOW]
        upper_bound = Bounds['Time'][HIGH]
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetOnTime', old_value, new_value, lower_bound, upper_bound)

    elif 'metering_off_time' == attr_name:
        old_value = pump.metering_off_time
        lower_bound = Bounds['Time'][LOW]
        upper_bound = Bounds['Time'][HIGH]
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetOffTime', old_value, new_value, lower_bound, upper_bound)

    elif 'high_pressure_trigger' == attr_name:
        if units is not None:
            old_value = pump.high_pressure_trigger
            new_value_psi = UnitsOfMeasure.convert_pressure(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            lower_bound = Bounds['Pressure'][LOW]
            upper_bound = Bounds['Pressure'][HIGH]
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetHighPressureTrigger', old_value, new_value_psi, lower_bound, upper_bound)
        else:
            message = 'Change requires units'

    elif 'low_pressure_trigger' == attr_name:
        if units is not None:
            old_value = pump.low_pressure_trigger
            new_value_psi = UnitsOfMeasure.convert_pressure(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            lower_bound = Bounds['Pressure'][LOW]
            upper_bound = Bounds['Pressure'][HIGH]
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetLowPressureTrigger', old_value, new_value_psi, lower_bound, upper_bound)
        else:
            message = 'Change requires units'

    elif 'low_battery_trigger' == attr_name:
        old_value = pump.low_battery_trigger
        new_value = int(float(new_value) * Pump.SCALING_FACTOR_BATT)
        lower_bound = Bounds['Battery'][LOW] * Pump.SCALING_FACTOR_BATT
        upper_bound = Bounds['Battery'][HIGH] * Pump.SCALING_FACTOR_BATT
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetLowBatteryTrigger', old_value, new_value, lower_bound, upper_bound)

    elif 'battery_warning_trigger' == attr_name:
        old_value = pump.battery_warning_trigger
        new_value = int(float(new_value) * Pump.SCALING_FACTOR_BATT)
        lower_bound = Bounds['Battery'][LOW] * Pump.SCALING_FACTOR_BATT
        upper_bound = Bounds['Battery'][HIGH] * Pump.SCALING_FACTOR_BATT
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetBatteryWarningTrigger', old_value, new_value, lower_bound, upper_bound)

    elif 'tank_level_notify_trigger' == attr_name:
        if pump.tank_type > pump.TANK_UNKNOWN or pump.sensor_type > pump.SENSOR_TANK_PERCENTAGE:
            if units is not None:
                old_value = pump.tank_level_notify_trigger
                new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
                scaled_value = int(new_value_gallons * scale_factor_tank_param)
                lower_bound = Bounds['TankVolume'][LOW] * Pump.SCALING_FACTOR_TANK_LEVEL
                upper_bound = Bounds['TankVolume'][HIGH] * Pump.SCALING_FACTOR_TANK_LEVEL
                (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetTankLevelNotifyTrigger', old_value, scaled_value, lower_bound, upper_bound)
            else:
                message = 'Tank level notify requires units'
        elif pump.sensor_type == pump.SENSOR_TANK_PERCENTAGE:
            old_value = pump.tank_level_notify_trigger
            new_value = int(float(new_value))
            lower_bound = Bounds['Percent'][LOW]
            upper_bound = Bounds['Percent'][HIGH]
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetTankLevelNotifyTrigger', old_value, new_value, lower_bound, upper_bound)
        
    elif 'tank_level_shutoff_trigger' == attr_name:
        if pump.tank_type > pump.TANK_UNKNOWN or pump.sensor_type > pump.SENSOR_TANK_PERCENTAGE:
            if units is not None:
                old_value = pump.tank_level_shutoff_trigger
                new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
                scaled_value = int(new_value_gallons * scale_factor_tank_param)
                lower_bound = Bounds['TankVolume'][LOW] * Pump.SCALING_FACTOR_TANK_LEVEL
                upper_bound = Bounds['TankVolume'][HIGH] * Pump.SCALING_FACTOR_TANK_LEVEL
                (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetTankLevelShutoffTrigger', old_value, scaled_value, lower_bound, upper_bound)
            else:
                message = 'Tank level shutoff requires units'
        elif pump.sensor_type == pump.SENSOR_TANK_PERCENTAGE:
            old_value = pump.tank_level_shutoff_trigger
            new_value = int(float(new_value))
            lower_bound = Bounds['Percent'][LOW]
            upper_bound = Bounds['Percent'][HIGH]
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetTankLevelShutoffTrigger', old_value, new_value, lower_bound, upper_bound)
            
    elif 'flow_verify_percentage' == attr_name:
        old_value = pump.flow_verify_percentage
        lower_bound = Bounds['Percent'][LOW]
        upper_bound = Bounds['Percent'][HIGH] * Pump.SCALING_FACTOR_FLOW_VERIFY
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetFlowVerifyPercentage', old_value, new_value, lower_bound, upper_bound)            

    elif 'temperature_setpoint' == attr_name:
        if units is not None:
            old_value = pump.temperature_setpoint
            if units == UnitsOfMeasure.METRIC:
                new_value_degF = int(UnitsOfMeasure.celsius_to_fahrenheit(float(new_value)))
            else:
                new_value_degF = int(new_value)
            lower_bound = Bounds['Temperature'][LOW]
            upper_bound = Bounds['Temperature'][HIGH]
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetTemperatureSetpoint', old_value, new_value_degF, lower_bound, upper_bound)
        else:
            message = 'Change requires units'

    elif 'temperature_control' == attr_name:
        old_value = pump.temperature_control
        lower_bound = Bounds['TempControl'][LOW]
        upper_bound = Bounds['TempControl'][HIGH]
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetTemperatureControl', old_value, new_value, lower_bound, upper_bound)
    
    elif 'well_flow_rate_1' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_1
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W1SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'well_flow_rate_2' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_2
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W2SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'well_flow_rate_3' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_3
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W3SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'well_flow_rate_4' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_4
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W4SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'well_flow_rate_5' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_5
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W5SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'well_flow_rate_6' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_6
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W6SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'well_flow_rate_7' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_7
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W7SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'well_flow_rate_8' == attr_name:
        if units is not None:
            old_value = pump.well_flow_rate_8
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W8SetRate', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'totalizer_well_1' == attr_name:
        old_value = pump.totalizer_well_1
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W1ResetTot', old_value, new_value)
    
    elif 'totalizer_well_2' == attr_name:
        old_value = pump.totalizer_well_2
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W2ResetTot', old_value, new_value)
    
    elif 'totalizer_well_3' == attr_name:
        old_value = pump.totalizer_well_3
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W3ResetTot', old_value, new_value)
    
    elif 'totalizer_well_4' == attr_name:
        old_value = pump.totalizer_well_4
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W4ResetTot', old_value, new_value)
    
    elif 'totalizer_well_5' == attr_name:
        old_value = pump.totalizer_well_5
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W5ResetTot', old_value, new_value)
    
    elif 'totalizer_well_6' == attr_name:
        old_value = pump.totalizer_well_6
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W6ResetTot', old_value, new_value)
    
    elif 'totalizer_well_7' == attr_name:
        old_value = pump.totalizer_well_7
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W7ResetTot', old_value, new_value)
    
    elif 'totalizer_well_8' == attr_name:
        old_value = pump.totalizer_well_8
        new_value = 0  # Force reset to 0
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'W8ResetTot', old_value, new_value)
        
    elif 'analog_input_mode' == attr_name:
        old_value = pump.analog_input_mode
        lower_bound = Bounds['AinMode'][LOW]
        upper_bound = Bounds['AinMode'][HIGH]
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetAnalogInputMode', old_value, new_value, lower_bound, upper_bound)
        
    elif 'ain_mA_low' == attr_name:
        old_value = pump.ain_mA_low
        new_value = int(float(new_value) * Pump.SCALING_FACTOR_MA)
        lower_bound = Bounds['mA'][LOW] * Pump.SCALING_FACTOR_MA
        upper_bound = Bounds['mA'][HIGH] * Pump.SCALING_FACTOR_MA
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetAinmALow', old_value, new_value, lower_bound, upper_bound)
        
    elif 'ain_mA_high' == attr_name:
        old_value = pump.ain_mA_high
        new_value = int(float(new_value) * Pump.SCALING_FACTOR_MA)
        lower_bound = Bounds['mA'][LOW] * Pump.SCALING_FACTOR_MA
        upper_bound = Bounds['mA'][HIGH] * Pump.SCALING_FACTOR_MA
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetAinmAHigh', old_value, new_value, lower_bound, upper_bound)
        
    elif 'ain_flow_rate_low' == attr_name:
        if units is not None:
            old_value = pump.ain_flow_rate_low
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetAinFlowRateLow', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
            
    elif 'ain_flow_rate_high' == attr_name:
        if units is not None:
            old_value = pump.ain_flow_rate_high
            new_value_gallons = UnitsOfMeasure.convert_volume(units, UnitsOfMeasure.IMPERIAL, float(new_value))
            scaled_value = int(new_value_gallons * Pump.SCALING_FACTOR_FLOW_RATE)
            lower_bound = Bounds['FlowRate'][LOW] * Pump.SCALING_FACTOR_FLOW_RATE
            upper_bound = Bounds['FlowRate'][HIGH] * Pump.SCALING_FACTOR_FLOW_RATE
            (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetAinFlowRateHigh', old_value, scaled_value, lower_bound, upper_bound)
        else:
            message = 'Flow rate requires units'
    
    elif 'point' in attr_name:
        ''' Custom injection point name'''
        custom_name_match = re.match(r'^point_(?P<point_id>[0-9]{1,2})_custom$', attr_name)
        
        if custom_name_match:
            status = 200
            message = 'Custom name deleted'
            
            point_id = custom_name_match.group('point_id')
            new_name = re.sub(PointCustomization.CUSTOM_NAME_REGEX, '', new_value)
            
            # Auto-delete names if empty
            pump.pointcustomization_set.filter(point_id=point_id).delete()
            
            if len(re.sub(r'[\s]', '', new_name)) > 0:
                PointCustomization.objects.create(point_id=point_id, pump=pump, custom_point_name=new_name)
                message = 'Custom name saved'
                
            pump.save()  # Force an update to the pump object's modification timestamp so that the web page will show refreshed data
    
    elif 'well_status' in attr_name:
#        ''' well status bitfield '''
        old_value = pump.well_status
        lower_bound = Bounds['Status'][LOW]
        upper_bound = Bounds['Status'][HIGH]
        new_bit_value = int(new_value)
        attr_list = attr_name.split('_')
        bit_position = int(attr_list[2]) - 1
        if (new_bit_value > 0):
            new_bitfield = old_value | (1 << bit_position)
        else:
            new_bitfield = old_value & ~(1 << bit_position)
        (status, message, waiting_for_pump) = check_and_send_pump(pump_id_filtered, 'SetWellStatus', old_value, new_bitfield)
        
    else:
        '''Custom alarm name'''
        custom_name_match = re.match(r'^alarm_(?P<alarm_id>[0-9]{1,2})_custom$', attr_name)

        if custom_name_match:
            status = 200
            message = 'Custom name deleted'

            alarm_id = custom_name_match.group('alarm_id')
            new_name = re.sub(AlarmCustomization.CUSTOM_NAME_REGEX, '', new_value)

            # Auto-delete names if empty
            pump.alarmcustomization_set.filter(alarm_id=alarm_id).delete()

            if len(re.sub(r'[\s]', '', new_name)) > 0:
                AlarmCustomization.objects.create(alarm_id=alarm_id, pump=pump, custom_alarm_name=new_name)
                message = 'Custom name saved'

            pump.save()  # Force an update to the pump object's modification timestamp so that the web page will show refreshed data

    log_command(request, Log.TARGET_PUMP, pump_id, attr_name, new_value, status=status)

    return (status, message, waiting_for_pump)


def pump_helper_associate(request, customer_id):
    '''Associate a pump -- add a known pump to the specified group'''
    pump = None
    status = 200
    message = 'OK'
    response_dict = {}

    customer = get_customer_if_authorized(request, customer_id, require_manager=True)

    if not customer or customer.is_none_customer():
        status = 400
        message = 'Not authorized'
    else:
        if 'association_key' not in request.PUT:
            status = 400
            message = 'Missing required parameter'
        else:
            association_key = sanitize(re.sub(Pump.ASSOCIATION_KEY_FILTER, '', request.PUT['association_key']))

            '''
            Allow lookups based on the actual activation key. Also checks if a pump is already
            associated with another group
            '''
            pump = Pump.check_pump_for_user_registration(association_key)

            if not pump:
                status = 400
                message = 'Invalid association key or pump already registered'

    if pump:
        # Clear the association key to cause a new one to be set
        pump.activation_key = ''
        pump.customer = customer
        pump.save()

        if customer.has_valid_subscription():
            Payments.recalculate(customer.get_current_subscription())

        message = 'Pump associated'
        response_dict['unique_id'] = pump.unique_id

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.message = ''
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.user_actor = request.user
    log_entry.target_type = Log.TARGET_PUMP
    log_entry.action = Log.ACTION_ASSOCIATE

    if pump is not None:
        log_entry.attribute = 'customer_id'
        log_entry.new_value = pump.customer.id
        log_entry.target_id = pump.unique_id

    if status >= 200 and status < 300:
        log_entry.success = 1

    log_entry.save()

    return respond(request, status=status, message=message, response_dict=response_dict)


@really_defeat_caching
@login_required
def pumps_history_api_1_0(request, pump_id):
    '''Data for historical charting'''
    maximum_days = 180
    default_days = 30

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        return respond_error(request, status=400, message='Bad target')

    if not request.user.userprofile.is_admin() and not pump.customer.has_valid_subscription():
        status = 200
        message = "Subscription expired"
        return respond_error(request, status=status, message=message, flash_message=message)

    pump_dict = model_to_dict(pump)
    pump_dict['timestamp'] = int(pump.timestamp.timestamp())

    prefs_dict = {}
    units = request.user.userprofile.display_units
    prefs_dict['display_units'] = units

    response_dict = {'pump_obj': pump_dict,
                     'prefs': prefs_dict}
    
    scale_factor_tank_param = pump.get_tank_parameter_scale_factor()
    
    # Data plotted on the chart
    if 'chart_type' in request.GET and request.GET['chart_type'] in History.ACCEPTABLE_HISTORY_CHARTS:
        chart_type = request.GET['chart_type']
    else:
        return respond_error(request, status=400, message='Invalid chart type')

    # Date age range
    if 'chart_days' in request.GET:
        try:
            days_to_load = int(request.GET['chart_days'])
        except:
            return respond_error(request, status=400, message='Invalid day count')
    else:
        days_to_load = default_days

    if days_to_load > maximum_days:
        days_to_load = maximum_days

    # Fetch and reorganize the data
    if chart_type == 'monthly_report':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand', History.BY_MONTH, request.user.userprofile.time_zone)
        history_list = convert_totalizer_local(history, request, History.BY_MONTH)
    elif chart_type == 'totalizer_grand':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'battery_voltage':    
        history = History.get_history_all(pump.unique_id, chart_type, request.user.userprofile.time_zone, day_limit=days_to_load)
        history_list = convert_history_to_local(history, request, Pump.SCALING_FACTOR_BATT)
    elif chart_type == 'pressure_level':
        history = History.get_history_all(pump.unique_id, chart_type, request.user.userprofile.time_zone, day_limit=days_to_load)
        max_value = UnitsOfMeasure.convert_pressure(UnitsOfMeasure.IMPERIAL, units, Pump.MAX_PRESSURE_VALUE)
        history_list = convert_history_to_local(history, request, min_value=0, max_value=max_value, conversion_function=UnitsOfMeasure.convert_pressure)
    elif chart_type == 'tank_level':
        history = History.get_history_all(pump.unique_id, chart_type, request.user.userprofile.time_zone, day_limit=days_to_load)
        if pump_dict['tank_type'] > pump.TANK_UNKNOWN or pump_dict['sensor_type'] > pump.SENSOR_TANK_PERCENTAGE:
            max_value_unscaled = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, units, pump.tank_level_volume_max * pump.MAX_TANK_LEVEL_PERCENTAGE)
            max_value_intermediate = pump.get_tank_parameter_intermediate(max_value_unscaled)
            max_value_scaled = max_value_intermediate / Pump.SCALING_FACTOR_TANK_LEVEL
            history_list = convert_history_to_local(history, request, scaling_factor=scale_factor_tank_param, min_value=0, max_value=max_value_scaled, conversion_function=UnitsOfMeasure.convert_volume)
        elif pump_dict['sensor_type'] == pump.SENSOR_TANK_PERCENTAGE:
            max_value = Pump.MAX_PERCENTAGE_VALUE
            history_list = convert_history_to_local(history, request, min_value=0, max_value=max_value)
    elif chart_type == 'temperature':
        history = History.get_history_all(pump.unique_id, chart_type, request.user.userprofile.time_zone, day_limit=days_to_load)
        max_value = UnitsOfMeasure.convert_temperature(UnitsOfMeasure.IMPERIAL, units, Pump.MAX_TEMPERATURE_VALUE)
        history_list = convert_history_to_local(history, request, min_value=Pump.MIN_TEMPERATURE_VALUE, max_value=max_value, conversion_function=UnitsOfMeasure.convert_temperature)
    elif chart_type == 'totalizer_grand_well_1':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_1', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'totalizer_grand_well_2':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_2', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'totalizer_grand_well_3':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_3', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'totalizer_grand_well_4':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_4', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'totalizer_grand_well_5':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_5', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'totalizer_grand_well_6':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_6', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'totalizer_grand_well_7':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_7', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    elif chart_type == 'totalizer_grand_well_8':
        history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand_well_8', History.BY_DAY, request.user.userprofile.time_zone, num_periods=days_to_load)
        history_list = convert_totalizer_local(history, request, History.BY_DAY)
    else:
        return respond_error(request, status=400, message='Could not sort data')

    response_dict['history'] = history_list

    return respond(request, response_dict=response_dict)

def convert_totalizer_local(totalizer_history, request, period_length):
    '''Convert a list of historical totalizer flows per period into a list of flows per period converted to the correct units and local time zone'''
    output = []
    units = request.user.userprofile.display_units

    if period_length == History.BY_DAY:
        time_format = '%m/%d'
        day_offset = 0
    elif period_length == History.BY_MONTH:
        time_format = '%b %Y'
        day_offset = 15
    else:
        raise ValueError('Invalid period')

    for period_data in totalizer_history:
        tz = pytz.timezone(request.user.userprofile.time_zone)
        timestamp_offset = period_data[0] + relativedelta(days=day_offset)
        timestamp_pretty = timestamp_offset.astimezone(tz).strftime(time_format)

        if period_data[1] is None:
            period_flow = 0
        else:
            period_flow = float(period_data[1]) / Pump.SCALING_FACTOR
            period_flow = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL,
                                                        units,
                                                        period_flow)

        # Should never see negative flows, but just in case...
        if period_flow < 0:
            period_flow = 0

        output.append([timestamp_pretty, period_flow])

    return output


def convert_history_to_local(history, request, scaling_factor=1, min_value=None, max_value=None, conversion_function=None):
    '''Convert a list of historical daily activity to local time'''
    output = []
    units = request.user.userprofile.display_units

    for day_data in history:
        tz = pytz.timezone(request.user.userprofile.time_zone)
        timestamp_pretty = day_data[0].astimezone(tz).strftime('%m/%d/%Y %H:%M:%S')

        day_scaled = float(day_data[1]) / scaling_factor

        if conversion_function:
            day_scaled = conversion_function(UnitsOfMeasure.IMPERIAL, units, day_scaled)

        if min_value and day_scaled < min_value:
            continue
        elif max_value and day_scaled > max_value:
            continue
        else:
            output.append([timestamp_pretty, day_scaled])

    return output


def parse_pump(pump_obj, request):
    '''Interpret some aspects of the pump's state'''
    pump_parsed = None
    pump_last_seen = ''

    user_units = request.user.userprofile.display_units

    # These gymnastics are to match the firmware's strange way of rounding
    # flow_rate_unscaled = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.flow_rate)
    # if user_units == UnitsOfMeasure.METRIC:
    #    flow_rate_intermediate = int((flow_rate_unscaled + Pump.SCALING_FACTOR / 2) / Pump.SCALING_FACTOR)
    #    flow_rate_str = '%d.%d' % (int(flow_rate_intermediate / Pump.SCALING_FACTOR), int(flow_rate_intermediate % Pump.SCALING_FACTOR))
    # else:
    #    flow_rate_str = '%d.%02d' % (int(flow_rate_unscaled / Pump.SCALING_FACTOR_FLOW_RATE), int(flow_rate_unscaled % Pump.SCALING_FACTOR_FLOW_RATE))

    if pump_obj.tank_type > Pump.TANK_UNKNOWN or pump_obj.sensor_type > Pump.SENSOR_TANK_PERCENTAGE:
        tank_level_unscaled = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.tank_level)
        tank_level_intermediate = pump_obj.get_tank_parameter_intermediate(tank_level_unscaled)
        tank_level_max_unscaled = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.tank_level_volume_max)
        tank_level_max_intermediate = pump_obj.get_tank_parameter_intermediate(tank_level_max_unscaled)
        tank_level_str = '%d.%d of %d.%d' % (int(tank_level_intermediate / Pump.SCALING_FACTOR_TANK_LEVEL), int(tank_level_intermediate % Pump.SCALING_FACTOR_TANK_LEVEL),
                                            int(tank_level_max_intermediate / Pump.SCALING_FACTOR_TANK_LEVEL), int(tank_level_max_intermediate % Pump.SCALING_FACTOR_TANK_LEVEL))
        
        tank_level_notify_unscaled = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.tank_level_notify_trigger)                               
        tank_level_notify_intermediate = pump_obj.get_tank_parameter_intermediate(tank_level_notify_unscaled)
        tank_level_notify_str = '%d.%d' % (int(tank_level_notify_intermediate / Pump.SCALING_FACTOR_TANK_LEVEL), int(tank_level_notify_intermediate % Pump.SCALING_FACTOR_TANK_LEVEL))
        
        tank_level_shutoff_unscaled = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.tank_level_shutoff_trigger)                               
        tank_level_shutoff_intermediate = pump_obj.get_tank_parameter_intermediate(tank_level_shutoff_unscaled)
        tank_level_shutoff_str = '%d.%d' % (int(tank_level_shutoff_intermediate / Pump.SCALING_FACTOR_TANK_LEVEL), int(tank_level_shutoff_intermediate % Pump.SCALING_FACTOR_TANK_LEVEL))        
    else:
        if pump_obj.tank_level_volume_max > 0:
            tank_level_str = '%d' % ((pump_obj.tank_level * 100) / pump_obj.tank_level_volume_max)
            tank_level_notify_str = '%d' % ((pump_obj.tank_level_notify_trigger * 100) / pump_obj.tank_level_volume_max)
            tank_level_shutoff_str = '%d' % ((pump_obj.tank_level_shutoff_trigger * 100) / pump_obj.tank_level_volume_max)            
        else:
            tank_level_str = '%d' % ((pump_obj.tank_level * 100) / 100)
            tank_level_notify_str = '%d' % ((pump_obj.tank_level_notify_trigger * 100) / 100)
            tank_level_shutoff_str = '%d' % ((pump_obj.tank_level_shutoff_trigger * 100) / 100)

    active_alarms = alarm_bitfield_to_list(pump_obj.alarms_status)

    if pump_obj:
        pump_parsed = {'connection': PrettyPump['connection'][int(pump_obj.connection)],
                       'alarms_status': alarm_list_to_display(pump_obj, active_alarms),
                       'metering_mode': Pump.METERING_MODES[pump_obj.metering_mode],
                       'flow_rate': pump_obj.get_flow_rate_scaled(pump_obj.flow_rate, user_units),
                       'totalizer_grand': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand / Pump.SCALING_FACTOR),
                       'totalizer_resetable': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_resetable / Pump.SCALING_FACTOR),
                       'pressure_level': UnitsOfMeasure.convert_pressure(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.pressure_level),
                       'battery_voltage': pump_obj.battery_voltage / Pump.SCALING_FACTOR_BATT,
                       'high_pressure_trigger': UnitsOfMeasure.convert_pressure(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.high_pressure_trigger),
                       'low_pressure_trigger': UnitsOfMeasure.convert_pressure(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.low_pressure_trigger),
                       'low_battery_trigger': pump_obj.low_battery_trigger / Pump.SCALING_FACTOR_BATT,
                       'battery_warning_trigger': pump_obj.battery_warning_trigger / Pump.SCALING_FACTOR_BATT,
                       'location_reported': pump_obj.get_lat_long(),
                       'power_save_mode': Pump.POWER_SAVE_MODES.get(pump_obj.power_save_mode, 'Unknown'),
                       'subscription': pump_obj.has_valid_subscription(),
                       'can_suspend': pump_obj.can_suspend(),
                       'can_unsuspend': pump_obj.can_unsuspend(),
                       'sensor_type': Pump.SENSOR_TYPES[pump_obj.sensor_type],
                       'tank_type': Pump.TANK_TYPES[pump_obj.tank_type],
                       'tank_level': tank_level_str,
                       'tank_level_notify_trigger': tank_level_notify_str,
                       'tank_level_shutoff_trigger': tank_level_shutoff_str,
                       'analog_flow_rate': pump_obj.get_flow_rate_scaled(pump_obj.flow_rate, user_units),
                       'temperature': UnitsOfMeasure.convert_temperature(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.temperature),
                       'temperature_control': Pump.TEMP_CONTROL_OPTIONS[pump_obj.temperature_control],
                       'temperature_setpoint': UnitsOfMeasure.convert_temperature(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.temperature_setpoint),
                       'wellsite_flow_rate': pump_obj.get_flow_rate_scaled(pump_obj.flow_rate, user_units),
                       'totalizer_grand_wellsite': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand / Pump.SCALING_FACTOR),
                       'totalizer_resetable_wellsite': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_resetable / Pump.SCALING_FACTOR),
                       'well_flow_rate_1': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_1, user_units),
                       'well_flow_rate_2': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_2, user_units),
                       'well_flow_rate_3': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_3, user_units),
                       'well_flow_rate_4': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_4, user_units),
                       'well_flow_rate_5': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_5, user_units),
                       'well_flow_rate_6': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_6, user_units),
                       'well_flow_rate_7': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_7, user_units),
                       'well_flow_rate_8': pump_obj.get_flow_rate_scaled(pump_obj.well_flow_rate_8, user_units),
                       'totalizer_well_1': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_1 / Pump.SCALING_FACTOR),
                       'totalizer_well_2': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_2 / Pump.SCALING_FACTOR),
                       'totalizer_well_3': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_3 / Pump.SCALING_FACTOR),
                       'totalizer_well_4': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_4 / Pump.SCALING_FACTOR),
                       'totalizer_well_5': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_5 / Pump.SCALING_FACTOR),
                       'totalizer_well_6': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_6 / Pump.SCALING_FACTOR),
                       'totalizer_well_7': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_7 / Pump.SCALING_FACTOR),
                       'totalizer_well_8': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_well_8 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_1': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_1 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_2': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_2 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_3': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_3 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_4': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_4 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_5': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_5 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_6': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_6 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_7': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_7 / Pump.SCALING_FACTOR),
                       'totalizer_grand_well_8': UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, pump_obj.totalizer_grand_well_8 / Pump.SCALING_FACTOR),
                       'well_status_1': (pump_obj.well_status >> 0) & 0b1,
                       'well_status_2': (pump_obj.well_status >> 1) & 0b1,
                       'well_status_3': (pump_obj.well_status >> 2) & 0b1,
                       'well_status_4': (pump_obj.well_status >> 3) & 0b1,
                       'well_status_5': (pump_obj.well_status >> 4) & 0b1,
                       'well_status_6': (pump_obj.well_status >> 5) & 0b1,
                       'well_status_7': (pump_obj.well_status >> 6) & 0b1,
                       'well_status_8': (pump_obj.well_status >> 7) & 0b1,
                       'analog_input_mode': Pump.AIN_MODES[pump_obj.analog_input_mode],
                       'ain_mA_low': pump_obj.ain_mA_low / Pump.SCALING_FACTOR_MA,
                       'ain_mA_high': pump_obj.ain_mA_high / Pump.SCALING_FACTOR_MA,
                       'ain_flow_rate_low': pump_obj.get_flow_rate_scaled(pump_obj.ain_flow_rate_low, user_units),
                       'ain_flow_rate_high': pump_obj.get_flow_rate_scaled(pump_obj.ain_flow_rate_high, user_units),
                       }

        '''Pump last-seen feature'''
        try:
            # Filter out the pumps with a default "last seen" timestamp
            if pump_obj.last_seen and pump_obj.last_seen.year >= 2015:
                tz = pytz.timezone(request.user.userprofile.time_zone)
                pump_last_seen = pump_obj.last_seen.astimezone(tz).strftime('%Y-%m-%d %X %Z')
        except:
            pass
        pump_parsed['last_seen'] = pump_last_seen

        '''Pump custom alarm names, not just the active ones'''
        custom_alarm_names = pump_obj.alarmcustomization_set.all()
        if len(custom_alarm_names) > 0:
            pump_parsed['custom_alarm_names'] = {}
            for name_obj in custom_alarm_names:
                pump_parsed['custom_alarm_names']['alarm_%u_custom' % name_obj.alarm_id] = name_obj.custom_alarm_name
                
        '''Pump injection point names, not just the active ones'''
        custom_point_names = pump_obj.pointcustomization_set.all()
        if len(custom_point_names) > 0:
            pump_parsed['custom_injection_point_names'] = {}
            for name_obj in custom_point_names:
                pump_parsed['custom_injection_point_names']['point_%u_custom' % name_obj.point_id] = name_obj.custom_point_name

        '''Pump customer data'''
        try:
            pump_parsed['customer'] = pump_obj.customer.organization_name
        except:
            pump_parsed['customer'] = 'Unknown'

        '''Admin-only data'''
        if request.user.userprofile.is_admin() or (request.user.userprofile.is_distributor() and request.user == pump_obj.customer.manager):
            pump_parsed['activation_key'] = pump_obj.activation_key

    return pump_parsed


def alarm_list_to_display(pump_obj, active_alarm_list):
    '''Convert an active-alarms list to a string for display'''
    retval = 'None'

    if len(active_alarm_list) > 0:
        out_list = []
        custom_names = pump_obj.alarmcustomization_set

        for alarm_id in active_alarm_list:
            # Check the alarm_id against the list of custom options
            custom_obj = custom_names.filter(alarm_id=alarm_id).first()

            # Use the custom name if it exists, or else use the default name
            if custom_obj:
                out_list.append(custom_obj.custom_alarm_name)
            else:
                out_list.append(AlarmList[alarm_id])

        retval = ', '.join(out_list)

    return retval


def check_and_send_pump(pump_id_filtered, command, old_value, new_value, lower_bound=-1, upper_bound=-1, retain=False):
    '''Send command to the pump, but only if it has changed and is in bounds'''
    status = 200
    message = 'No change required'
    waiting_for_pump = False

    if new_value != old_value:
        if (lower_bound != -1 and int(new_value) < lower_bound) or (upper_bound != -1 and int(new_value) > upper_bound):
            status = 416  # out of range
            message = 'Input out of bounds'
        else:
            try:
                PumpSet.set_pump(pump_id_filtered, command, new_value, retain)
                status = 201  # accepted and queued
                message = 'Waiting for pump...'
                waiting_for_pump = True
            except Exception as e:
                log_message = 'Error setting pump parameter: %s' % (repr(e),)
                print(log_message)
                status = 500

                log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                               event_type=Log.EVENT_COMMAND,
                                               action=Log.ACTION_UPDATE,
                                               success=Log.STATUS_FAIL,
                                               message=sanitize(log_message),
                                               target_type=Log.TARGET_PUMP,
                                               target_id=pump_id_filtered,
                                               attribute=sanitize(command),
                                               new_value=sanitize(new_value),
                                               )
                log_entry.save()

                message = 'Error communicating with pump'

    return (status, message, waiting_for_pump)


@really_defeat_caching
def aeris_api_1_0(request, pump_id=None):
    response = None

    try:
        # Do this instead of using the @login_required decorator so that we can send a failure status code
        if not request.user.is_authenticated():
            response = respond_not_logged_in(request)

            # Handle pump data query
        elif pump_id and request.method == 'GET':
            response = aeris_helper_fetch(request, pump_id)

        # Handle pump settings change
        elif pump_id and request.method == 'POST':
            response = aeris_helper_change(request, pump_id)

        else:
            response = respond_error(request, status=400, message='Bad command or target')

    except Exception as e:
        log_message = repr(e)
        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_API_ERROR,
                                       action=Log.ACTION_UNKNOWN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=sanitize(log_message),
                                       target_type=Log.TARGET_DEBUG,
                                       attribute='aeris',
                                       )
        log_entry.save()
        response = respond_error(request, status=500, message='Server error')

    return response


def aeris_helper_fetch(request, pump_id):
    '''
    Fetch Aeris API status data for the specified pump
    '''
    response_dict = {}

    if not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        return respond_error(request, status=410, message='Bad target')
    pump_dict = model_to_dict(pump, exclude='mqtt_auth')

    aeris_status, _ = aeris_get_status(pump)
    if not aeris_status or not aeris_status.data_retrieved:
        return respond_error(request, status=400, message='Aeris API error', flash_message='Aeris API error')

    prefs_dict = {'display_units': request.user.userprofile.display_units}
    tz = pytz.timezone(request.user.userprofile.time_zone)

    aeris_dict = {}
    for k, v in aeris_status.__dict__.items():
        if type(v) is datetime:
            aeris_dict[k] = v.astimezone(tz).strftime('%Y-%m-%d %X %Z')
        elif v is None:
            aeris_dict[k] = '?'
        else:
            aeris_dict[k] = v

    # Return the values stored in the container class
    response_dict['aeris'] = aeris_dict
    response_dict['pump_obj'] = pump_dict
    response_dict['prefs'] = prefs_dict

    return respond(request, response_dict=response_dict)


def aeris_helper_change(request, pump_id):
    '''
    Handle changes to the Aeris configuration for the given pump (mostly via pseudo-fields)
    '''
    status = 200
    message = 'Success'
    waiting_for_pump = False

    if not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        return respond_error(request, status=410, message='Bad target')

    try:
        new_value = sanitize(request.POST['new_value'])
        attr_name = sanitize(request.POST['attr_name'])

        if 'clear_registration' == attr_name:
            try:
                if not aeris_clear_registration(pump):
                    status = 400
            except:
                status = 400

            if status == 400:
                message = 'Could not clear registration'
            else:
                message = 'Registration cleared'

        log_command(request, Log.TARGET_PUMP, pump_id, attr_name, new_value, status=status)

    except:
        status = 500
        message = 'Error while processing Aeris command'

    response_dict = {'waiting_for_pump': waiting_for_pump}
    return respond(request, status=status, message=message, response_dict=response_dict)
