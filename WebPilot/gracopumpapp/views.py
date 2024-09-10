import datetime
import gzip
import json
import math
import platform
import random
import re
from subprocess import TimeoutExpired
import subprocess

from django.conf import settings
from django.contrib import messages
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.decorators import login_required
from django.contrib.auth.models import User
from django.urls import reverse
from django.db.models import Sum
from django.forms.models import model_to_dict
from django.http.response import HttpResponse
from django.shortcuts import render, redirect
from django.template import RequestContext
from django.utils import timezone
from django.views.decorators.csrf import ensure_csrf_cookie, csrf_exempt
from gracopumpapp.decorators import check_tos, check_email
from gracopumpapp.models import Pump, Log, History, Customer, UserProfile, \
    UnitsOfMeasure, Notification, get_none_customer, \
    TermsOfService, Invitation, AlarmWork, Subscription, Plan, PaymentAccount, \
    AlarmPreference, SitePreference
from gracopumpapp.payments import Payments
from gracopumpapp.render_helpers import respond_not_authorized, respond_error, get_pump_if_authorized, \
    get_customer_if_authorized, ip_in_allowed_subnets, sanitize
import pytz
import recurly
from recurly.errors import NotFoundError


def index(request):
    response_dict = {}

    if 'bad_pw' in request.GET:
        messages.error(request, 'Bad username or password', 'danger')
    elif 'not_active' in request.GET:
        messages.error(request, 'Account not active', 'danger')

    if request.user.is_authenticated():
        return redirect('pump_list')
    else:
        if 'next' in request.GET:
            next_filtered = re.sub(r'[><\'";]', '', request.GET['next'])
            response_dict['next'] = next_filtered
            messages.warning(request, 'You need to log in first to access that page')
        return render(request, 'index.html', response_dict)


def _login(request):
    """Handle only login requests. The actual form is on the index page."""
    bad_pw = False
    username = None
    not_active = False

    if 'username' in request.POST and 'password' in request.POST:
        username = request.POST['username']
        password = request.POST['password']
        remember_me = request.POST.get('remember_me', False)
        user = authenticate(username=username, password=password)

        # Allow the user the option to log in with an email address instead
        if user is None:
            try:
                # Slight hack to avoid changing the underlying user model. Use the 'get'
                # query to avoid one-to-many result problems
                username_lookup = User.objects.get(email__iexact=username)
                if username_lookup:
                    user = authenticate(username=username_lookup.username, password=password)
            except:
                pass

        if user is not None and user.is_active:
            if not remember_me:
                # Expire session on browser close
                request.session.set_expiry(0)
            login(request, user)
            timezone.activate(pytz.timezone(user.userprofile.time_zone))
        else:
            if user is not None and not user.is_active:
                not_active = True
            else:
                bad_pw = True

    if request.user.is_authenticated():
        if 'next' in request.POST:
            next_filtered = re.sub(r'[^-_0-9a-zA-Z/]', '', request.POST['next'])
            return redirect(next_filtered)
        else:
            return redirect('pump_list')
    else:
        response = redirect('/')
        attribute = ''
        new_value = ''
        log_message = ''

        if username:
            attribute = 'username'
            new_value += re.sub(UserProfile.USERNAME_REGEX, '', username)

        if bad_pw:
            response['Location'] += '?bad_pw=1'
            log_message = 'Wrong u or p'
        elif not_active:
            response['location'] += '?not_active=1'
            log_message = 'Not active'

        if 'next' in request.POST:
            next_filtered = re.sub(r'[><\'";]', '', request.POST['next'])
            response['Location'] += '&next=%s' % (next_filtered,)

        log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                       event_type=Log.EVENT_LOGIN,
                                       action=Log.ACTION_LOGIN,
                                       success=Log.STATUS_FAIL,
                                       origin_ip=request.META['REMOTE_ADDR'],
                                       message=log_message,
                                       target_type=Log.TARGET_USER,
                                       attribute=attribute,
                                       new_value=new_value)
        log_entry.save()

        return response


def _logout(request):
    logout(request)
    return redirect('/')


def csrf_failure(request, reason=""):
    '''CSRF failure view'''
    resp_dict = {'error_number': '403',
                 'error_explanation': 'Forbidden: CSRF failure'}
    response = render(request, 'error.html', resp_dict)
    response.status_code = 403
    return response


def handler401(request, *args, **kwargs):
    '''401 page generator'''
    resp_dict = {'error_number': '401',
                 'error_explanation': 'Not authorized'}
    response = render(request, 'error.html', resp_dict)
    response.status_code = 401
    return response


def handler403(request, *args, **kwargs):
    '''403 page generator'''
    resp_dict = {'error_number': '403',
                 'error_explanation': 'Forbidden'}
    response = render(request, 'error.html', resp_dict)
    response.status_code = 403
    return response


def handler404(request, *args, **kwargs):
    '''404 page generator'''
    resp_dict = {'error_number': '404',
                 'error_explanation': 'Sorry, we could not find that page.'}
    response = render(request, 'error.html', resp_dict)
    response.status_code = 404
    return response


def handler500(request, *args, **kwargs):
    '''500 page generator'''
    resp_dict = {'error_number': '500',
                 'error_explanation': 'Sorry, there was a server error.'}
    response = render(request, 'error.html', resp_dict)
    response.status_code = 500

    # TODO: Also fire off a notification that this was encountered

    return response


def cron(request):
    '''
    Endpoint that will be hit by a cron job periodically so that things like scheduled
    notifications or other async, possibly long-running, tasks can be done without requiring
    some sort of separate external system.

    The expectation is that this endpoint would be hit fairly frequently, perhaps once a
    minute or more.
    '''
    time_now = datetime.datetime.utcnow().replace(tzinfo=pytz.utc)
    message = ''

    '''
    Restrict access to localhost only.

    For this to work correctly, the curl cron job that hits the cron endpoint must be configured to
    originate its requests on the localhost interface
    '''
    allowed_cron_ips = ['127.0.0.1', '198.58.118.79']
    if request.META['REMOTE_ADDR'] not in allowed_cron_ips:
        print('Invalid IP attempting to access site: %s' % (repr(request.META['REMOTE_ADDR']),))
        # Pretend the page doesn't exist at all if access denied
        return handler404(request)

    # Check for any alarm alerts that are due
    try:
        message += AlarmWork.service_all(time_now)
    except Exception as e:
        message += 'Exception caught processing alarm work (%s)' % (repr(e),)

    message += ' '

    # Check for any notifications that are due
    try:
        message += Notification.service_all(time_now)
    except Exception as e:
        message += 'Exception caught processing notifications (%s)' % (repr(e),)

    message += ' '

    # Update activation keys
    try:
        message += Pump.check_activation_keys(time_now)
    except Exception as e:
        message += 'Exception caught processing activation keys (%s)' % (repr(e),)

    message += ' '

    # Find disconnected pumps for notifications
    try:
        message += Pump.find_pumps_for_disconnect_alarm(time_now)
    except Exception as e:
        message += 'Exception caught searching for disconnectd pumps (%s)' % (repr(e),)
    
    message += ' '
        
    # Check the flow verify alarm
    try:
        message += Pump.check_flow_verify(time_now)
    except Exception as e:
        message += 'Exception caught processing the flow verify alarm (%s)' % (repr(e),)

    message += ' '

    return HttpResponse(message)


