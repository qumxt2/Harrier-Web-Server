'''
Support routines for accessing the Aeris API
'''
from datetime import datetime

from gracopumpapp.models import Pump
import pytz
from suds.client import Client


AERIS_API_URL = 'https://aeradminapi.aeris.com/AerAdmin_WS_5_0/ws?wsdl'
AERIS_API_KEY = '739f3c0d-1c0f-11e4-ad58-9523ebaf2059'
AERIS_ACCOUNT_ID = '11165'
AERIS_EMAIL = 'unnecessary@example.com'
AERIS_DEFAULT_PARAMS = {'accountID': AERIS_ACCOUNT_ID, 'email': AERIS_EMAIL}
AERIS_SUCCESS_CODE = '0'


class AerisStatus(object):
    '''
    Data-storage object class for results from the Aeris API query
    '''

    def __init__(self):
        # Indicate whether a data update was successful
        self.data_retrieved = False

        # Response fields
        self.technology = None
        self.activation_date = None
        self.data_start_date = None
        self.data_stop_date = None
        self.data_inactive_date = None
        self.registration_date = None
        self.is_registered = False
        self.network_status = None


def aeris_client():
    '''
    Get an Aeris API client instance
    '''
    url = AERIS_API_URL
    cl = Client(url)
    cl.set_options(headers={'apiKey': AERIS_API_KEY})

    return cl


def aeris_wsdl_dump():
    '''
    Get description of the AerAdmin API functions and types and dump to console
    '''
    client = aeris_client()
    print(client)


def aeris_heartbeat():
    '''
    Do a heartbeat check of the Aeris API

    This can also serve to keep the VPN connection open
    '''
    success = False

    try:
        client = aeris_client()
        result = client.service.heartBeat(**AERIS_DEFAULT_PARAMS)
        if result.resultCode == AERIS_SUCCESS_CODE:
            success = True
    except:
        success = False

    return success


def aeris_datetime(datetime_str):
    '''
    Convert an Aeris-foramtted datetime to a Python datetiem
    '''
    result = None
# 2015-07-13T20:45:23.000Z

    try:
        fmt = '%Y-%m-%dT%H:%M:%S.000Z'
        result = datetime.strptime(datetime_str, fmt)
        result = result.replace(tzinfo=pytz.utc)
    except:
        print('Failure converting Aeris datetime: %s' % (datetime_str,))

    return result


def aeris_deviceID(client, pump):
    '''Create an Aeris API deviceID object for this pump'''
    # The DeviceID is an object type defined in the WSDL
    device_id = client.factory.create('ns1:DeviceID')
    device_id.imsi = pump.unique_id

    return device_id


def aeris_get_status(pump, tech_only=False):
    '''
    Get device details from Aeris for the given pump

    The QueryDeviceStatus endpoint allows us to find: the technology (CDMA, GSM, etc), the activation date, and whether it's active

    Returns an AerisStatus object and the client used
    '''
    status_obj = AerisStatus()
    client = None

    try:
        if type(pump) != Pump:
            raise TypeError('Pump object wrong type')

        client = aeris_client()

        # First stage
        result = client.service.queryDeviceDetail(IMSI=pump.unique_id, **AERIS_DEFAULT_PARAMS)
        if result.resultCode == AERIS_SUCCESS_CODE:
            r = result.deviceAttributes[0]
            status_obj.technology = r.technology
            status_obj.activation_date = aeris_datetime(getattr(r, 'activationDate', '?'))
        else:
            raise Exception('Aeris deviceDetail API fetch failed')

        if not tech_only:
            # Second stage
            result = client.service.queryDeviceNetworkStatus(IMSI=pump.unique_id, **AERIS_DEFAULT_PARAMS)

            try:
                r = result.networkResponse[0]
                status_obj.data_start_date = aeris_datetime(getattr(r.dataSession, 'lastStartTime', '?'))
                status_obj.data_stop_date = aeris_datetime(getattr(r.dataSession, 'lastStopTime', '?'))
                status_obj.data_inactive_date = aeris_datetime(getattr(r.registration, 'lastInactiveTime', '?'))
                status_obj.registration_date = aeris_datetime(getattr(r.registration, 'lastRegistrationTime', '?'))
                status_obj.is_registered = getattr(r.registration, 'isRegistered', False)
            except:
                raise Exception('Aeris queryDeviceNetworkStatus API fetch failed')

            # Third stage
            if status_obj.technology == 'LTE':
                # No network status is provided for LTE clients
                status_obj.network_status = '?'
            else:
                device_id = aeris_deviceID(client, pump)
                result = client.service.getDeviceNetworkStatus(deviceID=device_id, technology=status_obj.technology, **AERIS_DEFAULT_PARAMS)
                if result.resultCode == AERIS_SUCCESS_CODE:
                    status_obj.network_status = getattr(result, 'deviceNetworkStatus', '?')
                else:
                    raise Exception('Aeris getDeviceNetworkStatus API fetch failed')

        status_obj.data_retrieved = True

    except Exception as e:
        print('Aeris API status exception: %s' % (repr(e),))
        status_obj.data_retrieved = False

    return (status_obj, client)


def aeris_clear_registration(pump):
    '''
    Clear the network registration for the given pump

    This is useful if the pump appears to be having problems registering itself on the Aeris network

    First step is to find the device technology, which can be obtained from the QueryDeviceStatus endpoint

    Then, the ClearDeviceRegistration endpoint can be used.

    '''
    success = False

    try:
        status_obj, client = aeris_get_status(pump, tech_only=True)

        if not status_obj.data_retrieved:
            raise Exception('Problem retrieving preliminary data from Aeris')

        device_id = aeris_deviceID(client, pump)

        technology = status_obj.technology

        result = client.service.clearDeviceRegistration(deviceID=device_id, technology=technology, **AERIS_DEFAULT_PARAMS)

        if result.resultCode == AERIS_SUCCESS_CODE:
            success = True

    except Exception as e:
        print('Aeris API clear-registration exception: %s' % (repr(e),))
        success = False

    return success
