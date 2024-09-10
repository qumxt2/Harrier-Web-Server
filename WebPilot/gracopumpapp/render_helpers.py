'''
Helpers
'''

import hashlib
import ipaddress
import re

from django.http.response import JsonResponse, HttpResponseNotModified
from django.shortcuts import render
from gracopumpapp.models import Pump, Customer, Log, get_none_customer, sanitize
from version import GetVersion


def json_requested(request):
    if ('format' in request.GET and request.GET['format'] == 'json') or \
            ('format' in request.POST and request.POST['format'] == 'json'):
        return True
    return False


def respond_not_logged_in(request):
    return respond(request, status=401, message='Not logged in')


def respond_not_authorized(request, json=True):
    return respond(request, status=401, message='Not authorized', json=json)


def respond_error(request, status=500, message='Error', json=True, flash_message=None):
    '''Standard error reponse'''
    return respond(request, status, message, json=json, flash_message=flash_message)


def generate_etag(obj):
    '''
    Generate a unique representation of the data. Not absolutely guaranteed to be the same from run to run with the same data,
    but guaranteed to be different if the input is different.
    '''
    try:
        string_rep = repr(obj)
        etag = hashlib.md5(string_rep.encode('utf-8')).hexdigest()
    except Exception:
        etag = None

    return etag


def respond(request, status=200, message='OK', response_dict={}, json=True, flash_message=None, flash_safe=False):
    '''Standardized response'''
    net_response_dict = {'status': status,
                         'message': message,
                         'version': GetVersion(),
                         }

    if flash_message:
        net_response_dict['flash'] = flash_message

        if flash_safe:
            net_response_dict['safeFlash'] = True

    net_response_dict['data'] = response_dict

    if json or json_requested(request):
        etag = generate_etag(net_response_dict)
        modified = True
        if etag is not None and 'HTTP_IF_NONE_MATCH' in request.META:
            modified = (etag != request.META['HTTP_IF_NONE_MATCH'])

        if modified:
            response = JsonResponse(net_response_dict, status=status)
            if etag:
                response['ETag'] = etag
            return response
        else:
            return HttpResponseNotModified()
    else:
        resp_dict = {'error_number': status,
                     'error_explanation': message}
        response = render(request, 'error.html', resp_dict)
        response.status_code = status
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


def log_command(request, target_type=Log.TARGET_UNKNOWN, target_id='', attribute='', new_value='', action=Log.ACTION_UPDATE, status=0, message=''):
    '''Make a log entry of the command and who did it'''

    # Don't log old or new passwords
    if attribute == 'password':
        new_value = ''

    success = 1 if status >= 200 and status < 300 else 0

    log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB, event_type=Log.EVENT_COMMAND)
    log_entry.origin_ip = request.META['REMOTE_ADDR']
    log_entry.user_actor = request.user
    log_entry.target_type = target_type
    log_entry.action = action
    log_entry.success = success
    log_entry.target_id = sanitize(str(target_id))
    log_entry.new_value = sanitize(str(new_value))
    log_entry.attribute = sanitize(str(attribute))
    log_entry.message = sanitize(message)
    log_entry.save()


def get_pump_if_authorized(request, pump_id, access=None):
    '''Get a pump object if user is authorized and the pump exists'''
    pump = Pump.objects.filter(unique_id=pump_id, is_active=True).first()

    # Non-admins can see only their own pumps
    if pump and not request.user.userprofile.is_admin():
        user_customers = request.user.userprofile.customers.all()

        # ...but not pumps without a group
        none_customer = Customer.objects.get(pk=get_none_customer())

        if pump.customer not in user_customers or pump.customer == none_customer:
            pump = None

    # Only allow admins and group managers to delete pumps
    if access == 'delete':
        if not request.user.userprofile.is_admin() and not pump.customer.manager == request.user:
            pump = None

    return pump


def get_customer_if_authorized(request, customer_id, require_manager=False):
    '''Get a customer if the user is authorized and the customer exists'''
    ret_val = None

    try:
        customer = Customer.objects.get(pk=customer_id)

        # Only admins and a customer's members can get a customer's data
        if customer:
            if request.user.userprofile.is_admin():
                ret_val = customer
            elif customer in request.user.userprofile.customers.all():
                if require_manager:
                    if customer.manager == request.user:
                        ret_val = customer
                else:
                    ret_val = customer
    except:
        # Ignore, return None
        pass

    return ret_val