@csrf_exempt
def recurly_webhook(request):
    '''Endpoing for Recurly webhooks'''
    message = 'Error'
    status = 400

    # Recurly requests will always come from one of these addresses, per the Recurly webhook docs
    allowed_subnets = ['127.0.0.1',  # for testing
                       '74.201.212.175',
                       '64.74.141.175',
                       '75.98.92.102',
                       '174.51.224.59',
                       '74.201.212.0/24',
                       '64.74.141.0/24',
                       '75.98.92.96/28',
                       '8.36.93.0/24',
                       '50.18.192.88',
                       '52.8.32.100',
                       '52.9.209.233',
                       '50.0.172.150',
                       '52.203.102.94',
                       '52.203.192.184',
                       '35.233.168.62',
                       '35.185.253.62',
                       '35.188.232.138',
                       '35.236.210.191',
                       '35.194.77.56',
                       '34.86.76.227',
                       '34.86.231.208',
                       '35.245.149.42',
                       '34.105.107.15',
                       '35.230.60.156',
                       '35.197.126.78',
                       '35.199.174.181',
                       '76.120.106.199',
                       ]

    if not ip_in_allowed_subnets(request.META['REMOTE_ADDR'], allowed_subnets):
        # Pretend the page doesn't exist at all if access denied
        message = 'Invalid IP attempting to access Recurly webhook'
        Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                           event_type=Log.EVENT_PAYMENTS,
                           action=Log.ACTION_WEBHOOK,
                           success=Log.STATUS_FAIL,
                           origin_ip=request.META['REMOTE_ADDR'],
                           message=message,
                           target_type=Log.TARGET_PAYMENT,
                           )

        return handler404(request)

    try:
        # Parse and handle the notification
        notif = recurly.objects_for_push_notification(request.body)

        if not Payments.process_webhook(notif):
            print('Unrecognized notification received: %s' % (notif['type'],))
            message = 'Error'
            status = 400
        else:
            # We were successful if we got this far with no exceptions
            message = 'OK'
            status = 200

        log_status = Log.STATUS_SUCCESS if status == 200 else Log.STATUS_FAIL
        Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                           event_type=Log.EVENT_PAYMENTS,
                           action=Log.ACTION_WEBHOOK,
                           success=log_status,
                           origin_ip=request.META['REMOTE_ADDR'],
                           message=message,
                           target_type=Log.TARGET_PAYMENT,
                           attribute=sanitize(notif['type']),
                           )

    except Exception as e:
        notif_type = ''
        if notif and 'type' in notif:
            notif_type = repr(notif['type'])

        Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                           event_type=Log.EVENT_PAYMENTS,
                           action=Log.ACTION_WEBHOOK,
                           success=Log.STATUS_FAIL,
                           origin_ip=request.META['REMOTE_ADDR'],
                           message=sanitize(repr(e)),
                           target_type=Log.TARGET_PAYMENT,
                           attribute=sanitize(notif_type),
                           )

    return HttpResponse(message, status=status)


@login_required
@check_email
def help_page(request):
    '''Help page'''
    return render(request, 'help.html', {})


@login_required
@check_email
def admin(request):
    NORMAL_SYSTEM_THRESHOLD = 900

    if not request.user.userprofile.is_admin():
        return redirect('/')

    # System status is based on having evidence of recent updates from the mqtt/db worker
    age_of_recent_update = History.get_time_since_last_update()
    if age_of_recent_update < NORMAL_SYSTEM_THRESHOLD:
        mqtt_db_status = '%d seconds since last report (normal)' % age_of_recent_update
    else:
        mqtt_db_status = '%d seconds since last report (a bit long?)' % age_of_recent_update

    stats = {}
    stats['pumps_known'] = Pump.objects.filter(is_active=True).count()
    stats['pumps_connected'] = Pump.objects.filter(is_active=True, connection=True).count()

    return render(request, 'admin.html', {'mqtt_db_status': mqtt_db_status, 'stats': stats})


@login_required
@check_email
def admin_event_log(request):
    if not request.user.userprofile.is_admin():
        return respond_not_authorized(request)

    id_filter = None
    if 'id_filter' in request.GET:
        id_filter = re.sub(r'[^-_a-zA-Z0-9*]', '', request.GET['id_filter'])

    page_size = Log.DEFAULT_EVENT_LOG_PAGE_SIZE
    if 'page_size' in request.GET:
        page_size = int(request.GET['page_size'])
    if page_size > Log.EVENT_LOG_MAX_PAGE_SIZE:
        page_size = Log.EVENT_LOG_MAX_PAGE_SIZE
    if page_size < Log.EVENT_LOG_MIN_PAGE_SIZE:
        page_size = Log.EVENT_LOG_MIN_PAGE_SIZE

    page = 0
    if 'page' in request.GET:
        page = int(request.GET['page'])
    if page < 0:
        page = 0

    target_type = None
    if 'target_type' in request.GET:
        target_type = int(request.GET['target_type'])

    message_contains = None
    if 'message_contains' in request.GET:
        message_contains = sanitize(request.GET['message_contains'])

    message_exclude = None
    if 'message_exclude' in request.GET:
        message_exclude = sanitize(request.GET['message_exclude'])

    exclude_common = None
    if 'exclude_common' in request.GET:
        exclude_common = True

    # Download option
    if 'format' in request.GET and request.GET['format'] == 'download':
        log_name = 'graco-web-events-%s' % datetime.datetime.now()
        log_name = re.sub(' ', '_', log_name)

        all_log_entries = Log.fetch_filtered(request, paginate=False).order_by('pk')

        response = log_download_helper(all_log_entries, log_name)
        return response

    # View logs
    else:
        log_download_url = reverse('admin_event_log')
        log_download_url += '?format=download'

        if id_filter:
            log_download_url += '&id_filter=%s' % (id_filter,)

        if target_type:
            log_download_url += '&target_type=%s' % (target_type,)

        return render(request, 'admin_event_log.html', {'page_size': page_size,
                                                        'page': page,
                                                        'target_type': target_type,
                                                        'id_filter': id_filter,
                                                        'message_contains': message_contains,
                                                        'message_exclude': message_exclude,
                                                        'exclude_common': exclude_common,
                                                        'log_download_url': log_download_url,
                                                        'event_types': json.dumps(Log.EVENTS),
                                                        'origin_types': json.dumps(Log.ORIGINS),
                                                        'action_types': json.dumps(Log.ACTIONS),
                                                        'status_types': json.dumps(Log.STATUS),
                                                        'target_types': json.dumps(Log.TARGETS),
                                                        'extra_content_div_class': 'clear-max-width',
                                                        })


@login_required
def admin_history_log_download(request):
    '''Send the history log as a downloadable file'''
    if not request.user.userprofile.is_admin():
        return redirect('/')

    log_name = 'graco-pump-history-%s' % datetime.datetime.now()
    log_name = re.sub(' ', '_', log_name)

    all_log_entries = History.objects.order_by('pk')

    response = log_download_helper(all_log_entries, log_name)
    return response


@login_required
def admin_database_backup_download(request):
    '''Generate a backup of the database and offer it for download'''
    if not request.user.userprofile.is_admin():
        return redirect('/')

    file_name = 'harrier-db-backup-%s' % datetime.datetime.now()
    success = False
    message = ''

    if settings.DEBUG:
        raw_dump = 'Test' + ''.join(random.SystemRandom().sample('A' * 1000, 201))
        outs = raw_dump.encode()
        compressed_output = gzip.compress(outs)

        success = True
    else:
        try:
            # Generate the mysqldump command-line arguments
            db_settings = settings.DATABASES['default']
            db_name = db_settings['NAME']
            db_username = db_settings['USER']
            db_password = '%s%s' % (db_settings['PASSWORD'], '\n')
            args = ['/usr/bin/mysqldump', '-u', db_username, '-p', '--opt', db_name]
            args_gzip = ['/bin/gzip']

            # Dump and compress in one shot due to the very large size of the backup
            proc = subprocess.Popen(' '.join(args) + ' | ' + ' '.join(args_gzip), stdout=subprocess.PIPE, stdin=subprocess.PIPE, shell=True)

            # Read the compressed output
            compressed_output,_ = proc.communicate(input=db_password.encode(), timeout=60*3)

            success = True
            message = 'Database Backup generated'
        except TimeoutExpired:
            # Cleanup pattern, per the subprocess docs
            proc.kill()
            compressed_output, _ = proc.communicate()
            message = 'Database backup timed out'
        except:
            proc.kill()
            compressed_output, _ = proc.communicate()
            message = 'Database backup failed for unknown reasons'

    # Sanity check
    if success and len(compressed_output) < 1:
        message = 'Database backup unusually short'
        success = False

    if success:
        response = HttpResponse(compressed_output, content_type='application/octet-stream')
        response['Content-Encoding'] = 'application/octet-stream'
        response['Content-Length'] = str(len(compressed_output))
        response['Content-Disposition'] = 'attachment; filename="%s.sql.gz"' % file_name
    else:
        response = handler500(request)

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_BACKUP, action=Log.ACTION_CREATE)
    log_entry.success = Log.STATUS_SUCCESS if success else Log.STATUS_FAIL
    log_entry.user_actor = request.user
    log_entry.message = message
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.save()

    return response


@login_required
def admin_testing(request):
    '''Tools for development testing'''
    if not request.user.userprofile.is_admin():
        return redirect('/')

    return render(request, 'admin_testing.html')


@login_required
@check_tos
@check_email
def customer_list(request):
    '''List of customers'''
    return render(request, 'group_list.html', {})


@login_required
@check_tos
@check_email
def customer_settings(request, customer_id=None):
    '''Customer settings page'''
    customer = get_customer_if_authorized(request, customer_id)

    if not customer:
        messages.error(request, 'Group not found', 'danger')
        return redirect('customer_list')

    # Must not edit "none" customer
    response_dict = {'customer_id': customer.pk,
                     'organization_name': customer.organization_name,
                     'subscription_global_override': customer.subscription_global_override(),
                     }

    # The None group shouldn't have managers
    if customer.id == get_none_customer():
        response_dict['is_none_customer'] = True

    if customer.manager == request.user or request.user.userprofile.is_admin():
        response_dict['is_manager'] = True

    return render(request, 'group_settings.html', response_dict)


@login_required
@check_tos
@check_email
def customer_users(request, customer_id):
    '''List all of the users associated with this customer'''
    response_dict = {}

    customer = get_customer_if_authorized(request, customer_id)
    if not customer:
        messages.error(request, 'Group not found', 'danger')
        return redirect('customer_list')

    response_dict['mode'] = 'customer'
    response_dict['customer_name'] = customer.organization_name
    response_dict['customer_id'] = customer.id
    response_dict['user_id'] = request.user.id
    response_dict['is_manager'] = (request.user == customer.manager)
    response_dict['none_customer_id'] = get_none_customer()

    response_dict['is_none_customer'] = (customer.id == get_none_customer())

    return render(request, 'user_list.html', response_dict)


@login_required
@check_tos
@check_email
def customer_pumps(request, customer_id):
    '''List all of the pumps associated with this customer'''
    response_dict = {}

    customer = get_customer_if_authorized(request, customer_id)
    if not customer:
        messages.error(request, 'Group not found', 'danger')
        return redirect('customer_list')

    response_dict['mode'] = 'customer'
    response_dict['customer_name'] = customer.organization_name
    response_dict['customer_id'] = customer.id
    response_dict['is_manager'] = (request.user == customer.manager)
    response_dict['user_id'] = request.user.id
    response_dict['none_customer_id'] = get_none_customer()

    response_dict['is_none_customer'] = (customer.id == get_none_customer())

    return render(request, 'pump_list.html', response_dict)


@login_required
@check_tos
@check_email
def customer_create(request):
    if not request.user.userprofile.is_admin() and not request.user.userprofile.is_distributor():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    return render(request, 'group_create.html', {})


@login_required
@check_tos
@check_email
def customer_invite(request, customer_id):
    '''Allow admins and account managers to invite people into this group'''
    response_dict = {}

    # The no-ID case is present just for URL generation, not for actual requests
    if not customer_id:
        return respond_error(request)

    # ONly managers and admins can issue group invitations
    customer = get_customer_if_authorized(request, customer_id, require_manager=True)
    if not customer:
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    response_dict['customer'] = customer

    if not request.user.userprofile.is_admin() and not customer.has_valid_subscription():
        messages.error(request, 'Feature unavailable due to expired subscription', 'danger')
        return redirect('customer_settings', customer_id)

    return render(request, 'group_user_invite.html', response_dict)


@login_required
@check_tos
@check_email
def customer_subscription(request, customer_id):
    '''Managers and admins can see and manage subscriptions for this group'''
    response_dict = {}
    tz = pytz.timezone(request.user.userprofile.time_zone)

    customer = get_customer_if_authorized(request, customer_id, require_manager=True)
    if not customer:
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    # Don't let groups see subscriptions if they're overridden for that group
    if customer.subscription_overridden():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    response_dict['customer'] = customer

    subscription = customer.get_current_subscription()

    response_dict['valid_subscription'] = customer.has_valid_subscription()

    if subscription is None:
        response_dict['status_text'] = 'No subscription found'
        response_dict['expiration_text'] = '???'
        response_dict['plan_text'] = '????'
        response_dict['plan_button_text'] = 'Start subscription'
        response_dict['suspended_pump_count'] = '%u' % customer.suspended_pump_count()
        response_dict['active_pump_count'] = '%u' % customer.active_pump_count()
        response_dict['at_least_one_pump'] = (customer.active_pump_count() + customer.suspended_pump_count() > 0)
    else:
        # Strip the leading zero when we can
        if platform.system() == 'Windows':
            date_format = '%b %d, %Y'
        else:
            date_format = '%b %-d, %Y'

        response_dict['suspended_pump_count'] = '%u' % customer.suspended_pump_count()
        response_dict['active_pump_count'] = '%u' % customer.active_pump_count()
        response_dict['charge_per_pump'] = 'US$ {:.2f}'.format(subscription.plan.price_cents / 100.0)
        response_dict['total_charge'] = 'US$ {:.2f}'.format(subscription.monthly_charge())

        response_dict['status_text'] = Subscription.STATUS_TEXT.get(subscription.status, 'Unrecognized')
        response_dict['expiration_text'] = subscription.expiration.astimezone(tz).strftime(date_format) if subscription.expiration else '????'
        if subscription.trial_end:
            trial_time_remaining = subscription.trial_end.replace(tzinfo=None) - datetime.datetime.utcnow()
            if trial_time_remaining.total_seconds() > 0:
                response_dict['trial_days_remaining'] = str(math.ceil((trial_time_remaining.total_seconds() / 86400)))
        response_dict['plan_text'] = subscription.plan.name if subscription.plan else '????'
        response_dict['plan_button_text'] = 'Change billing'
        response_dict['at_least_one_pump'] = (customer.active_pump_count() + customer.suspended_pump_count() > 0)

    '''
    Show account balance if non-zero even if there isn't currently a valid subscription

    The most recent payment account should be the most correct
    '''
    payment_accounts = customer.get_all_payment_accounts(user=customer.manager)
    formatted_balance = None
    if len(payment_accounts) > 0:
        # Show zero balance only for active subscriptions, but always show non-zero amounts
        if payment_accounts[0].balance != 0 or subscription is not None:
            formatted_balance = payment_accounts[0].formatted_balance()

    if formatted_balance is not None:
        response_dict['account_balance'] = formatted_balance

    return render(request, 'group_subscription.html', response_dict)


@login_required
@check_tos
@check_email
def customer_subscription_invoices(request, customer_id):
    '''Managers and admins can see invoices for this group's subscription'''
    response_dict = {}
    invoices_raw = []
    invoices = []

    # Strip the leading zero when we can
    if platform.system() == 'Windows':
        date_format = '%b. %d, %Y'
    else:
        date_format = '%b. %-d, %Y'

    customer = get_customer_if_authorized(request, customer_id, require_manager=True)
    if not customer:
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    # Don't let groups see subscriptions if they're overridden for that group
    if customer.subscription_overridden():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    try:
        payment_accounts = customer.get_all_payment_accounts()
        for payment_account in payment_accounts:
            recurly_account = None
            try:
                recurly_account = recurly.Account.get(payment_account.account_code)
            except NotFoundError:
                '''Account no longer present, apparently'''
                pass
            if recurly_account is not None:
                for invoice in recurly_account.invoices():
                    invoices_raw.append(invoice)

        for raw_invoice in invoices_raw:
            invoice = {'number': raw_invoice.invoice_number,
                       'date': raw_invoice.created_at.strftime(date_format),
                       'amount': '$%0.2f' % (int(raw_invoice.total_in_cents) / 100),
                       'state': raw_invoice.state,
                       }
            invoices.append(invoice)
    except Exception as e:
        print('Exception getting invoice list: %s' % repr(e))
        messages.error(request, 'Sorry, there was a server error', 'danger')
        return redirect('customer_list')

    response_dict['invoices'] = invoices
    response_dict['customer'] = customer

    return render(request, 'group_subscription_invoices.html', response_dict)


# No email check
@login_required
@check_tos
def invoice_pdf(request, invoice_id):
    '''Proxy a request for the invoice download'''

    # Access control
    try:
        recurly_invoice = recurly.Invoice.get(invoice_id)
    except:
        recurly_invoice = None

    if not recurly_invoice:
        messages.error(request, 'Invoice unavailable', 'danger')
        return redirect('customer_list')

    try:
        local_accounts = PaymentAccount.objects.filter(account_code=recurly_invoice.account().account_code).all()
        subscriptions = Subscription.objects.filter(account__in=local_accounts).all()
        customer = Customer.objects.filter(subscription__in=subscriptions).distinct().first()
    except Exception as e:
        print('Exception getting invoice: %s' % repr(e))
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    # Check for the requesting user being authorized to fetch this invoice
    if not get_customer_if_authorized(request, customer.id, require_manager=True):
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    invoice_content = recurly.Invoice.pdf(invoice_id)

    response = HttpResponse(invoice_content, content_type='application/pdf')
    response['Content-Disposition'] = 'inline; filename="graco-harrier-invoice-%s.pdf"' % invoice_id
    return response


@login_required
@check_tos
@check_email
def customer_subscription_config(request, customer_id):
    '''Managers and admins can configure this group's subscription'''
    response_dict = {}
    subscription = None

    customer = get_customer_if_authorized(request, customer_id, require_manager=True)
    if not customer:
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    # Don't let groups see subscriptions if they're overridden for that group
    if customer.subscription_overridden():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    if customer.manager != request.user and not request.user.userprofile.is_admin():
        messages.error(request, 'Only the group manager can configure subscriptions and payments for the group.')
        return redirect('customer_subscription', customer_id)

    if customer.active_pump_count() < 1:
        messages.error(request, 'You need at least one active pump to be able to set up billing')
        return redirect('customer_subscription', customer_id)

    if customer.has_valid_subscription():
        subscription = customer.get_current_subscription()

    response_dict['customer'] = customer
    response_dict['use_existing_card_magic'] = PaymentAccount.USE_EXISTING_CARD

    if subscription:
        response_dict['valid_subscription'] = True
        response_dict['purchase_button_text'] = 'Update subscription'
        response_dict['subscription'] = subscription
        response_dict['plan_text'] = subscription.plan.name if subscription.plan else '????'
        response_dict['current_plan_code'] = subscription.plan.code

    else:
        response_dict['current_plan_code'] = None
        response_dict['valid_subscription'] = False
        response_dict['purchase_button_text'] = 'Subscribe'

    response_dict['recurly_api_public_key'] = settings.RECURLY_KEY

    # ##
    # Generate list of plans
    # ##

    plans_processed = []
    plans = Plan.objects.filter(is_available=True)

    # Non-admins can select from only a subset of plans
    if plans and not request.user.userprofile.is_admin():
        plans = plans.filter(user_selectable=True)

    if plans:
        plans = plans.order_by('price_cents').all()

    for plan in plans:
        plan_amount = (plan.price_cents / 100)
        interval_unit = ''
        if plan.plan_interval == 1 and plan.plan_units == 'months':
            interval_unit = 'month'
        elif plan.plan_interval == 12 and plan.plan_units == 'months':
            interval_unit = 'year'
        plan_name = '%s: US$%0.2f/%s/pump' % (plan.name, plan_amount, interval_unit)
        plans_processed.append((plan.code, plan_name))

        if plan.code == response_dict['current_plan_code']:
            response_dict['current_plan_name'] = plan_name

    response_dict['plan_list'] = plans_processed
    response_dict['month_list'] = [x for x in range(1, 13)]
    response_dict['country_list'] = [('US', 'United States'),
                                     ('CA', 'Canada')]

    # ##
    # Pass the current payment info if it exists
    # ##
    
    response_dict['payment_cards'] = []
    
    try:
        if request.user.userprofile.is_admin():
            account_code = PaymentAccount.generate_account_code(customer.manager, customer)
        else:
            account_code = PaymentAccount.generate_account_code(request.user, customer)
        recurly_account = recurly.Account.get(account_code)
    except NotFoundError:
        recurly_account = None
    except Exception as e:
        print('Exception getting payment info (does the group have a manager?): %s' % repr(e))
        recurly_account = None
    
    if recurly_account:
        try:
            # TODO: If the user previously canceled their subscription, the old billing info will no longer be available. Need to handle this better.
            billing = recurly_account.billing_info
            existing_card = 'Card on file (%s **-%s exp. %u/%u)' % (billing.card_type, billing.last_four, billing.month, billing.year)
            response_dict['payment_cards'].append((PaymentAccount.USE_EXISTING_CARD, existing_card))
        except:
            pass

    response_dict['payment_cards'].append((0, 'New payment card'))

    # ##
    # Render
    # ##

    return render(request, 'group_subscription_config.html', response_dict)


@login_required
@check_tos
@check_email
def customer_pump_add(request, customer_id):
    '''
    Interface for adding a pump to one of the current user's groups.

    Not applicable for admins.
    '''
    response_dict = {}
    customer_list = []

    '''
    Admins can add a pump to any group, but normal users can add a pump only
    to groups they manage
    '''
    customer = get_customer_if_authorized(request, customer_id, require_manager=True)
    if not customer:
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    response_dict['customer'] = customer

    return render(request, 'pump_add.html', response_dict)


@login_required
@check_tos
@check_email
def customer_user_add(request, customer_id):
    '''
    Interface for an admin to add a user to a group.

    Not applicable for normal users or managers; they must invite users to groups, not add them by fiat.
    '''
    response_dict = {}

    if not request.user.userprofile.is_admin():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('customer_list')

    customer = get_customer_if_authorized(request, customer_id, require_manager=True)
    if not customer:
        messages.error(request, 'Group not found', 'danger')
        return redirect('customer_list')

    response_dict['customer'] = customer

    return render(request, 'group_user_add.html', response_dict)


@login_required
@check_tos
@check_email
def user_list(request):
    '''User administration page'''
    if not request.user.userprofile.is_admin():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('index')

    return render(request, 'user_list.html', {})


@login_required
@check_tos
def user_settings(request, user_id=None):
    '''User settings'''
    if not user_id:
        user_id = request.user.pk

    user = User.objects.filter(pk=user_id).first()

    # Only admins can access user pages other than their own
    if not user or (user != request.user and not request.user.userprofile.is_admin()):
        messages.error(request, 'User not found', 'danger')
        return redirect('index')

    customer_list = []
    if request.user.userprofile.is_admin():
        customers = Customer.objects.all()
        for customer in customers:
            customer_values = (customer.pk,
                               customer.organization_name)
            customer_list.append(customer_values)

    # Admins can't make themselves non-admins
    is_self = (user == request.user)

    # Remind the user about their email address needing verification
    if is_self:
        if not request.user.userprofile.email_confirmed and len(request.user.email) > 0:
            resend_url = reverse('user_verification_resend', args=[])
            messages.warning(request, 'Your email address still needs to be verified. Check your email inbox for the verification message and follow the instructions inside of it. If needed, you can <a href="%s">resend</a> the verification email.' % (resend_url,), extra_tags='safe')

    units_list = [(index, item) for index, item in enumerate(UnitsOfMeasure.TEXT_SYSTEM)]

    resp_dict = {'user_id': user_id,
                 'timezones': pytz.common_timezones,
                 'customer_list': customer_list,
                 'unit_options': units_list,
                 'is_self': is_self}

    return render(request, 'user_settings.html', resp_dict)


@login_required
@check_tos
@check_email
def user_alarm_prefs(request, user_id=None):
    '''User-level alarm preferences'''
    if not user_id:
        user_id = request.user.pk

    user = User.objects.filter(pk=user_id).first()

    # Only admins can access user pages other than their own
    if not user or (user != request.user and not request.user.userprofile.is_admin()):
        messages.error(request, 'User not found', 'danger')
        return redirect('index')

    configurable_alarms = []
    for alarm_id in AlarmPreference.ALARMS_CONFIGURABLE:
        element_name = '%s%d' % (AlarmPreference.QUALIFIER, alarm_id)
        element = (element_name, AlarmPreference.get_alarm_name(alarm_id))
        configurable_alarms.append(element)

    resp_dict = {'user_id': user_id,
                 'configurable_alarms': configurable_alarms
                 }

    return render(request, 'user_alarm_prefs.html', resp_dict)


@login_required
@check_tos
@check_email
def user_create(request):
    if not request.user.userprofile.is_admin():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('index')

    customer_list = []
    customers = Customer.objects.order_by('id')
    for customer in customers:
        customer_values = (customer.pk,
                           customer.organization_name)
        customer_list.append(customer_values)

    units_list = [(index, item) for index, item in enumerate(UnitsOfMeasure.TEXT_SYSTEM)]

    resp_dict = {'timezones': pytz.common_timezones,
                 'unit_options': units_list,
                 'unit_default': UnitsOfMeasure.IMPERIAL,
                 'customer_list': customer_list,
                 'customer_default': get_none_customer(), }

    return render(request, 'user_create.html', resp_dict)


# No login needed
@ensure_csrf_cookie
def user_register(request):
    """For allowing a user to register a new account for themselves"""
    # For simplicity, don't allow logged-in users to create new accounts
    if request.user and request.user.is_authenticated():
        return redirect('/')

    units_list = [(index, item) for index, item in enumerate(UnitsOfMeasure.TEXT_SYSTEM)]

    response_dict = {'timezones': pytz.common_timezones,
                     'unit_options': units_list,
                     'unit_default': UnitsOfMeasure.IMPERIAL,
                     }

    return render(request, 'user_register.html', response_dict)


# No login needed
@ensure_csrf_cookie
def user_register_done(request):
    """For allowing a user to register a new account for themselves"""
    response_dict = {}

    # For simplicity, don't allow logged-in users to create new accounts
    if request.user and request.user.is_authenticated():
        return redirect('/')

    return render(request, 'user_register_done.html', response_dict)


# No login needed
@ensure_csrf_cookie
def user_register_verify(request, verification_key):
    '''This is where clicking the link in the email verification message from the account activation email will take the user'''
    response_dict = {}

    user_profile = UserProfile.register(verification_key)
    if user_profile is not None:
        response_dict['activation_success'] = True
    else:
        response_dict['activation_success'] = False

    log = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND, message='Activate account')
    log.target_type = Log.TARGET_USER
    log.origin_ip = request.META['REMOTE_ADDR']
    log.action = Log.ACTION_UPDATE
    log.attribute = 'is_active'
    log.new_value = 1
    if user_profile is not None:
        log.target_id = user_profile.user.pk
        log.user_actor = user_profile.user
        log.success = 1
    else:
        log.success = 0
    log.save()

    return render(request, 'user_register_verify.html', response_dict)


@login_required
def user_email_verify(request, verification_key):
    '''This is where clicking the link in the email verification message will take the user'''
    response_dict = {}

    success = request.user.userprofile.verify(verification_key)
    response_dict['verification_successs'] = success

    log = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND, message='')
    log.target_type = Log.TARGET_USER
    log.origin_ip = request.META['REMOTE_ADDR']
    log.action = Log.ACTION_UPDATE
    log.attribute = 'email_confirmed'
    log.new_value = 1
    log.target_id = request.user.pk
    log.user_actor = request.user
    log.success = 1 if success else 0
    log.save()

    return render(request, 'user_email_verify.html', response_dict)


@login_required
def user_verification_resend(request):
    '''Resend the email verification message to the calling user'''
    response_dict = {}

    response_dict['email_confirmed'] = request.user.userprofile.email_confirmed
    response_dict['username'] = request.user.userprofile.get_username()

    if not response_dict['email_confirmed']:
        if re.match(UserProfile.EMAIL_REGEX, request.user.email):

            # Re-send verification email
            request.user.userprofile.send_email_verification(UserProfile.EMAIL_CHANGE_VERIFICATION)

            log = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_EMAIL)
            log.target_type = Log.TARGET_USER
            log.origin_ip = request.META['REMOTE_ADDR']
            log.action = Log.ACTION_SEND
            log.target_id = request.user.pk
            log.user_actor = request.user
            log.message = 'Verification resend'
            log.success = 1
            log.save()

        else:
            response_dict['invalid_email'] = True

    return render(request, 'user_verification_resend.html', response_dict)


@login_required
def user_tos(request):
    '''Show the current Terms of Service to the user and ask them to accept. Don't allow them to proceed until they have accepted the TOS'''
    response_dict = {}

    most_recent_tos = TermsOfService.objects.last()

    if request.user.userprofile.tos_agreed == most_recent_tos:
        response_dict['already_agreed'] = True

    response_dict['tos'] = most_recent_tos
    response_dict['user_id'] = request.user.id

    return render(request, 'user_tos.html', response_dict)


# No login required
@ensure_csrf_cookie
def user_invitation(request, invitation_code):
    '''Landing page for users accepting an invitation request'''
    response_dict = {}

    invitation_code_clean = re.sub(Invitation.INVITATION_REGEX, '', invitation_code)

    invitation = Invitation.is_valid(invitation_code_clean)

    username = None
    user_id = None
    if request.user.is_authenticated():
        username = request.user.userprofile.get_username()
        user_id = request.user.id

    response_dict['logged_in'] = request.user.is_authenticated()
    response_dict['username'] = username
    response_dict['user_id'] = user_id
    response_dict['invitation'] = invitation

    return render(request, 'user_invitation_landing.html', response_dict)


@ensure_csrf_cookie
def user_invitation_register(request, invitation_code):
    '''Registration page for users who are accepting invitations'''

    units_list = [(index, item) for index, item in enumerate(UnitsOfMeasure.TEXT_SYSTEM)]

    response_dict = {'timezones': pytz.common_timezones,
                     'unit_options': units_list,
                     'unit_default': UnitsOfMeasure.IMPERIAL,
                     }

    invitation = Invitation.is_valid(invitation_code)

    response_dict['invite_mode'] = True
    response_dict['invitation'] = invitation

    return render(request, 'user_register.html', response_dict)


@login_required
@check_tos
@check_email
def pump_list(request):
    '''Display pump list'''
    response_dict = {}

    response_dict['user_id'] = request.user.pk
    response_dict['is_manager'] = Customer.objects.filter(manager=request.user).exists()

    return render(request, 'pump_list.html', response_dict)


@login_required
@check_tos
@check_email
def pump_details(request, pump_id):
    '''Display the details about a given pump and handle commands.'''
    pump_id_filtered = re.sub(Pump.regex_for_clean, '', pump_id)
    pump = get_pump_if_authorized(request, pump_id_filtered)

    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')

    units_key = {'volume': UnitsOfMeasure.TEXT_VOLUME,
                 'pressure': UnitsOfMeasure.TEXT_PRESSURE,
                 'tank': UnitsOfMeasure.TEXT_TANK,
                 'temperature': UnitsOfMeasure.TEXT_TEMP}
    units_decode = json.dumps(units_key)

    customers = None
    customer_list = []
    if request.user.userprofile.is_admin():
        customers = Customer.objects.all()
    else:
        customers = Customer.objects.filter(manager=request.user)

    if customers is not None:
        for customer in customers:
            customer_values = (customer.pk,
                               customer.organization_name)
            customer_list.append(customer_values)

    status_names = pump.get_status_names()
    status_true_ids = pump.get_status_true_ids()

    payment_switchover_enable = (SitePreference.get_pref(SitePreference.PREF_NOTIFY_UPCOMING_PAYMENT_SWITCHOVER, default="0") == "1")
    if payment_switchover_enable and not request.user.userprofile.is_admin():
        payment_notice_text_one = 'Starting January 1, 2018, you will need to have a paid subscription to continue monitoring and managing this pump. \
            To avoid service interruptions, please ensure that your payment information for this group is up to date before that time.'
        payment_notice_text_two = 'Starting January 1, 2018, you will need to have a paid subscription to continue monitoring and managing this pump. \
            More information will be available soon.'

        # Using the less-definite version for now
        payment_notice_text = payment_notice_text_two

#         if request.user == pump.customer.manager:
#             payment_url = reverse('customer_subscription', args=[pump.customer.id])
#             payment_notice_text = payment_notice_text_one + ' You can update your payment information on this group\'s <a href="%s">subscription page</a>' % (payment_url,)
        messages.error(request, payment_notice_text, 'danger safe')

    pump_event_log_url = ''
    if request.user.userprofile.is_admin():
        rows_per_page = 200
        pump_event_log_url = reverse('admin_event_log')
        pump_event_log_url += '?target_type=1&id_filter=%s&page_size=%u' % (pump_id, rows_per_page)

    temperature_list = [(0, 'Disabled'), (1, 'Display'), (2, 'On Below'), (3, 'On Above')]

    response_dict = {'customer_list': customer_list,
                     'pump_event_log_url': pump_event_log_url,
                     'pump_id': pump_id_filtered,
                     'units_decode': units_decode,
                     'power_save_mode_off': Pump.POWER_SAVE_OFF,
                     'power_status_alarm_id': Pump.POWER_STATUS_DISABLED_BY_REMOTE_ALARM_ID,
                     'power_status_id': Pump.POWER_STATUS_DISABLED_BY_REMOTE,
                     'status_names': status_names,
                     'is_manager': request.user == pump.customer.manager,
                     'status_true_ids': status_true_ids,
                     'temperature_list': temperature_list, }
    return render(request, 'pump_details.html', response_dict)


@login_required
@check_tos
@check_email
def pump_alarm_custom(request, pump_id):
    '''Customization of alarms for a pump'''
    response_dict = {}

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')

    alarms_for_custom = [(7, 'Input 1'),
                         (8, 'Input 2'),
                         (11, 'Remote')]

    response_dict['alarms_for_custom'] = [('alarm_%u_custom' % x[0], x[1]) for x in alarms_for_custom]
    response_dict['pump_obj'] = pump

    return render(request, 'pump_alarm_custom.html', response_dict)

@login_required
@check_tos
@check_email
def pump_injection_point_custom(request, pump_id):
    '''Customization of injection points for a pump'''
    response_dict = {}

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')

    injection_points_for_custom = [(0, '1. '),
                                   (1, '2. '),
                                   (2, '3. '),
                                   (3, '4. '),
                                   (4, '5. '),
                                   (5, '6. '),
                                   (6, '7. '),
                                   (7, '8. ')]

    response_dict['injection_points_for_custom'] = [('point_%u_custom' % x[0], x[1]) for x in injection_points_for_custom]
    response_dict['pump_obj'] = pump

    return render(request, 'pump_injection_point_custom.html', response_dict)

@login_required
@check_tos
@check_email
def pump_multiwell_flow_rates(request, pump_id):
    '''List of injection point flow rates'''
    response_dict = {}

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')
    
    well_status_names = pump.get_well_status_names()
    well_status_true_ids = pump.get_well_status_true_ids()
    
    units_key = {'volume': UnitsOfMeasure.TEXT_VOLUME,
                 'pressure': UnitsOfMeasure.TEXT_PRESSURE,
                 'tank': UnitsOfMeasure.TEXT_TANK,
                 'temperature': UnitsOfMeasure.TEXT_TEMP}
    units_decode = json.dumps(units_key)    

    flow_rates = [(1, 'Flow Rate 1'),
                  (2, 'Flow Rate 2'),
                  (3, 'Flow Rate 3'),
                  (4, 'Flow Rate 4'),
                  (5, 'Flow Rate 5'),
                  (6, 'Flow Rate 6'),
                  (7, 'Flow Rate 7'),
                  (8, 'Flow Rate 8')]
    
    custom_inj_point_names = {}
    
    '''Pump injection point names, not just the active ones'''
    custom_point_names = pump.pointcustomization_set.all()
    if len(custom_point_names) > 0:
        custom_inj_point_names['custom_injection_point_names'] = {}
        for name_obj in custom_point_names:
            custom_inj_point_names['custom_injection_point_names']['point_%u_custom' % name_obj.point_id] = str(name_obj.point_id+1) + '. ' + name_obj.custom_point_name

    response_dict['flow_rates'] = [('well_flow_rate_%u' % x[0], x[1]) for x in flow_rates]
    response_dict['pump_obj'] = pump
    response_dict['point_names'] = custom_inj_point_names
    response_dict['units_decode'] = units_decode
    response_dict['well_status_names'] = well_status_names
    response_dict['well_status_true_ids'] = well_status_true_ids

    return render(request, 'pump_flow_rates.html', response_dict)

@login_required
@check_tos
@check_email
def pump_multiwell_totalizers(request, pump_id):
    '''List of injection point totalizers'''
    response_dict = {}

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')
    
    units_key = {'volume': UnitsOfMeasure.TEXT_VOLUME,
                 'pressure': UnitsOfMeasure.TEXT_PRESSURE,
                 'tank': UnitsOfMeasure.TEXT_TANK,
                 'temperature': UnitsOfMeasure.TEXT_TEMP}
    units_decode = json.dumps(units_key)    

    totalizers = [(1, 'Totalizer 1', 'GTotalizer 1'),
                  (2, 'Totalizer 2', 'GTotalizer 2'),
                  (3, 'Totalizer 3', 'GTotalizer 3'),
                  (4, 'Totalizer 4', 'GTotalizer 4'),
                  (5, 'Totalizer 5', 'GTotalizer 5'),
                  (6, 'Totalizer 6', 'GTotalizer 6'),
                  (7, 'Totalizer 7', 'GTotalizer 7'),
                  (8, 'Totalizer 8', 'GTotalizer 8')]
    
    custom_inj_point_names = {}
    
    '''Pump injection point names, not just the active ones'''
    custom_point_names = pump.pointcustomization_set.all()
    if len(custom_point_names) > 0:
        custom_inj_point_names['custom_injection_point_names'] = {}
        for name_obj in custom_point_names:
            custom_inj_point_names['custom_injection_point_names']['point_%u_custom' % name_obj.point_id] = str(name_obj.point_id+1) + '. ' + name_obj.custom_point_name

    response_dict['GTotalizers'] = [('totalizer_grand_well_%u' % x[0], x[1]) for x in totalizers]
    response_dict['totalizers'] = [('totalizer_well_%u' % x[0], x[1]) for x in totalizers]
    response_dict['pump_obj'] = pump
    response_dict['point_names'] = custom_inj_point_names
    response_dict['units_decode'] = units_decode

    return render(request, 'pump_totalizers.html', response_dict)

@login_required
@check_tos
@check_email
def analog_in_settings(request, pump_id):
    '''Settings related to the analog input'''
    response_dict = {}

    pump = get_pump_if_authorized(request, pump_id)
    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')
    
    units_key = {'volume': UnitsOfMeasure.TEXT_VOLUME,
                 'pressure': UnitsOfMeasure.TEXT_PRESSURE,
                 'tank': UnitsOfMeasure.TEXT_TANK,
                 'temperature': UnitsOfMeasure.TEXT_TEMP}
    units_decode = json.dumps(units_key)
    
    ain_modes_list = [(0, 'Off'), (1, 'On')]
    
    response_dict['pump_obj'] = pump
    response_dict['units_decode'] = units_decode
    response_dict['ain_modes_list'] = ain_modes_list

    return render(request, 'analog_in_settings.html', response_dict)

@login_required
@check_tos
@check_email
def pump_history(request, pump_id):
    '''Charting of historical data'''
    pump_id_filtered = re.sub(Pump.regex_for_clean, '', pump_id)
    pump = get_pump_if_authorized(request, pump_id_filtered)

    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')

    pump_dict = model_to_dict(pump)
    pump_dict['timestamp'] = int(pump.timestamp.timestamp())

    units_key = {'volume': UnitsOfMeasure.TEXT_VOLUME,
                 'pressure': UnitsOfMeasure.TEXT_PRESSURE,
                 'tank': UnitsOfMeasure.TEXT_TANK,
                 'temperature': UnitsOfMeasure.TEXT_TEMP}
    units_decode = json.dumps(units_key)

    custom_inj_point_names = {}
    
    '''Pump injection point names, not just the active ones'''
    custom_point_names = pump.pointcustomization_set.all()
    if len(custom_point_names) > 0:
        custom_inj_point_names['custom_injection_point_names'] = {}
        for name_obj in custom_point_names:
            custom_inj_point_names['custom_injection_point_names']['point_%u_custom' % name_obj.point_id] = name_obj.custom_point_name

    response_dict = {'pump_id': pump_id_filtered, 'pump_obj': pump_dict, 'units_decode': units_decode, 'point_names': custom_inj_point_names}

    return render(request, 'pump_history.html', response_dict)


@login_required
def pump_history_download(request, pump_id):
    '''Download the history only for the given pump'''
    pump = get_pump_if_authorized(request, pump_id)

    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')

    if not request.user.userprofile.is_admin() and not pump.has_valid_subscription():
        messages.error(request, 'Feature unavailable due to expired subscription', 'danger')
        return redirect('pump_details', pump_id)

    log_name = 'graco-pump-%s-history-%s' % (pump.unique_id, datetime.datetime.now())
    log_name = re.sub(r'\s', '_', log_name)

    log_objects = History.objects.filter(pump_id=pump.unique_id).order_by('pk')

    response = log_download_helper(log_objects, log_name)
    return response


@login_required
@check_tos
@check_email
def pump_aeris(request, pump_id):
    '''Aeris API interface for this pump'''
    if not request.user.userprofile.is_admin():
        messages.error(request, 'You cannot do that', 'danger')
        return redirect('index')

    pump_id_filtered = re.sub(Pump.regex_for_clean, '', pump_id)
    pump = get_pump_if_authorized(request, pump_id_filtered)

    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('pump_list')

    pump_dict = model_to_dict(pump)
    pump_dict['timestamp'] = int(pump.timestamp.timestamp())

    response_dict = {'pump_id': pump_id_filtered, 'pump_obj': pump_dict}

    return render(request, 'pump_aeris.html', response_dict)


@login_required
@check_tos
@check_email
def notification_list(request, pump_id=None, user_id=None):
    '''Notifications for the given pump'''
    pump_id_filtered = None

    page_type = None
    page_subject = None

    '''
    Pumps
    '''

    if pump_id:
        pump_id_filtered = re.sub(Pump.regex_for_clean, '', pump_id)
        pump = get_pump_if_authorized(request, pump_id_filtered)

        if not pump:
            messages.error(request, 'Pump not found', 'danger')
            return redirect('index')

        page_type = 'Pump'
        if len(pump.pretty_name) > 0:
            page_subject = pump.pretty_name
        else:
            page_subject = pump.unique_id

    '''
    Users
    '''

    if user_id:
        user = User.objects.get(pk=user_id)
        # Only admins can access user pages other than their own
        if not user or (user != request.user and not request.user.userprofile.is_admin()):
            messages.error(request, 'User not found', 'danger')
            return redirect('index')
        if not pump_id:
            page_type = 'User'
            page_subject = user.userprofile.get_username()

    elif not request.user.userprofile.is_admin():
        user = request.user
        user_id = user.id

    criteria_decode = json.dumps(Notification.CRITERIA_TYPES)

    response_dict = {'pump_id': pump_id_filtered,
                     'user_id': user_id,
                     'page_type': page_type,
                     'page_subject': page_subject,
                     'criteria_decode': criteria_decode}

    return render(request, 'notification_list.html', response_dict)


@login_required
@check_tos
@check_email
def notification_create(request, pump_id):
    '''Create notifications'''
    pump_id_filtered = re.sub(Pump.regex_for_clean, '', pump_id)
    pump = get_pump_if_authorized(request, pump_id_filtered)

    if not pump:
        messages.error(request, 'Pump not found', 'danger')
        return redirect('index')

    response_dict = {}

    if len(pump.pretty_name) > 0:
        pump_name = pump.pretty_name
    else:
        pump_name = pump.unique_id
    response_dict['pump_name'] = pump_name

    # Skip the "unknown" option
    subject_options = Notification.SUBJECT_TYPES[1:]
    response_dict['subject_options'] = subject_options

    period_options = ['1', '3', '6', '12', '24', Notification.PERIOD_OTHER_VALUE]

    if True or request.user.userprofile.is_admin():
        period_options.append(Notification.IMMEDIATE)

    response_dict['period_options'] = period_options

    response_dict['period_default'] = '6'

    response_dict['subject_other_value'] = Notification.SUBJECT_OTHER_VALUE
    response_dict['period_other_value'] = Notification.PERIOD_OTHER_VALUE

    response_dict['pump_id'] = pump_id_filtered
    response_dict['user_id'] = request.user.pk

    if not request.user.userprofile.email_confirmed and len(request.user.email) > 0:
        resend_url = reverse('user_verification_resend', args=[])
        messages.warning(request, 'Your email address needs to be verified before you can create reminders. Check your email inbox for the verification message and follow the instructions inside of it. If needed, you can <a href="%s">resend</a> the verification email.' % (resend_url,), extra_tags='safe')

    return render(request, 'notification_create.html', response_dict)


def log_download_helper(log_objects, log_name):
    '''Send the history log as a downloadable file'''
    first = True
    log_lines = []
    log_content = ''
    line_count = 0
    line_threshold = 100000

    if len(log_objects) > 0:
        for log_entry in log_objects:
            if first:
                log_lines.append(log_entry.get_csv_header())
                first = False
            log_lines.append(log_entry.get_csv_line())
            line_count += 1

            # Flush the buffer periodically
            if line_count > line_threshold:
                line_count = 0
                log_content += '\n'.join(log_lines) + '\n'
                log_lines.clear()

    else:
        log_lines = ['No records found']

    log_content += '\n'.join(log_lines)
    response = HttpResponse(log_content, content_type='text/plain')
    response['Content-Disposition'] = 'attachment; filename="%s.csv"' % log_name
    return response



