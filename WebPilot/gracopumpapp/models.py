import code
import copy
from datetime import datetime, time, timedelta
import random
import re
import string

from GracoPump.local_settings import SITE_DOMAIN, USE_HTTPS
from dateutil.relativedelta import relativedelta
from django.conf import settings
from django.contrib.auth.models import User, Group
from django.core.mail import send_mail
from django.urls import reverse
from django.db import models
from django.template import Context
from django.template.loader import get_template
from gracopumpapp.libs.mosquitto_auth_plug.hashing_passwords import make_hash
from gracopumpapp.pumps import PumpSet, AlarmList, AlarmsManuallyCleared
import pytz
import recurly


def get_none_customer():
    customer_obj = Customer.objects.get(organization_name=Customer.NONE_CUSTOMER_NAME)
    return customer_obj.pk


def sanitize(value):
    '''Do some basic input sanitization'''
    regex = r'[\'"<;:%&]'
    return re.sub(regex, '', value)


class History(models.Model):
    '''Model for logging all of the state changes that happen on the pumps.
    Basically a big immutable key-value store.'''
    id = models.AutoField(primary_key=True)
    ACCEPTABLE_HISTORY_CHARTS = ['totalizer_grand',
                                 'battery_voltage',
                                 'pressure_level',
                                 'monthly_report',
                                 'tank_level',
                                 'temperature',
                                 'totalizer_grand_well_1',
                                 'totalizer_grand_well_2',
                                 'totalizer_grand_well_3',
                                 'totalizer_grand_well_4',
                                 'totalizer_grand_well_5',
                                 'totalizer_grand_well_6',
                                 'totalizer_grand_well_7',
                                 'totalizer_grand_well_8', ]

    BY_DAY = 0
    BY_MONTH = 1
    BY_LAST_24_HOURS = 2

    pump_id = models.CharField(max_length=100, default='')
    timestamp = models.DateTimeField(auto_now_add=False)

    # Should be a column name from the Pump model
    attribute = models.CharField(max_length=200, default='')

    # Should be the raw value, coerced to a string, not the decoded value
    value = models.CharField(max_length=200, default='')

    def get_csv_line(self):
        '''Represent the current object as a CSV line'''
        values_to_encode = [str(self.pk),
                            '#' + self.pump_id,
                            str(self.timestamp),
                            self.attribute,
                            self.value,
                            ]

        output = ','.join([re.sub(r',', ';', v) for v in values_to_encode])

        return output

    def get_csv_header(self):
        '''Get the header'''
        column_names = ['Entry number',
                        'Pump ID',
                        'Timestamp',
                        'Attribute',
                        'Value',
                        ]
        return ','.join(column_names)

    @staticmethod
    def get_time_since_last_update():
        '''Calculate how much time has elapsed since the mqtt/db worker did something'''
        time_elapsed = 0
        most_recent_history = History.objects.order_by('id').last()
        if most_recent_history is not None:
            history_timestamp = most_recent_history.timestamp.timestamp()
            current_timestamp = datetime.utcnow().replace(tzinfo=pytz.utc).timestamp()
            time_elapsed = current_timestamp - history_timestamp
        return time_elapsed

    @staticmethod
    def get_history_totalizer(pump_id, attribute, history_period_type, timezone_text, num_periods=None, max_volume=None):
        '''
        Get the flow rate per period for the given pump by summing the history deltas with each period
        '''
        output = []
        found_at_least_one_nonzero = False
        tz = pytz.timezone(timezone_text)
        refill_threshold = 0.0
        REFILL_PERCENTANGE = (10/100)

        # Only allow this to be used for the totalizer and tank level attributes
        if not ('totalizer_grand' in attribute) and attribute != 'tank_level':
            raise ValueError('Invalid attribute')

        if not num_periods:
            if history_period_type == History.BY_MONTH:
                num_periods = 4
            elif history_period_type == History.BY_DAY:
                num_periods = 7
            else:
                raise ValueError('Invalid history type')

        # We want the data at the very end of the day, not at all in the next day. But we want to include today.
        today_utc = datetime.utcnow().replace(tzinfo=pytz.utc)
        today_user_tz = today_utc.astimezone(tz)
        today_combined = datetime.combine(today_user_tz.date(), time())  # Start of day
        tomorrow = today_combined + timedelta(days=1)
        today_just_before_midnight = tomorrow - timedelta(seconds=1)
        end_of_day = today_just_before_midnight

        if history_period_type == History.BY_MONTH:
            month_start = datetime(today_combined.year, today_combined.month, 1)

            # End of this month is basically the same as the start of the next month
            end_of_period = month_start + relativedelta(months=1)

            # The start of the series of months
            end_of_period -= relativedelta(months=num_periods)
        elif history_period_type == History.BY_DAY:
            end_of_period = end_of_day
            end_of_period -= relativedelta(days=num_periods)
        elif history_period_type == History.BY_LAST_24_HOURS:
            end_of_period = datetime.combine(today_utc.date(), today_utc.time())
            end_of_period -= relativedelta(days=num_periods)    # 24 hours before now
        else:
            raise ValueError('Invalid history type')

        last_totalizer = None

        '''
        Move the query for the period data outside of the loop.

        This had been inside the loop in the past, which explains the strange structure of this section.
        The goal is to require minimal changes inside of that loop.
        '''

        t2 = tz.localize(end_of_period).astimezone(pytz.utc)
        
        if history_period_type == History.BY_MONTH:
            t1 = t2 - relativedelta(months=1)
        else:
            t1 = t2 - timedelta(days=1)
        history_all_periods = list(History.objects.filter(pump_id=pump_id, attribute=attribute, timestamp__lte=end_of_day, timestamp__gt=t1).order_by('timestamp').all())

        # Offset of two periods to get us back to the end of the "now" period
        for period in range(0, num_periods + 1):
            period_end = tz.localize(end_of_period).astimezone(pytz.utc)

            if history_period_type == History.BY_MONTH:
                period_start = period_end - relativedelta(months=1)
            else:
                period_start = period_end - timedelta(days=1)

            if period == 0:
                '''
                For the time before the first (earliest) period, we need to find something to
                serve as a reference since the totalizer is always counting up. Therefore, we
                search for the newest data point from before the first period.
                '''
                history_in_period = list(History.objects.filter(pump_id=pump_id, attribute=attribute, timestamp__lte=period_end).order_by('timestamp').reverse()[:1])
            else:
                history_in_period = [x for x in history_all_periods if x.timestamp > period_start and x.timestamp <= period_end]
            
            '''
            Sum the marginal increases in the totalizer for every history entry within the time period. The total is the period's total.
            '''
            if max_volume and attribute == 'tank_level':
                refill_threshold = float(max_volume) * REFILL_PERCENTANGE
                
            running_total = 0
            for epsilon in history_in_period:
                '''
                For totalizer_grand, if there's an indication that the totalier was reset during this interval, ignore
                its contribution to the results and use it as the new basis for the next interval
                in the period.  For tank_level, determine whether a refill happened & if so, use it as the new basis, otherwise
                add/subtract small increases & decreases from running total.
                '''
                if 'totalizer_grand' in attribute:
                    period_gals = float(epsilon.value)
                    if last_totalizer is not None and period_gals > last_totalizer:
                        running_total += (period_gals - last_totalizer)
                    last_totalizer = period_gals
                elif attribute == 'tank_level':
                    period_gals = float(epsilon.value)
                    if last_totalizer is not None:
                        delta = (period_gals - last_totalizer)
                        if delta < 0:
                            ''' Level Decreasing '''
                            running_total -= abs(delta)
                            last_totalizer = period_gals
                        else:
                            ''' Increase Detected '''
                            if delta > refill_threshold:
                                ''' Refill Detected, large increase, ignore its contribution to the results & use it as the new basis '''
                                last_totalizer = period_gals
                            else:
                                ''' Include it's contribution to the results.  Noise will cancel itself out eventually '''
                                running_total += delta
                                last_totalizer = period_gals
                    else:
                        last_totalizer = period_gals
                    
            ''' Flip the sign on negative totals for tank level, which can have negative totals due to noise causing level to go up and down. '''
            if running_total < 0 and attribute == 'tank_level':
                running_total = -running_total
                    
            '''
            Store data and adjust to the next period end time

            The first period is just for a reference point; it's not for actual data
            '''
            if period > 0:
                if history_period_type == History.BY_MONTH:
                    # Note the use of period_start!
                    output.append((period_start, running_total))
                else:
                    output.append((period_end, running_total))

            if history_period_type == History.BY_MONTH:
                end_of_period += relativedelta(months=1)
            else:
                end_of_period += timedelta(days=1)

            if running_total > 0:
                found_at_least_one_nonzero = True

        # Clean up artifacts related to the pump having at least one ancient data point but nothing in the current range
        if not found_at_least_one_nonzero:
            output = []

        return output

    @staticmethod
    def get_history_all(pump_id, attribute, timezone_text, day_limit=7):
        '''Get all data points for the given attribute in the history for the given pump, in
        the user's local time zone, up to the specified number of days'''
        output = []

        # We want the data at the very end of the day, not at all in the next day. But we want to include today.
        today_utc = datetime.utcnow().replace(tzinfo=pytz.utc)
        early_limit = today_utc - timedelta(days=day_limit)

        data_points_raw = History.objects.filter(pump_id=pump_id, attribute=attribute, timestamp__gte=early_limit).order_by('timestamp')

        for data_point in data_points_raw:
            output.append((data_point.timestamp, data_point.value))

        return output

    @staticmethod
    def copy_history(pump_id_orig, pump_id_dest, attribute=None, offset=None, start_date=None, end_date=None, delete_in_dest_date_range=False):
        """Copy the history for one pump to another pump, constrained to the given attribute (or all attributes if none specified)"""
        filter_terms = {}
        delete_queryset = None

        if attribute is not None:
            filter_terms['attribute'] = attribute

        '''
        Allow filtering on start and end dates
        '''
        if start_date is not None:
            filter_terms['timestamp__gte'] = start_date
        if end_date is not None:
            filter_terms['timestamp__lte'] = end_date

        '''
        Allow the data in the specified range for the destination pump to be automatically deleted
        '''
        if delete_in_dest_date_range:
            delete_filter = {}
            if start_date is None and end_date is None:
                raise Exception('Destination-delete specified, but no date bounds provided. Cannot proceed.')
            else:
                if start_date is not None:
                    delete_filter['timestamp__gte'] = start_date
                if end_date is not None:
                    delete_filter['timestamp__lte'] = end_date
                if attribute is not None:
                    delete_filter['attribute'] = attribute
            delete_queryset = History.objects.filter(pump_id=pump_id_dest, **delete_filter)

        data_queryset = History.objects.filter(pump_id=pump_id_orig, **filter_terms)

        # Don't delete the destination data unless a plausible replacement exists
        if delete_in_dest_date_range:
            if data_queryset.count() > 0:
                delete_queryset.delete()
                print('Deleted history for destination pump in time range')
            else:
                raise Exception('Destination pump does not contain data in specified range, but delete-in-dest enabled. Error.')

        for obj in data_queryset:
            obj.pk = None  # This effectively allows the copy. If the primary key is none, a new one will be creased on copy.
            obj.pump_id = pump_id_dest

            if offset:
                try:
                    val = int(obj.value) + offset
                    obj.value = str(val)
                except:
                    pass

            obj.save()

    @staticmethod
    def delete_phantom(pump_id):
        '''Delete phantom data (history entries from after the last real disconnection of the given pump)'''
        last_connected = None
        entry_count = 0
        history_entries = []

        try:
            pump = Pump.objects.get(unique_id=pump_id)

            if not pump.connection:
                last_connected = pump.last_seen

            if last_connected is not None:
                history_entries = History.objects.filter(pump_id=pump.unique_id, timestamp__gte=last_connected)
                entry_count = history_entries.count()
                if entry_count > 0:
                    history_entries.delete()

                print('%u entries deleted for pump %s starting at %s' % (entry_count, pump.unique_id, last_connected))

        except Exception as e:
            print('Error: %s' % repr(e))


class Plan(models.Model):
    '''Subscription plans'''
    id = models.AutoField(primary_key=True)
    CURRENCY_CODE = 'USD'

    name = models.CharField(max_length=100, blank=False)
    code = models.CharField(max_length=100, default=None, null=True)

    price_cents = models.IntegerField(default=0)
    setup_cents = models.IntegerField(default=0)

    trial_interval = models.IntegerField(default=0)
    trial_units = models.CharField(max_length=100, default=None, null=True)

    plan_interval = models.IntegerField(default=0)
    plan_units = models.CharField(max_length=100, default=None, null=True)

    is_available = models.BooleanField(default=True)

    user_selectable = models.BooleanField(default=False)

    # Minimum and maximum number of pumps in a group for this to be auto-selected
    # To disable auto-selection, set max_pumps to 0
    min_pumps = models.IntegerField(default=0)
    max_pumps = models.IntegerField(default=0)


class SitePreference(models.Model):
    '''Preferences for the entire site, set by admins'''
    id = models.AutoField(primary_key=True)
    PREF_DISABLE_ALL_SUBSCRIPTIONS = 'disable_all_subscriptions'
    PREF_NOTIFY_UPCOMING_PAYMENT_SWITCHOVER = 'notify_payment_switchover'

    key = models.CharField(max_length=100, null=False, blank=False)
    value = models.CharField(max_length=100, null=False, blank=False)

    @staticmethod
    def get_pref(key, default=None):
        '''Get the named preference, or return the given default if none exists'''
        result = default

        try:
            pref = SitePreference.objects.get(key=key)
            result = pref.value
        except:
            pass

        return result


class Customer(models.Model):
    '''Customers are the owners of pumps'''
    id = models.AutoField(primary_key=True)
    NONE_CUSTOMER_NAME = 'None'
    id = models.AutoField(primary_key=True)
    organization_name = models.CharField(max_length=200, unique=True, blank=False)
    manager = models.ForeignKey(User, blank=True, null=True, on_delete=models.SET_NULL)

    # Allow the subscription requirement to be overridden. This will allow the members of this group
    # to use the group without the group having a valid subscription.
    override_subscription = models.BooleanField(default=False)

    def get_user_id_name_list(self):
        '''Get list of (id,name) for all users associated with this customer'''
        userprofiles = self.userprofile_set.all()
        ret_val = []

        for userprofile in userprofiles:
            ret_val.append({'id': userprofile.user.pk,
                            'name': userprofile.get_username()})

        return ret_val

    def get_pump_id_name_list(self):
        '''Get list of (id,name) for all pumps associated with this customer'''
        pumps = Pump.objects.filter(customer=self, is_active=True)

        ret_val = []
        for pump in pumps:
            pump_name = pump.pretty_name
            if len(pump_name) == 0:
                pump_name = pump.unique_id
            ret_val.append({'id': pump.unique_id,
                            'name': pump_name})

        return ret_val

    def is_none_customer(self):
        '''Determine if this is the None customer'''
        return (self.organization_name == self.NONE_CUSTOMER_NAME)

    def get_current_subscription(self):
        '''Get the current subscription for this customer'''
        sub = None

        if not self.is_none_customer() and not self.override_subscription:
            sub = self.subscription_set.filter(status__in=Subscription.ACTIVE_STATUSES).order_by('id').reverse()[:1].first()

        return sub

    def get_all_subscriptions(self):
        '''Get all subscritpions, past and present, for this customer'''
        subs = []

        if not self.is_none_customer():
            subs = self.subscription_set.order_by('-id')

        return subs

    def get_all_payment_accounts(self, user=None):
        '''
        Get all payment accounts, past and present, that have been used for this customer
        with an optional filter for specific users
        '''
        payment_accounts = []

        subs = self.get_all_subscriptions()
        for sub in subs:
            if sub.account:
                payment_account = sub.account
                if payment_account not in payment_accounts:
                    if user is None or payment_account.user == user:
                        payment_accounts.append(payment_account)

        return payment_accounts

    def subscription_global_override(self):
        return (SitePreference.get_pref(SitePreference.PREF_DISABLE_ALL_SUBSCRIPTIONS, default="0") == "1")

    def subscription_overridden(self):
        '''Determine if the subscription requirement has been defeated for this group'''
        result = False

        if self.override_subscription or self.subscription_global_override():
            result = True

        return result

    def has_valid_subscription(self):
        '''Determine whether this customer has a valid subscription'''
        valid_sub = True
        if self.is_none_customer():
            valid_sub = False
        else:
            # Check for group-level subscription override
            if not self.subscription_overridden():
                # Check for valid group subscription
                if self.subscription_set.filter(status__in=Subscription.ACTIVE_STATUSES).count() == 0:
                    valid_sub = False
        return valid_sub

    def active_pump_count(self):
        '''Count the number of active (non-suspended) pumps in this group'''
        count = Pump.objects.filter(customer=self, suspended=False, is_active=True).count()
        return count

    def suspended_pump_count(self):
        '''Count the number of suspended pumps in this group'''
        count = Pump.objects.filter(customer=self, suspended=True, is_active=True).count()
        return count


class PaymentAccount(models.Model):
    '''Local representation of a Recurly account'''
    id = models.AutoField(primary_key=True)
    USE_EXISTING_CARD = -1

    '''
    The payment account code is a function of both the user's ID and
    the group's ID. This was done so that a single user could pay for subscriptions
    to multiple groups. Otherwise, Recurly restricts a user to a single
    instance of a subscription plan.
    '''
    account_code = models.CharField(max_length=50, null=True)  # Combined user ID and group ID

    user = models.ForeignKey(User, blank=True, null=True, on_delete=models.SET_NULL)  # who signed up for this
    customer = models.ForeignKey(Customer, blank=True, null=True, on_delete=models.SET_NULL)
    is_active = models.BooleanField(default=True)
    balance = models.IntegerField(default=0)

    def formatted_balance(self):
        '''Get a formatted version of the account balance'''
        if self.balance < 0:
            pretty_balance = '(${:.2f})'.format(self.balance / -100.0)
        else:
            pretty_balance = '${:.2f}'.format(self.balance / 100.0)

        return pretty_balance

    @staticmethod
    def generate_account_code(user, customer):
        '''Generate an account code if one doesn't exist'''
        acct = None

        try:
            acct = PaymentAccount.objects.get(user=user, customer=customer)
            code = acct.account_code
        except:
            # Make the account code at least somewhat unique, though not guaranteed
            source_set = string.hexdigits
            random_str = ''.join(random.SystemRandom().sample(source_set * 10, 8))
            code = '%u_%u_%s' % (user.id, customer.id, random_str)

        if not acct:
            PaymentAccount.objects.create(user=user, customer=customer, account_code=code)
        else:
            acct.account_code = code
            acct.save()

        return code


class Subscription(models.Model):
    '''Known subscriptions, both active and inactive'''
    id = models.AutoField(primary_key=True)
    # Status types based on Recurly's state types
    STATUS_UNKNOWN = 0
    STATUS_ACTIVE = 1
    STATUS_EXPIRED = 2
    STATUS_REFUNDED = 3
    STATUS_CANCELED = 4
    STATUS_IN_TRIAL = 5
    STATUS_LIVE = 6
    STATUS_PAST_DUE = 7
    STATUS_CANCELATION_PENDING = 8

    STATUS_TEXT = {STATUS_UNKNOWN: 'Unknown',
                   STATUS_ACTIVE: 'Active',
                   STATUS_EXPIRED: 'Expired',
                   STATUS_REFUNDED: 'Refunded',
                   STATUS_CANCELED: 'Canceled',
                   STATUS_IN_TRIAL: 'In trial',
                   STATUS_LIVE: 'Live',
                   STATUS_PAST_DUE: 'Past due',
                   STATUS_CANCELATION_PENDING: 'Cancelation pending',
                   }

    ACTIVE_STATUSES = [STATUS_ACTIVE,
                       STATUS_IN_TRIAL,
                       STATUS_LIVE,
                       STATUS_PAST_DUE,
                       ]

    SUBSCRIPTION_EXPIRED_MESSAGE = 'The subscription for this group has expired. Functionality will be very limited. Please renew the subscription for this group to restore functionality.'

    # A subscription can have only one account; an account can have many subscriptions over time
    account = models.ForeignKey(PaymentAccount, null=True, on_delete=models.SET_NULL)

    # A subscription can have only one customer; a customer can have many subscriptions over time
    customer = models.ForeignKey(Customer, null=True, on_delete=models.SET_NULL)

    plan = models.ForeignKey(Plan, blank=True, null=True, on_delete=models.SET_NULL)
    status = models.IntegerField(default=STATUS_UNKNOWN)

    start = models.DateTimeField(null=True, default=None)
    expiration = models.DateTimeField(null=True, default=None)

    trial_start = models.DateTimeField(null=True, default=None)
    trial_end = models.DateTimeField(null=True, default=None)

    quantity = models.IntegerField(default=0)

    # If an admin overrode the selected plan for this subscription, don't change it
    admin_overrode_plan_selection = models.BooleanField(default=False)

    recurly_uuid = models.CharField(max_length=200, null=True, default=None)

    def terminate(self):
        '''Immediately cancel, deactivate, and refund (pro-rated) this subscription'''
        recurly_sub = recurly.Subscription.get(self.recurly_uuid)

        '''
        Check if the subscription is in a trial period, and if so, do a "none" refund type 
        instead of a full or partial refund, or else Recurly will kick back an error and
        fail to do the termination
        '''
        in_trial = self.in_trial()
        if in_trial:
            recurly_sub.terminate(refund='none')
            self.status = self.STATUS_CANCELED
        else:
            recurly_sub.terminate(refund='partial')
            self.status = self.STATUS_REFUNDED
        self.save()

        # Un-suspend all pumps associated with this subscription so that the
        # user can make a new subscription
        pumps = Pump.objects.filter(customer=self.customer, suspended=True).all()
        for pump in pumps:
            pump.suspended = False
            pump.save()

    def in_trial(self):
        '''
        Determine if this subscription is in a trial period.

        Hits the Recurly API, so currently somewhat expensive to call

        Returns true if a trial appears to be active; false otherwise.
        '''
        result = False

        # Get current UTC time
        now = datetime.utcnow().replace(tzinfo=pytz.utc)

        # Get the trial end time, if any, from the Recurly object
        try:
            recurly_sub = recurly.Subscription.get(self.recurly_uuid)
            if recurly_sub.trial_ends_at and self.status in Subscription.ACTIVE_STATUSES:
                if recurly_sub.trial_ends_at > now:
                    result = True
        except:
            result = False

        return result

    def monthly_charge(self):
        '''Compute the monthly charge given the number of active pumps and the plan rate'''
        per_pump = self.plan.price_cents
        pump_count = self.customer.active_pump_count()

        charge = per_pump * pump_count

        return (charge / 100.0)


def create_graco_user(email='', customer=None, is_admin=False, time_zone=None, show_disconnected_pumps=True, display_units=None, enable_notifications=True, password=None, is_active=False):
    '''Create a user'''
    char_set = string.ascii_uppercase + string.ascii_lowercase + string.digits
    admin_group = Group.objects.get(name='Admins')

    if not customer:
        customer = Customer.objects.get(pk=get_none_customer())

    if not password or len(password) == 0:
        password = ''.join(random.SystemRandom().sample(char_set * 6, 12))

    username = UserProfile.generate_username()
    user = User.objects.create_user(username, email=email, password=password)

    # Users are usually inactive until their email addresses have been verified
    user.is_active = is_active

    if is_admin:
        user.groups.add(admin_group)

    user.save()

    up = UserProfile.objects.create(user=user)
    up.customers.add(customer)

    # Email address needs to be confirmed
    up.email_confirmed = False

    if time_zone is None or time_zone not in pytz.all_timezones:
        time_zone = pytz.timezone('UTC')
    user.userprofile.time_zone = time_zone

    if display_units is not None and int(display_units) < len(UnitsOfMeasure.TEXT_SYSTEM) and int(display_units) >= 0:
        user.userprofile.display_units = int(display_units)
    else:
        user.userprofile.display_units = UnitsOfMeasure.DEFAULT_UNITS

    if show_disconnected_pumps is not None:
        user.userprofile.show_inactive_pumps = show_disconnected_pumps

    if enable_notifications is not None:
        user.userprofile.enable_notifications = enable_notifications

    up.save()

    return user


class UnitsOfMeasure(object):
    '''For conversion among units'''

    IMPERIAL = 0
    METRIC = 1

    DEFAULT_UNITS = IMPERIAL

    TEXT_SYSTEM = ['U.S.',
                   'Metric']
    TEXT_VOLUME = ['gal',
                   'L']
    TEXT_PRESSURE = ['PSI',
                     'bar']
    TEXT_TANK = ['gal',
                 'L',
                 '%']
    TEXT_TEMP = ['\u00B0F',
                 '\u00B0C']

    VOLUME_CONV_FACTOR = [1.0, 3.78541]
    PRESSURE_CONV_FACTOR = [1.0, 0.06895]

    @staticmethod
    def convert_pressure(origin_system, destination_system, value):
        '''Convert the given pressure from the given units to the given units'''
        imperial_intermediate = value / UnitsOfMeasure.PRESSURE_CONV_FACTOR[origin_system]
        result = imperial_intermediate * UnitsOfMeasure.PRESSURE_CONV_FACTOR[destination_system]
        return result

    @staticmethod
    def convert_volume(origin_system, destination_system, value):
        '''Convert the given volume from the given units to the given units'''
        imperial_intermediate = value / UnitsOfMeasure.VOLUME_CONV_FACTOR[origin_system]
        result = imperial_intermediate * UnitsOfMeasure.VOLUME_CONV_FACTOR[destination_system]
        return result
    
    @staticmethod
    def convert_temperature(origin_system, destination_system, value):
        '''Convert the given temperature from the given units to the given units'''
        if origin_system == UnitsOfMeasure.IMPERIAL and destination_system == UnitsOfMeasure.METRIC:
            result = UnitsOfMeasure.fahrenheit_to_celsius(value)
        elif origin_system == UnitsOfMeasure.METRIC and destination_system == UnitsOfMeasure.IMPERIAL:
            result = UnitsOfMeasure.celsius_to_fahrenheit
        else:
            result = value
        return result
    
    @staticmethod
    def fahrenheit_to_celsius(value):
        result = (value - 32)*(5/9)
        return result
    
    @staticmethod
    def celsius_to_fahrenheit(value):
        result = value*(9/5) + 32
        return result

class TermsOfService(models.Model):
    '''Terms of service'''
    id = models.AutoField(primary_key=True)
    content = models.TextField(null=True, default=None)


class UserProfile(models.Model):
    '''Extension of the User class, sort of, since apparently Django doesn't tolerate actual changes to the User class very well'''
    id = models.AutoField(primary_key=True)
    ADMIN_GROUP_NAME = 'Admins'
    DISTRIBUTOR_GROUP_NAME = 'Distributors'
    USERNAME_REGEX = r'[\s<"\']'
    PASSWORD_MIN_LENGTH = 8
    PASSWORD_REQUIREMENT_MESSAGE = 'Passwords must contain at least one letter and one number and be at least %u characters long' % (PASSWORD_MIN_LENGTH,)
    REGISTRATION_KEY_LENGTH = 40
    EMAIL_REGEX = r'^[^@\s]+@[^@\s]+\.[^@\s]+$'
    USERNAME_MAGIC_KEY = '+""+'
    USERNAME_GENERATE_LENGTH = 20

    EMAIL_UNKNOWN = 0
    EMAIL_USER_ACTIVATION = 1
    EMAIL_CHANGE_VERIFICATION = 2
    EMAIL_USER_ADMIN_CREATED = 3

    user = models.OneToOneField(User, on_delete=models.SET_DEFAULT, default=None)
    customers = models.ManyToManyField(Customer)

    last_activity = models.DateTimeField(default=datetime(2014, 12, 19, 10, 32, 23, 347812))

    show_inactive_pumps = models.IntegerField(default=1)  # used to be a boolean
    show_unassigned_pumps = models.IntegerField(default=1)

    time_zone = models.CharField(default='UTC', max_length=100)
    display_units = models.IntegerField(default=UnitsOfMeasure.IMPERIAL)

    enable_alarm_alerts = models.IntegerField(default=1)

    email_confirmed = models.BooleanField(default=True)
    enable_notifications = models.IntegerField(default=1)  # Not a Boolean due to how the toggle widget needs to work, which in turn is due to how the pump status is conveyed as more than just binary
    password_change_needed = models.BooleanField(default=False)  # Whether to force a password change on login

    tos_agreement_date = models.DateTimeField(default=None, null=True)
    tos_agreed = models.ForeignKey(TermsOfService, null=True, default=None, on_delete=models.SET_DEFAULT)

    verification_key = models.CharField(default=None, null=True, max_length=128)

    def is_admin(self):
        '''Check if this user is an admin'''
        ret_val = False
        if self.user.groups.filter(name=self.ADMIN_GROUP_NAME).exists():
            ret_val = True

        return ret_val

    def make_admin(self, new_admin_state=True):
        '''Change whether this user is an admin'''
        admin_group = Group.objects.filter(name=self.ADMIN_GROUP_NAME).first()
        if new_admin_state:
            self.user.groups.add(admin_group)
        else:
            self.user.groups.remove(admin_group)
        self.user.save()

    def is_distributor(self):
        '''Check if this user is a distributor'''
        ret_val = False
        if self.user.groups.filter(name=self.DISTRIBUTOR_GROUP_NAME).exists():
            if self.user.is_active:
                ret_val = True
        return ret_val

    def make_distributor(self, new_distributor_state=True):
        '''Change whether this user is a distributor'''
        distributor_group = Group.objects.filter(name=self.DISTRIBUTOR_GROUP_NAME).first()
        if new_distributor_state:
            self.user.groups.add(distributor_group)
        else:
            self.user.groups.remove(distributor_group)
        self.user.save()

    def get_username(self):
        '''
        Get the user's proper username, either the email address or the username depending on the situation.

        This exists to facilitate legacy support for old usernames (before the primary username became the email address).
        '''
        effective_username = None

        # If the username has the magic token, it's the new style
        if self.user.username.startswith(self.USERNAME_MAGIC_KEY):
            # Verify that email address is present and looks sane
            if re.match(self.EMAIL_REGEX, self.user.email):
                effective_username = self.user.email
        else:
            # Still using a legacy username
            effective_username = self.user.username

        if effective_username is None:
            raise ValueError('Username not valid')

        return effective_username

    def is_old_style_username(self):
        '''Return true if the user's username is not the new-style username'''
        return not self.user.username.startswith(self.USERNAME_MAGIC_KEY)

    def convert_username(self):
        '''Convert the user's username to the new style (i.e., mostly random)'''
        # Verify that the user has an email address
        if not re.match(self.EMAIL_REGEX, self.user.email):
            raise ValueError('Invalid email address')

        # Ensure that the email address is not already in use with another account
        if User.objects.filter(email__iexact=self.user.email).count() != 1:
            raise ValueError('Email address already in use')

        # Generate a new-style username
        new_username = UserProfile.generate_username()

        # Save the new username
        self.user.username = new_username
        self.user.save()

    @staticmethod
    def generate_username():
        '''Generate a new-style (random) username. Static. Does not save or use the new username on its own.'''
        # Generate a random string
        char_set = string.ascii_uppercase + string.ascii_lowercase + string.digits
        MAX_ITERS = 10
        iters = 0

        # Ensure no collisions
        while iters < MAX_ITERS:
            potential_username = UserProfile.USERNAME_MAGIC_KEY + ''.join(random.SystemRandom().sample(char_set * 15, UserProfile.USERNAME_GENERATE_LENGTH))
            if not User.objects.filter(username=potential_username).exists():
                break
            iters += 1

        if iters >= MAX_ITERS:
            raise Exception('Could not generate username')

        return potential_username

    @staticmethod
    def check_password_requirements(prospective_password, return_reason=False):
        """
        Check a prospective password to see if it meets requirements.

        Returns True if passes or False if invalid, unless return_reason is True, in which case the reason
        for the failure is returned
        """
        retVal = True

        if len(prospective_password) < UserProfile.PASSWORD_MIN_LENGTH:
            retVal = 'Password must be at least %u characters' % (UserProfile.PASSWORD_MIN_LENGTH,)
        elif not re.search(r'[0-9]', prospective_password):
            retVal = 'Password must contain at least one number'
        elif not re.search(r'[a-zA-Z]', prospective_password):
            retVal = 'Password must contain at least one letter'

        if not return_reason:
            if retVal is not True:
                retVal = False

        return retVal

    def send_email_verification(self, email_type):
        '''Generation a key and attempt to verify the user's email address'''
        ret_val = False

        try:
            self.generate_verification_key()

            # Need to have a plausibly valid email address
            if re.match(self.EMAIL_REGEX, self.user.email):

                domain = SITE_DOMAIN
                email_origin_address = settings.EMAIL_ORIGIN_ADDRESS
                secure = 's' if USE_HTTPS else ''

                # Generate an email appropriate for this notification, and insert
                if email_type == self.EMAIL_USER_ACTIVATION:
                    mail_template_text = get_template('emails/activation_email.txt')
                    mail_template_html = get_template('emails/activation_email.html')
                    verification_url = reverse('user_register_verify', kwargs={'verification_key': self.verification_key})
                    subject = 'Account activation'
                elif email_type == self.EMAIL_USER_ADMIN_CREATED:
                    mail_template_text = get_template('emails/account_created_by_admin.txt')
                    mail_template_html = get_template('emails/account_created_by_admin.html')
                    verification_url = reverse('user_register_verify', kwargs={'verification_key': self.verification_key})
                    subject = 'Account activation'
                elif email_type == self.EMAIL_CHANGE_VERIFICATION:
                    mail_template_text = get_template('emails/verify_email.txt')
                    mail_template_html = get_template('emails/verify_email.html')
                    verification_url = reverse('user_email_verify', kwargs={'verification_key': self.verification_key})
                    subject = 'Email address confirmation'
                else:
                    raise ValueError('Invalid email verification type')

                confirmation_url = 'http%s://%s%s' % (secure, domain, verification_url)

                mail_context = Context({'username': self.get_username(),
                                        'first_name': self.user.first_name,
                                        'confirmation_url': confirmation_url,
                                        })

                mail_text = mail_template_text.render(mail_context)
                mail_html = mail_template_html.render(mail_context)

                from_email = 'Graco <%s>' % (email_origin_address,)
                to_email = self.user.email
                send_mail(subject, mail_text, from_email, [to_email], html_message=mail_html)

                ret_val = True

                self.save()
        except Exception as e:
            log_message = 'Exception during account activation: %s' % (repr(e),)
            log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                           event_type=Log.EVENT_COMMAND,
                                           action=Log.ACTION_UPDATE,
                                           success=Log.STATUS_FAIL,
                                           message=sanitize(log_message),
                                           target_type=Log.TARGET_USER,
                                           target_id=str(self.user.id),
                                           )
            log_entry.save()

        return ret_val

    def generate_verification_key(self):
        '''Generate, but do not save, a new verificatoin key'''
        char_set = string.ascii_uppercase + string.ascii_lowercase + string.digits
        MAX_ITERS = 10
        iters = 0

        # Ensure no collisions
        while iters < MAX_ITERS:
            prospective_key = ''.join(random.SystemRandom().sample(char_set * 15, self.REGISTRATION_KEY_LENGTH))
            if not UserProfile.objects.filter(verification_key=prospective_key).exists():
                break
            iters += 1

        if iters >= MAX_ITERS:
            raise Exception('Could not generate verification key')

        self.verification_key = prospective_key

    @staticmethod
    def register(presented_key):
        '''Try a registration key, and if it's correct, mark the user as active and their email address as confirmed'''
        registered_obj = None

        filtered_key = re.sub(UserProfile.USERNAME_REGEX, '', presented_key)

        if len(filtered_key) == UserProfile.REGISTRATION_KEY_LENGTH:  # guard against missing or abnormally short keys
            query = UserProfile.objects.filter(verification_key__iexact=filtered_key)
            if query.count() == 1:
                registered_obj = query.first()
                registered_obj.email_confirmed = True
                registered_obj.verification_key = None
                registered_obj.save()
                registered_obj.user.is_active = True
                registered_obj.user.save()

        return registered_obj

    def verify(self, presented_key):
        '''Try a registration key, and if it's correct, mark the user as active and their email address as confirmed'''
        success = False

        filtered_key = re.sub(UserProfile.USERNAME_REGEX, '', presented_key)

        if len(filtered_key) == UserProfile.REGISTRATION_KEY_LENGTH:  # Guard against missing or abnormally short keys
            if self.verification_key == filtered_key:
                self.email_confirmed = True
                self.verification_key = None
                success = True
                self.save()

        return success


class Invitation(models.Model):
    '''Invitations to join groups or register for accounts'''
    id = models.AutoField(primary_key=True)
    INVITATION_STATE_UNKNOWN = 0
    INVITATION_STATE_ACCEPTED = 1
    INVITATION_STATE_ACTIVE = 2
    INVITATION_STATE_INACTIVE = 3

    INVITE_CODE_LENGTH = 24

    INVITATION_REGEX = r'[^0-9a-zA-Z]'

    issued_at = models.DateTimeField(auto_now_add=True)
    invited_email = models.CharField(max_length=255, default='')  # the email address the invitation was sent to
    accepted_email = models.CharField(max_length=255, default='')  # the email address used to accept the invitation
    code = models.CharField(max_length=255, default='')
    state = models.IntegerField(default=INVITATION_STATE_UNKNOWN)
    customer_invited = models.ForeignKey(Customer, null=True, default=None, on_delete=models.SET_DEFAULT)  # The customer that the person was invited to join
    invite_from = models.ForeignKey(User, related_name='invitations_created', null=True, default=None, on_delete=models.SET_DEFAULT)  # The user who made the invitation
    invite_accepted_by = models.ForeignKey(User, related_name='+', null=True, default=None, on_delete=models.SET_DEFAULT)  # The user who accepted the invitation (possibly by registering a new account)

    @staticmethod
    def new(inviting_user, invited_email, customer):
        '''Helper for creating a new invitation'''

        if not re.match(UserProfile.EMAIL_REGEX, invited_email):
            raise ValueError('Invalid email address')

        invitation = Invitation.objects.create()
        invitation.invited_email = invited_email
        invitation.invite_from = inviting_user
        invitation.customer_invited = customer

        invitation.generate_invite_code()
        invitation.state = Invitation.INVITATION_STATE_ACTIVE

        invitation.save()

        return invitation

    @staticmethod
    def is_valid(invitation_code):
        '''See if an invitation code would work for accepting an invitation'''
        invitation = None
        filtered_code = re.sub(Invitation.INVITATION_REGEX, '', invitation_code)

        # Check for the code being valid
        if len(filtered_code) > 0:
            invitation_query = Invitation.objects.filter(code=filtered_code)
            if invitation_query.count() != 1:
                # Invalid invitation code
                invitation = None
            else:
                invitation = invitation_query.first()

        # Check for it not already being used
        if invitation and invitation.state != Invitation.INVITATION_STATE_ACTIVE:
            invitation = None

        # Return the object so that it can be updated with the new user's information
        return invitation

    def generate_invite_code(self):
        '''Generate, but do not save, a unique invitation code'''
        char_set = string.ascii_uppercase + string.ascii_lowercase + string.digits
        MAX_ITERS = 10
        iters = 0

        # Ensure no collisions
        while iters < MAX_ITERS:
            prospective_code = ''.join(random.SystemRandom().sample(char_set * 15, self.INVITE_CODE_LENGTH))
            if not Invitation.objects.filter(code=prospective_code).exists():
                break
            iters += 1

        if iters >= MAX_ITERS:
            raise Exception('Could not generate invitation code')

        self.code = prospective_code

    def use(self, user):
        '''Attempt to use an invitation code with the specified user'''
        success = False
        # Check for not already being used
        if user and self.state == Invitation.INVITATION_STATE_ACTIVE:
            # Mark it as used
            try:
                user.userprofile.customers.add(self.customer_invited)

                nc = Customer.objects.get(pk=get_none_customer())
                if nc in user.userprofile.customers.all():
                    user.userprofile.customers.remove(nc)

                self.state = Invitation.INVITATION_STATE_ACCEPTED
                self.accepted_email = user.email
                self.invite_accepted_by = user

                self.save()

                success = True
            except:
                success = False

        return success

    def send(self):
        '''Send the invitation email'''
        ret_val = False

        try:
            # Need to have a plausibly valid email address
            if not re.match(UserProfile.EMAIL_REGEX, self.invited_email):
                raise ValueError('Invitation has bad or missing email address')

            domain = SITE_DOMAIN
            email_origin_address = settings.EMAIL_ORIGIN_ADDRESS
            secure = 's' if USE_HTTPS else ''

            mail_template_text = get_template('emails/invitation_email.txt')
            mail_template_html = get_template('emails/invitation_email.html')
            invitation_url = reverse('user_invitation', kwargs={'invitation_code': self.code})
            subject = 'Invitation to join a Graco pump group'

            invitation_url_full = 'http%s://%s%s' % (secure, domain, invitation_url)

            has_account = False
            if User.objects.filter(email__iexact=self.invited_email, is_active=True).count() == 1:
                has_account = True

            mail_context = Context({'invitation_url': invitation_url_full,
                                    'invite_from': self.invite_from.userprofile.get_username(),
                                    'customer_name': self.customer_invited.organization_name,
                                    'has_account': has_account,
                                    })

            mail_text = mail_template_text.render(mail_context)
            mail_html = mail_template_html.render(mail_context)

            from_email = 'Graco <%s>' % (email_origin_address,)
            to_email = self.invited_email
            send_mail(subject, mail_text, from_email, [to_email], html_message=mail_html)

            self.save()

            ret_val = True

        except Exception as e:
            log_message = 'Exception during account invitation send: %s' % (repr(e),)
            log_entry = Log.objects.create(origin_type=Log.ORIGIN_WEB,
                                           event_type=Log.EVENT_COMMAND,
                                           action=Log.ACTION_SEND,
                                           success=Log.STATUS_FAIL,
                                           message=sanitize(log_message),
                                           target_type=Log.TARGET_INVITATION,
                                           target_id=self.id,
                                           )
            log_entry.save()

        return ret_val


class MqttAuth(models.Model):
    """
    Credentials for the mosquitto-auth-plug MQTT plugin

    The moquitto-auth-plug plugin uses screwy non-standard pbkff2 conventions, so we have to do some work to
    get our hashes to be in a form that the plugin can decode
    """
    id = models.AutoField(primary_key=True)

    '''
    Constants
    '''
    MIN_USERNAME_LEN = 8
    MIN_PW_LEN = 10
    MQTT_USERNAME_PREFIX = 'M!'
    MQTT_USERNAME_TOTAL_LENGTH = 16
    MQTT_PASSWORD_TOTAL_LENGTH = 16

    '''
    Fields
    '''

    username = models.CharField(max_length=24)
    pw_hashed = models.CharField(max_length=255, null=True)

    # Need to store the password in the clear so that it can be sent to the pump when the pump requests its credentials
    pw_clear = models.CharField(max_length=255, null=True)

    superuser = models.BooleanField(default=False)

    '''
    Methods
    '''
    @staticmethod
    def generate_password():
        """Generate a password"""
        char_set = string.ascii_uppercase + string.ascii_lowercase + string.digits
        length = MqttAuth.MQTT_PASSWORD_TOTAL_LENGTH

        pw_clear = ''.join(random.SystemRandom().sample(char_set * 15, length))

        return pw_clear

    @staticmethod
    def generate_username():
        """Generate a unique mosquitto username"""
        char_set = string.ascii_uppercase + string.ascii_lowercase + string.digits
        MAX_ITERS = 10
        iters = 0
        length = MqttAuth.MQTT_USERNAME_TOTAL_LENGTH - len(MqttAuth.MQTT_USERNAME_PREFIX)

        # Ensure no collisions
        while iters < MAX_ITERS:
            potential_username = MqttAuth.MQTT_USERNAME_PREFIX + ''.join(random.SystemRandom().sample(char_set * 15, length))
            if not MqttAuth.objects.filter(username=potential_username).exists():
                break
            iters += 1

        if iters >= MAX_ITERS:
            raise Exception('Could not generate username')

        return potential_username

    @staticmethod
    def new_credentials(pump):
        '''Create new credentials for the given pump and associate them with that pump'''
        username = MqttAuth.generate_username()
        password = MqttAuth.generate_password()

        credentials = MqttAuth.set_password(password=password, username=username, superuser=False)

        return credentials

    @staticmethod
    def set_password(username, password, superuser=False):
        '''
        Create or update an entry with the given MQTT username and cleartext password using the correct hash.

        Normally, you'll want to generate the usernames for pump accounts using the helper function.
        '''
        if username is None:
            raise ValueError('Username required')

        if password is None or not isinstance(password, str):
            raise ValueError('Password requried')

        if len(password) < MqttAuth.MIN_PW_LEN:
            raise ValueError('Password too short')

        if len(username) < MqttAuth.MIN_USERNAME_LEN:
            raise ValueError('Username too short')

        # Generate pbkdf2 hash for password
        new_hash_complete = make_hash(password)

        acct = MqttAuth.objects.filter(username=username).first()

        if acct is None:
            acct = MqttAuth.objects.create(username=username, pw_hashed=new_hash_complete, pw_clear=password, superuser=superuser)
        else:
            acct.pw_hashed = new_hash_complete
            acct.pw_clear = password
            acct.superuser = superuser
            acct.save()

        return acct


class MqttACL(models.Model):
    """
    ACLs for the mosquitto-auth-plug MQTT plugin
    """
    id = models.AutoField(primary_key=True)
    '''
    Constants
    '''
    PERMS_READ_ONLY = 1
    PERMS_READ_WRITE = 2

    '''
    Fields
    '''

    username = models.CharField(max_length=100)
    topic = models.CharField(max_length=255)
    rw = models.IntegerField(default=0)
    mqtt_auth = models.ForeignKey(MqttAuth, blank=True, null=True, on_delete=models.CASCADE)

    '''
    Methods
    '''

    @staticmethod
    def new_acl(pump, credentials):
        """Grant appropriate permissions to a pump and the given credentials"""
        username = credentials.username
        topic_match = '+/%s' % (pump.unique_id,)
        perms = MqttACL.PERMS_READ_WRITE

        if credentials.username is None or len(credentials.username) < MqttAuth.MIN_USERNAME_LEN:
            raise ValueError('Invalid username for ACL update')

        if pump is None or pump.unique_id is None or len(re.sub(Pump.regex_for_clean, '', pump.unique_id)) < 1:
            raise ValueError('Invalid pump for ACL update')

        acls_username = MqttACL.objects.filter(username=username).all()
        if len(acls_username) > 1:
            raise ValueError('Specified username has multiple ACL matches')

        acls_auth = MqttACL.objects.filter(mqtt_auth=credentials).all()
        if len(acls_auth) > 1:
            raise ValueError('Specified credentials have multiple auth matches')

        # Clean out old ACLs, if any
        acls_username.delete()
        acls_auth.delete()

        acl_new = MqttACL.objects.create(username=username, topic=topic_match, rw=perms, mqtt_auth=credentials)

        return acl_new


class Pump(models.Model):
    id = models.AutoField(primary_key=True)
    # Volumes are passed as 10x integers
    SCALING_FACTOR = 10.0

    # Flow rates need a higher resolution
    SCALING_FACTOR_FLOW_RATE = 100.0

    # Tank level is in xxx.x format and passed as xxxx
    SCALING_FACTOR_TANK_LEVEL = 10
    
    # Allow flow verify percentage to have max 200 %
    SCALING_FACTOR_FLOW_VERIFY = 2

    # Battery voltage is reported in mV
    SCALING_FACTOR_BATT = 1000.0
    
    # 4-20mA is x100
    SCALING_FACTOR_MA = 100
    
    # Allow values 10% above the max tank value to display on chart
    MAX_TANK_LEVEL_PERCENTAGE = 110/100

    # Magic number for access to mqtt u/p generation
    MAGIC_NUMBER = '98618265897354'  # as a string

    # Maximum realistic pressure value, in PSI
    MAX_PRESSURE_VALUE = 7500
    
    # Maximum realistic percentage value, in %
    MAX_PERCENTAGE_VALUE = 200

    # Min and max realistic temperature values, in deg F    
    MIN_TEMPERATURE_VALUE = -50
    MAX_TEMPERATURE_VALUE = 140
    
    # Invalid temp to signal pump disconnected, in deg F
    INVALID_TEMPERATURE = 999

    # What to display for the status for expired pumps
    INVALID_SUB_STATUS = -1

    # The topic name where activation keys are published
    ACTIVATION_KEY_TOPIC = 'ActivationKey'

    METERING_UNKNOWN = 0
    METERING_VOLUME = 1
    METERING_TIME = 2
    METERING_CYCLES = 3

    METERING_MODES = {METERING_UNKNOWN: 'Unknown',
                      METERING_VOLUME: 'Flow',
                      METERING_TIME: 'Time',
                      METERING_CYCLES: 'Cycles',
                      }
    
    AIN_UNKNOWN = 0
    AIN_OFF = 1
    AIN_ON = 2
    
    AIN_MODES = {AIN_UNKNOWN: 'Unknown',
                 AIN_OFF: 'Off',
                 AIN_ON: 'On'
                 }
    
    SENSOR_UNKNOWN = 0
    SENSOR_LINE_PRESSURE = 1
    SENSOR_TANK_PERCENTAGE = 2
    SENSOR_LEVEL_CHART = 3
    SENSOR_PRESSURE_CHART = 4
    SENSOR_HORIZONTAL_CYLINDER = 5
    SENSOR_VERTICAL_CYLINDER = 6
    SENSOR_RECTANGLE = 7
    
    SENSOR_TYPES = {SENSOR_UNKNOWN: 'Unknown',
                    SENSOR_LINE_PRESSURE: 'Line Pressure',
                    SENSOR_TANK_PERCENTAGE: 'Tank %',
                    SENSOR_LEVEL_CHART: 'Level Chart',
                    SENSOR_PRESSURE_CHART: 'Pressure Chart',
                    SENSOR_HORIZONTAL_CYLINDER: 'Horizontal Cylinder',
                    SENSOR_VERTICAL_CYLINDER: 'Vertical Cylinder',
                    SENSOR_RECTANGLE: 'Rectangle',
                    }                    
    
    TANK_UNKNOWN = 0
    TANK_VERTICAL = 1
    TANK_HORIZONTAL = 2
    TANK_CUSTOM = 3
    
    TANK_TYPES = {  TANK_UNKNOWN: 'Unknown',
                    TANK_VERTICAL: 'Vertical Tank',
                    TANK_HORIZONTAL: 'Horizontal Tank',
                    TANK_CUSTOM: 'Custom Tank',
                    }
    
    CONTROL_UNKNOWN = 0
    CONTROL_DISABLED = 1
    CONTROL_DISPLAY = 2
    CONTROL_ON_BELOW = 3
    CONTROL_ON_ABOVE = 4
    
    TEMP_CONTROL_OPTIONS = { CONTROL_UNKNOWN: 'Unknown',
                             CONTROL_DISABLED: 'Disabled',
                             CONTROL_DISPLAY: 'Display',
                             CONTROL_ON_BELOW: 'On Below',
                             CONTROL_ON_ABOVE: 'On Above',
                           }

    LOCATION_SOURCE_REPORTED = 0
    LOCATION_SOURCE_MARKED = 1
    LOCATION_SOURCE_MANUAL = 2

    PUMP_ID_MIN_LENGTH = 8
    PUMP_ID_MAX_LENGTH = 18

    POWER_SAVE_OFF = 0
    POWER_SAVE_NOTIFY = 1

    POWER_SAVE_MODES = {POWER_SAVE_OFF: 'Off',
                        POWER_SAVE_NOTIFY: 'Notify'}

    POWER_STATUS_NAMES = ['Standby',
                          'Run',
                          'Disabled by alarm',
                          'Disabled by remote',
                          'Powersave active',
                          'Disabled by temperature', ]
    POWER_STATUS_STANDBY = 0
    POWER_STATUS_RUN = 1
    POWER_STATUS_DISABLED_BY_ALARM = 2
    POWER_STATUS_DISABLED_BY_REMOTE = 3  # disabled-by-remote status ID
    POWER_STATUS_POWERSAVE_ACTIVE = 4
    
    WELL_STATUS_NAMES = ['Disabled',
                         'Enabled', ]
    
    WELL_STATUS_DISABLED = 0
    WELL_STATUS_ENABLED = 1

    MULTIWELL_DISABLED = 0
    MULTIWELL_VARIABLE = 1
    MULTIWELL_FIXED = 2
    
    MULTIWELL_MODES = {MULTIWELL_DISABLED: 'Disabled',
                       MULTIWELL_VARIABLE: 'Variable',
                       MULTIWELL_FIXED: 'Fixed',
                       }

    POWER_STATUS_DISABLED_BY_REMOTE_ALARM_ID = 11  # disabled-by-remote alarm ID

    PSEUDO_ALARMS = ['Unknown',
                     'Default',
                     'Pump no longer connected',
                     ]
    PSEUDO_ALARM_DISCONNECTION = -2

    ALARM_ID_DEFAULT = -1

    regex_for_clean = r'[^a-zA-Z_0-9\.]'
    ASSOCIATION_KEY_FILTER = regex_for_clean

    unique_id = models.CharField(max_length=100, blank=False, unique=True)
    pretty_name = models.CharField(max_length=200, default='')
    location_name = models.CharField(max_length=200, default='')
    connection = models.BooleanField(default=False)
    pump_topology = models.CharField(max_length=50, default='')

    status = models.IntegerField(default=0)  # used to be a Boolean
    flow_rate = models.FloatField(default=0)
    totalizer_resetable = models.FloatField(default=0)
    totalizer_grand = models.FloatField(default=0)
    alarms_status = models.IntegerField(default=0)
    metering_mode = models.IntegerField(default=0)
    sensor_type = models.IntegerField(default=0)
    tank_type = models.IntegerField(default=0)
    firmware_version = models.CharField(default='', max_length=20)
    metering_on_time = models.IntegerField(default=0)
    metering_off_time = models.IntegerField(default=0)
    metering_on_cycles = models.IntegerField(default=0)
    metering_on_timeout = models.IntegerField(default=0)
    location_reported = models.CharField(max_length=200, default='')
    location_source = models.IntegerField(default=0)
    location_marked = models.CharField(max_length=200, default='')
    temperature_control = models.IntegerField(default=0)    

    battery_voltage = models.IntegerField(default=-1)
    pressure_level = models.IntegerField(default=-1)
    tank_level = models.IntegerField(default=-1)
    tank_level_volume_max = models.IntegerField(default=-1)        
    signal_strength = models.IntegerField(default=-1)
    system_publication_period = models.IntegerField(default=-1)
    temperature = models.IntegerField(default=-999)

    activation_key = models.CharField(max_length=30, default='')
    activation_key_last_changed = models.DateTimeField(default=None, null=True)

    # MQTT credentials
    mqtt_auth = models.ForeignKey(MqttAuth, default=None, null=True, on_delete=models.SET_DEFAULT)

    # Alarm trigger levels
    high_pressure_trigger = models.IntegerField(default=-1)
    low_pressure_trigger = models.IntegerField(default=-1)
    low_battery_trigger = models.IntegerField(default=-1)
    battery_warning_trigger = models.IntegerField(default=-1)
    power_save_mode = models.IntegerField(default=-1)
    tank_level_notify_trigger = models.IntegerField(default=-1)    
    tank_level_shutoff_trigger = models.IntegerField(default=-1)    
    flow_verify_percentage = models.IntegerField(default=-1)
    next_flow_to_level_check = models.DateTimeField(default=datetime.min.replace(tzinfo=pytz.utc), null=True)
    flow_verify_enable = models.BooleanField(default=False)
    last_tank_level_totalizer = models.IntegerField(default=-1)
    analog_input_mode = models.IntegerField(default=0)
    raw_analog_in = models.IntegerField(default=-1)
    ain_mA_low = models.IntegerField(default=-1)
    ain_mA_high = models.IntegerField(default=-1)    
    ain_flow_rate_low = models.FloatField(default=0)
    ain_flow_rate_high = models.FloatField(default=0)
    temperature_setpoint = models.IntegerField(default=-999)
    motor_protection_settings = models.IntegerField(default=0)
    motor_current_mV = models.IntegerField(default=-1)
    
    # Whether the pump has had its subscription suspended independent of the group
    suspended = models.BooleanField(default=False)

    # A pump is "owned' by a single customer
    customer = models.ForeignKey(Customer, default=get_none_customer, on_delete=models.SET_DEFAULT)

    # For estimating whether the pump has acted on the most recent command
    timestamp = models.DateTimeField(auto_now=True)
    last_seen = models.DateTimeField(default=None, null=True)
    disconnection_noticed = models.BooleanField(default=True)

    is_active = models.BooleanField(default=True)
    
    # Multiwell fields
    multiwell_enable = models.IntegerField(default=0)   # used to be a Boolean
    well_flow_rate_1 = models.FloatField(default=0)
    well_flow_rate_2 = models.FloatField(default=0)
    well_flow_rate_3 = models.FloatField(default=0)
    well_flow_rate_4 = models.FloatField(default=0)
    well_flow_rate_5 = models.FloatField(default=0)
    well_flow_rate_6 = models.FloatField(default=0)
    well_flow_rate_7 = models.FloatField(default=0)
    well_flow_rate_8 = models.FloatField(default=0)
    
    totalizer_well_1 = models.FloatField(default=0)
    totalizer_well_2 = models.FloatField(default=0)
    totalizer_well_3 = models.FloatField(default=0)
    totalizer_well_4 = models.FloatField(default=0)
    totalizer_well_5 = models.FloatField(default=0)
    totalizer_well_6 = models.FloatField(default=0)
    totalizer_well_7 = models.FloatField(default=0)
    totalizer_well_8 = models.FloatField(default=0)
    
    totalizer_grand_well_1 = models.FloatField(default=0)
    totalizer_grand_well_2 = models.FloatField(default=0)
    totalizer_grand_well_3 = models.FloatField(default=0)
    totalizer_grand_well_4 = models.FloatField(default=0)
    totalizer_grand_well_5 = models.FloatField(default=0)
    totalizer_grand_well_6 = models.FloatField(default=0)
    totalizer_grand_well_7 = models.FloatField(default=0)
    totalizer_grand_well_8 = models.FloatField(default=0)
    
    notes_field = models.CharField(max_length=200, default='') 
    well_status = models.IntegerField(default=0)
    

    def can_suspend(self):
        '''Determine whether the subscription for this pump can be suspended'''
        result = not self.suspended and self.customer.has_valid_subscription() and not self.customer.subscription_overridden()
        return result

    def can_unsuspend(self):
        '''Determine whether the subscription for this pump can be unsuspended'''
        result = self.suspended and self.customer.has_valid_subscription() and not self.customer.subscription_overridden()
        return result

    def has_valid_subscription(self):
        '''Determine whether the customer owning this pump has a valid subscription'''
        valid_sub = True

        if not self.customer.is_none_customer():
            if self.suspended:
                # If the subscription for the pump's group doesn't matter anyway, due to a
                # group-specific override or a site-wide override, then ignore the suspension
                valid_sub = self.customer.subscription_overridden()
            else:
                valid_sub = self.customer.has_valid_subscription()

        return valid_sub

    def get_alarm_name(self, alarm_id):
        '''Get the proper name for the given alarm for this pump'''
        alarm_name = None

        # Check the alarm_id against the list of custom options
        custom_obj = self.alarmcustomization_set.filter(alarm_id=alarm_id).first()

        # Use the custom name if it exists, or else use the default name
        if custom_obj:
            alarm_name = custom_obj.custom_alarm_name
        else:
            if alarm_id >= 0 and alarm_id < len(AlarmList):
                alarm_name = AlarmList[alarm_id]
            elif alarm_id == self.PSEUDO_ALARM_DISCONNECTION:
                alarm_name = self.PSEUDO_ALARMS[-1 * alarm_id]

        return alarm_name

    def get_status_names(self):
        '''Get the available power status names'''

        # Note the deep copy so that our custom name doesn't bleed into other pumps
        status_names = copy.deepcopy(self.POWER_STATUS_NAMES)

        # The disabled-by-remote alarm name can appear as a power status, so we want to use
        # the customized version of that if defined
        custom_remote = self.alarmcustomization_set.filter(alarm_id=self.POWER_STATUS_DISABLED_BY_REMOTE_ALARM_ID).first()
        if custom_remote:
            status_names[self.POWER_STATUS_DISABLED_BY_REMOTE] = custom_remote.custom_alarm_name

        return status_names
    
    def get_well_status_names(self):
        ''' Get the available well status names'''
        
        # Note the deep copy so that our custom name doesn't bleed into other pumps
        well_status_names = copy.deepcopy(self.WELL_STATUS_NAMES)
        
        return well_status_names

    def get_status_true_ids(self):
        '''Get the state IDs corresponding to the pump running'''
        return [self.POWER_STATUS_RUN, self.POWER_STATUS_POWERSAVE_ACTIVE]
    
    def get_well_status_true_ids(self):
        ''' Get the stat IDc corresponding to the well being enabled '''
        return [self.WELL_STATUS_ENABLED]

    def get_firmware_rev_num(self):
        '''Get the firmware version formatted as an integer'''
        firmware_rev_num = 0
         
        firmware_rev_str = re.sub(r'[^0-9]', '', self.firmware_version)
        if firmware_rev_str:
            firmware_rev_num = int(firmware_rev_str)   
        
        return firmware_rev_num
    
    def get_tank_parameter_scale_factor(self):
        '''We need to support different scale factors for any tank parameter that can be set by the user.        
           TLM with metric units use x100 mulitpliers & those without metric units use x10 mulitipliers.

           Old TLM field test units, pre-metric units:   >=1.20.10 & <=1.31.10,
           New production units with TLM & metric units: >=1.10.10 & <1.20.10, 
           New TLM field test units with metric units:   > 1.31.10 '''
         
        firmware_num = self.get_firmware_rev_num()

        if (firmware_num >= 12010 and firmware_num <= 13110):
            # User settable tank related parameters are in xxx.x format and passed as xxxx
            scale_factor_tank_param = 10
        elif (firmware_num >= 111 and firmware_num < 12010) or firmware_num > 13110:
            # User settable tank related parameters are in xxx.xx format and passed as xxxxx
            scale_factor_tank_param = 100
        
        return scale_factor_tank_param
    
    def get_tank_parameter_intermediate(self, parameter):
        ''' Round & scale tank parameters on newer tank level units to only disply 1 decimal of precision'''
        
        firmware_num = self.get_firmware_rev_num()
        
        if (firmware_num >= 111 and firmware_num < 12010) or firmware_num > 13110:
            # Parameters are in xxx.xx *100 (xxxxx) format and need to be rounded & scaled to xxx.x (xxxx) format before displaying 
            tank_parameter_intermediate = int((parameter + self.SCALING_FACTOR_TANK_LEVEL/2) / self.SCALING_FACTOR_TANK_LEVEL)
        else:
            tank_parameter_intermediate = parameter
        
        return tank_parameter_intermediate
    
    def get_flow_rate_scaled(self, flow_rate, user_units):
        # These gymnastics are to match the firmware's strange way of rounding
        flow_rate_unscaled = UnitsOfMeasure.convert_volume(UnitsOfMeasure.IMPERIAL, user_units, flow_rate)
        if user_units == UnitsOfMeasure.METRIC:
            flow_rate_intermediate = int((flow_rate_unscaled + Pump.SCALING_FACTOR / 2) / Pump.SCALING_FACTOR)
            flow_rate_str = '%d.%d' % (int(flow_rate_intermediate / Pump.SCALING_FACTOR), int(flow_rate_intermediate % Pump.SCALING_FACTOR))
        else:
            flow_rate_str = '%d.%02d' % (int(flow_rate_unscaled / Pump.SCALING_FACTOR_FLOW_RATE), int(flow_rate_unscaled % Pump.SCALING_FACTOR_FLOW_RATE))
            
        return flow_rate_str    

    def regenerate_activation_key(self, time_now, publish=True):
        """Generate a unique activation key and publish it"""
        # Generate a string
        # Make the source characters not easily confusable
        source_set = 'ACEFGHJKLMNPRTWXY346789'

        # Verify that the key does not already exist, and generate a new one if it does
        timeout = 10
        success = False
        while timeout > 0 and success is not True:
            new_key = ''.join(random.SystemRandom().sample(source_set * 10, 8))
            # Check for collisions
            if not Pump.objects.filter(activation_key=new_key).exists():
                success = True
            else:
                # Guard against infinite loop
                timeout -= 1

        if success:
            try:
                # Publish the new key (retained flag set)
                if publish:
                    PumpSet.set_pump(self.unique_id, self.ACTIVATION_KEY_TOPIC, new_key, retain=True)
                self.activation_key = new_key
            except:
                success = False

        # If the publication succeeded without errors, update the key-last-changed timestamp
        if success:
            self.activation_key_last_changed = time_now
            self.save()
            message = 'Updated key for pump %s.' % (self.unique_id,)
        else:
            message = 'Error updating key for pump %s.' % (self.unique_id,)
            Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                               action=Log.ACTION_UPDATE,
                               event_type=Log.EVENT_DEBUG,
                               message='Error updating activation key',
                               success=Log.STATUS_FAIL,
                               target_type=Log.TARGET_PUMP,
                               target_id=self.unique_id)

        return message

    @staticmethod
    def publish_all_names():
        '''Publish all known pump names to the MQTT broker'''
        for pump in Pump.objects.all():
            PumpSet.set_pump(pump.unique_id, 'SetPumpName', pump.pretty_name, retain=True)
            print('Published pump name for pump %s' % (pump.unique_id,))

    @staticmethod
    def check_activation_keys(time_now):
        """Check for any blank activation keys and create/publish them as needed"""
        message = ''
        for pump in Pump.objects.all():
            if len(pump.activation_key) < 1:
                message += pump.regenerate_activation_key(time_now)

        if len(message) < 1:
            message += 'ActKeyOK'
        return message
    
    @staticmethod
    def check_flow_verify(time_now):
        """Compares the tank level to the totalizer and sets a flow verify alarm on a mismatch"""
        """Detects poor correlation, no change in tank level, and no change in totalizer"""
        
        FLOW_VERIFY_ALARM_ID = 5
        TANK_HYSTERESIS = 50  # 5.0 gallons
        ONE_HUNDRED_PERCENT = 100.0
        NUM_PUMPS_CHECKED_PER_CRON = 20
        count = 0
        pump_total = 0
        tank_total = 0
        in_window = False
        out_message = ''  # For concatenation, NOT for the Log object
        
        # Get a list of connected pumps that are ready to check the flow verify alarm
        pumps = Pump.objects.filter(connection=True, is_active=True, next_flow_to_level_check__lte=time_now)
        
        try:
            for pump in pumps:
                message = ''
                if pump.flow_verify_enable == True and pump.metering_mode == pump.METERING_VOLUME:
                    count += 1
                    if count <= NUM_PUMPS_CHECKED_PER_CRON:
                        if pump.next_flow_to_level_check.date() != datetime.min.date():
                            totalizer_history = History.get_history_totalizer(pump.unique_id, 'totalizer_grand', History.BY_LAST_24_HOURS, 'UTC', num_periods=1)
                            tank_level_history = History.get_history_totalizer(pump.unique_id, 'tank_level', History.BY_LAST_24_HOURS, 'UTC', num_periods=1, max_volume=pump.tank_level_volume_max)
                            
                            for data in totalizer_history:
                                pump_total = data[1]
                            
                            for data in tank_level_history:
                                # Reduce scaling on tank level so that anything using x100 becomes x10 & anything using x10 remains x10 to match scaling on pump total
                                tank_total = float(data[1]/(pump.get_tank_parameter_scale_factor()/10))
                            
                            # Needs to pump more than 5 gallons per day to indicate fluid is moving.  We assume that a total less than
                            # 5 gallons is due to noise.  If it's due to low flow rates, this alarm should be disabled.
                            if (tank_total < TANK_HYSTERESIS):
                                in_window = True
                            
                            # At least one needs to have valid data, skip the check if neither had data
                            # An empty list indicates no data found or total was 0
                            if tank_level_history or totalizer_history:
                                if not tank_level_history and pump_total > 0:
                                    message = 'No change in tank level'
                                elif in_window == True and pump_total > 0:
                                    message = 'No change in tank level'
                                elif in_window == False and not totalizer_history:
                                    message = 'No change in totalizer'
                                else:
                                    # % difference with totalizer as baseline.  Both totalizer & tank level are x10, so no need to use scale factors
                                    # Skip if pump total = 0, which indicates nothing pumped & tank is bouncing within window so no change in fluid level
                                    if (pump_total > 0):
                                        percent_different = (abs(pump_total - tank_total) / pump_total) * ONE_HUNDRED_PERCENT
                                    
                                        if int(percent_different) >= pump.flow_verify_percentage:
                                            message = 'Poor correlation between totalizer and tank level'
                    
                        # 86400 seconds = 1 day, 1800 seconds = 1/2 hr, ensures pumps are checked every 24 hours +/- 1.5 hours.
                        # Use random offset to ensure pumps don't all get scheduled at once & only a few are checked at a time
                        next_time_in_seconds = random.gauss(86400, 1800)
                        pump.next_flow_to_level_check = time_now + timedelta(seconds=next_time_in_seconds)
                        pump.save()
                        
                        if len(message) > 1:
                            out_message += 'Flow accuracy alarm for pump %s.' % (pump.unique_id,)
                            alarm_bitfield = pump.alarms_status
                            alarm_bitfield |= (1 << FLOW_VERIFY_ALARM_ID)
                            PumpSet.set_pump(pump.unique_id, 'ClearAlarmStatus', alarm_bitfield, retain=False)
                            
                            log_entry = Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                                                   event_type=Log.EVENT_DEBUG,
                                                   action=Log.ACTION_UPDATE,
                                                   success=Log.STATUS_SUCCESS,
                                                   message=message,
                                                   target_type=Log.TARGET_PUMP,
                                                   target_id=pump.unique_id,
                                                   )
                            log_entry.save()                        
    
            if len(out_message) < 1:
                out_message += 'FlowVerifyOK'
        except Exception as e:
            print('Flow accuracy exception: %s' % (repr(e),))
            out_message += 'Error servicing flow accuracy alarm for pump %s.' % (pump.unique_id)
            log_entry = Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                       event_type=Log.EVENT_DEBUG,
                       action=Log.ACTION_UPDATE,
                       success=Log.STATUS_SUCCESS,
                       message=message + ':  %s' % (repr(e),),
                       target_type=Log.TARGET_PUMP,
                       target_id=pump.unique_id,
                       )
             
        return out_message

    @staticmethod
    def check_pump_for_user_registration(association_key):
        """
        Check if a pump is available for a user to register a new account.

        A pump's activation key can be used for registering a new account if does not yet belong to a group.
        """
        if Pump.objects.filter(activation_key__iexact=association_key).count() == 1:
            pump = Pump.objects.filter(activation_key__iexact=association_key).first()
        else:
            pump = None

        # Cannot re-activate a pump that already belongs to somebody else unless they have expired
        if pump:
            if not pump.customer.is_none_customer() and pump.customer.has_valid_subscription():
                pump = None

        return pump

    def update_mosquitto_creds(self):
        '''
        Create or regenerate MQTT credentials/ACL for this pump
        '''
        credentials = None
        acl = None

        try:
            if self.mqtt_auth:
                self.mqtt_auth.delete()
                self.mqtt_auth = None
                self.save()

            credentials = MqttAuth.new_credentials(self)
            acl = MqttACL.new_acl(self, credentials)
        except Exception as e:
            print('Exception in MQTT credentials/ACL: %s' % repr(e))
        finally:
            if acl is not None and credentials is not None:
                self.mqtt_auth = credentials
                self.save()
                log_message = 'Pump credentials regenerated'
                log_entry = Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                                               event_type=Log.EVENT_DEBUG,
                                               action=Log.ACTION_UPDATE,
                                               success=Log.STATUS_SUCCESS,
                                               message=log_message,
                                               target_type=Log.TARGET_PUMP,
                                               target_id=self.unique_id,
                                               )
                log_entry.save()

            else:
                log_message = 'Pump credential generation failed'
                log_entry = Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                                               event_type=Log.EVENT_DEBUG,
                                               action=Log.ACTION_UPDATE,
                                               success=Log.STATUS_FAIL,
                                               message=log_message,
                                               target_type=Log.TARGET_PUMP,
                                               target_id=self.unique_id,
                                               )
                log_entry.save()

    def get_lat_long(self):
        '''Get the latitude and longitude of the reported location of the pump'''
        MIN_ELEMENTS_FOR_LAT_LONG = 3
        LAT = 1
        LONG = 2

        ret_val = ''
        try:
            if self.location_reported and len(self.location_reported) > 0:

                loc_data = self.location_reported.split(',')
                if len(loc_data) >= MIN_ELEMENTS_FOR_LAT_LONG:
                    if len(loc_data[LAT]) > 0 and len(loc_data[LONG]) > 0:
                        latitude = float(loc_data[LAT])

                        # There's a buffer overrun that can cause garbage data to be appended to the longitude
                        filtered_lon = re.match(r'([0-9.-]*).*', loc_data[LONG]).groups()[0]

                        longitude = float(filtered_lon)
                        ret_val = '%s,%s' % (latitude, longitude)
        except:
            # Silently fail, since many modems appear in capable of providing this data
            pass

        return ret_val

    @staticmethod
    def find_pumps_for_disconnect_alarm(time_now):
        """Find pumps that have been recently disconnected and need to have an alarm sent"""
        message = ''
        try:
            disconnect_time_min = time_now - timedelta(hours=1)
            disconnect_time_max = time_now - timedelta(hours=8)

            pumps = Pump.objects.filter(connection=False, is_active=True, last_seen__gt=disconnect_time_max, last_seen__lt=disconnect_time_min, disconnection_noticed=False)

            for pump in pumps:
                message += 'Noticed disconnection of pump %s' % (pump.unique_id,)
                log_message = 'Noticed long-term disconnection'
                log_entry = Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                                               event_type=Log.EVENT_DEBUG,
                                               action=Log.ACTION_UPDATE,
                                               success=Log.STATUS_SUCCESS,
                                               message=log_message,
                                               target_type=Log.TARGET_PUMP,
                                               target_id=pump.unique_id,
                                               )
                log_entry.save()

                # Create a pseudo-alarm work item about the disconnection
                AlarmWork.objects.create(pump=pump, alarm_id=Pump.PSEUDO_ALARM_DISCONNECTION, created_at=time_now)

                pump.disconnection_noticed = True
                pump.save()
        except Exception as e:
            message += 'Error processing disconnect notifications'
            log_message = 'Error processing disconnect notifications: %s' % (repr(e),)
            print(log_message)
            log_entry = Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                                           event_type=Log.EVENT_DEBUG,
                                           action=Log.ACTION_UPDATE,
                                           success=Log.STATUS_FAIL,
                                           message=log_message,
                                           target_type=Log.TARGET_UNKNOWN,
                                           )
            log_entry.save()

        if len(message) < 1:
            message += 'DiscNotifOK'
        return message

    @staticmethod
    def auth_req(pump_id):
        '''
        Get or create a pump object for the given ID. Intended for use when the pump makes its credential request.

        This is the only place pumps should be created
        '''
        pump_obj = None

        if len(pump_id) >= Pump.PUMP_ID_MIN_LENGTH and len(pump_id) <= Pump.PUMP_ID_MAX_LENGTH:
            pump_obj = Pump.objects.filter(unique_id=pump_id).first()

            # It appears to be a new pump, so create it
            if not pump_obj:
                pump_obj = Pump.objects.create(unique_id=pump_id)
                log_message = 'Pump created'
                Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                                   event_type=Log.EVENT_COMMAND,
                                   target_type=Log.TARGET_PUMP,
                                   action=Log.ACTION_CREATE,
                                   success=1,
                                   target_id=sanitize(str(pump_id)),
                                   message=sanitize(log_message),
                                   )

        ''''
        # Bring a "deleted" pump back to life

        NOTE: If the pump controller is not power-cycled after being deleted/deactivated, it
              will NOT hit this endpoint, since the credentials will be cached. In that case,
              the "reanimation" will occur in the mqtt/db worker, so whatever is coded here
              will not be run.
        '''
        if pump_obj and not pump_obj.is_active:
            # Start with a clean slate to ensure everything is in sync
            pump_obj.deactivate()

            pump_obj.is_active = True

            # Force generation of new credentials
            pump_obj.mqtt_username = ''
            pump_obj.mqtt_pw_clear = ''
            pump_obj.save()

            log_message = 'Pump reactivated'
            Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                               event_type=Log.EVENT_COMMAND,
                               target_type=Log.TARGET_PUMP,
                               action=Log.ACTION_UPDATE,
                               success=1,
                               attribute='is_active',
                               new_value='1',
                               target_id=sanitize(str(pump_id)),
                               message=sanitize(log_message),
                               )

        # Generate new credentials if they haven't been set
        if pump_obj:
            if pump_obj.mqtt_auth is None:
                pump_obj.update_mosquitto_creds()

        return pump_obj


class Log(models.Model):
    id = models.AutoField(primary_key=True)
    '''
    Constants
    '''

    ORIGIN_UNKNOWN = 0
    ORIGIN_PUMP = 1
    ORIGIN_SERVER = 2
    ORIGIN_WEB = 3
    ORIGIN_MQTT = 4
    ORIGINS = {ORIGIN_UNKNOWN: 'Unknown',
               ORIGIN_PUMP: 'Pump',
               ORIGIN_SERVER: 'Server',
               ORIGIN_WEB: 'Web',
               ORIGIN_MQTT: 'MQTT',
               }

    EVENT_UNKNOWN = 0
    EVENT_LOGIN = 1
    EVENT_LOGOUT = 2
    EVENT_COMMAND = 3
    EVENT_DEBUG = 4
    EVENT_STATUS = 5
    EVENT_EMAIL = 6
    EVENT_PAYMENTS = 7
    EVENT_BACKUP = 8
    EVENT_API_ERROR = 9

    EVENTS = {EVENT_UNKNOWN: '?',
              EVENT_LOGIN: 'Login',
              EVENT_LOGOUT: 'Logout',
              EVENT_COMMAND: 'Cmd',
              EVENT_DEBUG: 'Debug',
              EVENT_STATUS: 'Status',
              EVENT_EMAIL: 'Email',
              EVENT_PAYMENTS: 'Payments',
              EVENT_BACKUP: 'Backup',
              EVENT_API_ERROR: 'API error',
              }

    TARGET_UNKNOWN = 0
    TARGET_PUMP = 1
    TARGET_USER = 2
    TARGET_CUSTOMER = 3
    TARGET_DEBUG = 4
    TARGET_NOTIF = 5
    TARGET_ALARM_ALERT = 6
    TARGET_PAYMENT = 7
    TARGET_INVITATION = 8

    TARGETS = {TARGET_UNKNOWN: 'Unknown',
               TARGET_PUMP: 'Pump',
               TARGET_USER: 'User',
               TARGET_CUSTOMER: 'Group',
               TARGET_DEBUG: 'Debug',
               TARGET_NOTIF: 'Notification',
               TARGET_ALARM_ALERT: 'Alarm alert',
               TARGET_PAYMENT: 'Payment',
               TARGET_INVITATION: 'Invitation',
               }

    ACTION_UNKNOWN = 0
    ACTION_CREATE = 1
    ACTION_READ = 2
    ACTION_UPDATE = 3
    ACTION_DELETE = 4
    ACTION_ASSOCIATE = 5
    ACTION_NA = 6
    ACTION_SEND = 7
    ACTION_LOGIN = 8
    ACTION_WEBHOOK = 9

    ACTIONS = {ACTION_UNKNOWN: 'Unknown',
               ACTION_CREATE: 'Create',
               ACTION_READ: 'Read',
               ACTION_UPDATE: 'Update',
               ACTION_DELETE: 'Delete',
               ACTION_ASSOCIATE: 'Associate',
               ACTION_NA: 'n/a',
               ACTION_SEND: 'Send',
               ACTION_LOGIN: 'Login',
               ACTION_WEBHOOK: 'Webhook',
               }

    STATUS_FAIL = 0
    STATUS_SUCCESS = 1
    STATUS_UNKNOWN = 2

    STATUS = {STATUS_FAIL: 'FAIL',
              STATUS_SUCCESS: 'Success',
              STATUS_UNKNOWN: 'Unknown',
              }

    VERSION_UNKNOWN = 0
    VERSION_1 = 1
    VERSION_2 = 2

    DEFAULT_EVENT_LOG_PAGE_SIZE = 100
    EVENT_LOG_MAX_PAGE_SIZE = 500
    EVENT_LOG_MIN_PAGE_SIZE = 10

    '''
    Fields
    '''

    origin_type = models.IntegerField(default=ORIGIN_UNKNOWN)
    origin_id = models.CharField(max_length=100, default='')
    timestamp = models.DateTimeField(auto_now_add=True)
    event_type = models.IntegerField(default=EVENT_UNKNOWN)
    message = models.TextField(default='')

    # The user taking the action
    user_actor = models.ForeignKey(User, null=True, default=None, on_delete=models.SET_DEFAULT)

    origin_ip = models.CharField(max_length=100, default='')
    target_type = models.IntegerField(default=TARGET_UNKNOWN)
    target_id = models.CharField(max_length=100, default='')

    attribute = models.CharField(max_length=100, default='')
    old_value = models.CharField(max_length=100, default='')
    new_value = models.CharField(max_length=100, default='')
    success = models.IntegerField(default=STATUS_FAIL)  # Intentionally not a boolean, since success is not always black and white
    action = models.IntegerField(default=ACTION_UNKNOWN)

    # Version the log entries
    entry_format = models.IntegerField(default=VERSION_2)

    '''
    Methods
    '''

    def get_csv_line(self):
        '''Represent the current object as a CSV line'''

        self.convert_old_version_to_new()

        origin_type_txt = self.ORIGINS.get(self.origin_type, self.origin_type)
        event_type_txt = self.EVENTS.get(self.event_type, self.event_type)
        target_type_txt = self.TARGETS.get(self.target_type, self.target_type)
        action_txt = self.ACTIONS.get(self.action, self.action)
        success_txt = self.STATUS.get(self.success, self.success)

        if len(self.target_id) > 0:
            self.target_id = '#' + self.target_id

        self.message = re.sub(',', ';', self.message)

        username = ''
        if self.user_actor:
            username = self.user_actor.userprofile.get_username()

        values_to_encode = [self.pk,
                            self.timestamp,
                            origin_type_txt,
                            self.origin_ip,
                            event_type_txt,
                            username,
                            target_type_txt,
                            self.target_id,
                            action_txt,
                            self.attribute,
                            self.new_value,
                            success_txt,
                            self.message
                            ]
        output = ','.join([str(v) for v in values_to_encode])

        ''' DO NOT SAVE the object; our changes were meant to be temporary '''

        return output

    def convert_old_version_to_new(self):
        '''
        Convert old log entries to the new format, but don't save the changes
        '''
        if self.entry_format == self.VERSION_1 or self.entry_format == self.VERSION_UNKNOWN:
            # Coerce the old-style log entries to fit the new-style format
            self.target_type = self.TARGET_UNKNOWN
            if self.origin_type == self.ORIGIN_WEB:
                self.origin_ip = self.origin_id
                self.origin_id = ''
            if self.origin_type == self.ORIGIN_WEB and self.event_type == self.EVENT_COMMAND:
                message_content = self.message.split('/')
                if len(message_content) == 3:  # Message appears to be in a standard username/target_id/data format
                    username = re.sub('\s', '', message_content[0])
                    self.target_id = re.sub('\s', '', message_content[1])
                    self.message = message_content[2]

                    user = User.objects.filter(username=username).first()
                    if user:
                        self.user_actor = user
            if self.origin_type == self.ORIGIN_SERVER and self.event_type == self.EVENT_DEBUG:
                self.target_id = self.origin_id
                self.target_type = self.TARGET_PUMP
                self.origin_type = self.ORIGIN_MQTT

            self.success = self.STATUS_UNKNOWN

        elif self.entry_format == self.VERSION_2:
            pass

        else:
            raise Exception('Invalid entry version')

    def get_csv_header(self):
        '''Get the header'''

        column_names = ['Event number',
                        'Timestamp',
                        'Origin type',
                        'Origin IP',
                        'Event type',
                        'User',
                        'Target type',
                        'Target ID',
                        'Action',
                        'Attribute',
                        'New value',
                        'Success',
                        'Message',
                        ]
        return ','.join(column_names)

    @staticmethod
    def fetch_filtered(request, paginate=True):
        '''Get a filtered log queryset'''
        page_size = Log.DEFAULT_EVENT_LOG_PAGE_SIZE
        try:
            if 'page_size' in request.GET:
                page_size = int(request.GET['page_size'])
        except:
            page_size = Log.DEFAULT_EVENT_LOG_PAGE_SIZE
        if page_size > Log.EVENT_LOG_MAX_PAGE_SIZE:
            page_size = Log.EVENT_LOG_MAX_PAGE_SIZE
        if page_size < Log.EVENT_LOG_MIN_PAGE_SIZE:
            page_size = Log.EVENT_LOG_MIN_PAGE_SIZE

        page = 0
        try:
            if 'page' in request.GET:
                page = int(request.GET['page'])
        except:
            page = 0
        if page < 0:
            page = 0

        id_filter = None
        try:
            if 'id_filter' in request.GET:
                id_filter = re.sub(r'[^a-zA-Z0-9*]', '', request.GET['id_filter'])
                id_filter = re.sub(r'\*', '.*', id_filter)
                id_filter = '^' + id_filter + '$'
        except:
            id_filter = None

        target_type = None
        try:
            if 'target_type' in request.GET:
                target_type = int(request.GET['target_type'])
        except:
            pass

        message_contains = None
        try:
            if 'message_contains' in request.GET:
                message_contains = request.GET['message_contains']
        except:
            pass

        message_exclude = None
        try:
            if 'message_exclude' in request.GET:
                message_exclude = request.GET['message_exclude']
        except:
            pass

        exclude_common = False
        if 'exclude_common' in request.GET:
            exclude_common = True

        event_query_base = Log.objects

        if id_filter:
            event_query_base = event_query_base.filter(target_id__regex=id_filter)

        if target_type:
            event_query_base = event_query_base.filter(target_type=target_type)

        if message_contains:
            event_query_base = event_query_base.filter(message__contains=message_contains)

        if message_exclude:
            event_query_base = event_query_base.exclude(message__contains=message_exclude)

        if exclude_common:
            event_query_base = event_query_base.exclude(message__contains='onnected')
            event_query_base = event_query_base.exclude(message__contains='Power Cycle')
            event_query_base = event_query_base.exclude(message__contains='long-term')
            event_query_base = event_query_base.exclude(target_type=Log.TARGET_ALARM_ALERT)

        if paginate:
            all_events_raw = event_query_base.order_by('-pk')[page_size * page:page_size * (page + 1)]
        else:
            all_events_raw = event_query_base

        return all_events_raw


class AlarmPreference(models.Model):
    '''Allows users to change preferences about things like alarm notifications'''
    id = models.AutoField(primary_key=True)
    # List of alarms that can be configured
    ALARMS_CONFIGURABLE = [Pump.PSEUDO_ALARM_DISCONNECTION,
                           ]
    QUALIFIER = 'alarm_pref_id_'

    user = models.ForeignKey(User, on_delete=models.CASCADE)
    alarm_id = models.IntegerField(default=Pump.ALARM_ID_DEFAULT)
    send_email = models.BooleanField(default=True)

    @staticmethod
    def get_alarm_name(alarm_id):
        result = None
        try:
            if alarm_id < 0:
                result = Pump.PSEUDO_ALARMS[-1 * alarm_id]
            else:
                result = AlarmList[alarm_id]
        except:
            # Could not find a name for the given alarm ID
            result = None

        return result


class AlarmCustomization(models.Model):
    '''Customization of alarm metadata. Not for setting alarm thresholds themselves'''
    id = models.AutoField(primary_key=True)
    CUSTOM_NAME_REGEX = r'[^-a-zA-Z0-9\s.,!?@#$%)(+|_/&]'

    pump = models.ForeignKey(Pump, null=True, on_delete=models.SET_NULL)
    alarm_id = models.IntegerField(default=Pump.ALARM_ID_DEFAULT)
    custom_alarm_name = models.CharField(max_length=100, default='')

class PointCustomization(models.Model):
    '''Customization of injection point metadata. Not for setting alarm thresholds themselves'''
    id = models.AutoField(primary_key=True)
    CUSTOM_NAME_REGEX = r'[^-a-zA-Z0-9\s.,!?@#$%)(+|_/&]'

    pump = models.ForeignKey(Pump, null=True, on_delete=models.SET_NULL)
    point_id = models.IntegerField(default=Pump.ALARM_ID_DEFAULT)
    custom_point_name = models.CharField(max_length=100, default='')

class AlarmWork(models.Model):
    '''Work queue for alarm notifications'''
    id = models.AutoField(primary_key=True)
    SUBJECT_MAX_LENGTH = 128
    SUBJECT_REGEX = r'[^-a-zA-Z0-9\s.,!?@#$%)(+|_/&]'
    OTHER_REGEX = r'[\'"<;:%&]'

    RECENT_TIME_THRESHOLD = timedelta(hours=24)

    pump = models.ForeignKey(Pump, null=True, on_delete=models.SET_NULL)
    alarm_id = models.IntegerField(default=Pump.ALARM_ID_DEFAULT)
    done = models.BooleanField(default=False)

    created_at = models.DateTimeField(null=True, auto_now_add=False, auto_now=False)
    actually_sent_at = models.DateTimeField(null=True, auto_now_add=False, auto_now=False)

    def generate_alarm_email(self, user_profile):
        '''Generate an email to the given user about this alarm'''
        pump_name = self.pump.unique_id
        if len(self.pump.pretty_name) > 0:
            pump_name = self.pump.pretty_name

        # Use the standard alarm name unless a custom one exists for this pump
        alarm_name = self.pump.get_alarm_name(self.alarm_id)

        pump_name = re.sub(self.SUBJECT_REGEX, '', pump_name)
        group_name = re.sub(self.OTHER_REGEX, '', self.pump.customer.organization_name)
        alarm_name = re.sub(self.OTHER_REGEX, '', alarm_name)

        domain = SITE_DOMAIN
        email_origin = settings.EMAIL_ORIGIN_ADDRESS
        secure = 's' if USE_HTTPS else ''

        # URL for pump details
        partial_pump_details_url = reverse('pump_details', kwargs={'pump_id': self.pump.unique_id})
        pump_details_url = 'http%s://%s%s' % (secure, domain, partial_pump_details_url)

        # URL where the user can configure notifications
        partial_user_settings_url = reverse('user_settings_default', args=[])
        user_settings_url = 'http%s://%s%s' % (secure, domain, partial_user_settings_url)

        # Generate an email appropriate for this notification, and insert
        mail_template_text = get_template('emails/alarm_alert.txt')
        mail_template_html = get_template('emails/alarm_alert.html')
        mail_context = Context({'first_name': user_profile.user.first_name,
                                'group_name': group_name,
                                'pump_name': pump_name,
                                'alarm_name': alarm_name,
                                'user_settings_url': user_settings_url,
                                'pump_details_url': pump_details_url,
                                })

        mail_text = mail_template_text.render(mail_context)
        mail_html = mail_template_html.render(mail_context)

        subject = 'Alarm on your Graco Chemical Injection Pump - %s' % (pump_name,)
        from_email = 'Graco <%s>' % (email_origin,)
        to_email = user_profile.user.email
        if not send_mail(subject, mail_text, from_email, [to_email], html_message=mail_html):
            message = 'Error sending alarm alert for pump %s to user %u' % (self.pump.unique_id, user_profile.user.id)
            Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=None, message=message, success=Log.STATUS_FAIL, target_type=Log.TARGET_ALARM_ALERT, target_id=self.id)

    def sent_idential_alarm_recently(self):
        '''
        Check if another alarm work item matching this one was sent "recently" so that we don't repeatedly
        send out emails that oscillate between active and inactive.
        '''

        now = datetime.utcnow().replace(tzinfo=pytz.utc)
        threshold = now - self.RECENT_TIME_THRESHOLD
        sent_identical = False

        '''
        Check if the once-in-a-day limit should be overridden based on the alarm
        '''
        override_limit = False
        if self.alarm_id in AlarmsManuallyCleared:
            override_limit = True

        '''
        Find instances of the same alarm being sent recently
        '''
        identical_obj_count = AlarmWork.objects.filter(pump=self.pump, alarm_id=self.alarm_id, actually_sent_at__gt=threshold, done=True).exclude(id=self.id).count()

        if (identical_obj_count > 0) and not override_limit:
            sent_identical = True

        return sent_identical

    def service(self, time_now):
        '''Send emails for this alarm'''
        sent = False

        # There could be a problem here if these requests start taking more than one minute to complete, which
        # would cause subsequent cron jobs to also attempt to send these messages, resulting in duplicate
        # messages. Punting on that for now.
        try:
            # Don't send alerts to the None group
            if self.pump.customer.id == get_none_customer():
                message = 'Pump %s in None group' % (self.pump.unique_id,)
                Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=None, message=message, success=Log.STATUS_FAIL, target_type=Log.TARGET_ALARM_ALERT, target_id=self.id)
            # Make sure it's a valid-looking alarm
            elif self.alarm_id == Pump.ALARM_ID_DEFAULT:
                message = 'Invalid alarm ID'
                Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=None, message=message, success=Log.STATUS_FAIL, target_type=Log.TARGET_ALARM_ALERT, target_id=self.id)
            # Make sure we haven't sent emails about this alarm for this pump in the recent past
            elif self.alarm_id >= 0 and self.sent_idential_alarm_recently():
                message = 'Already sent this alarm recently'
                Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=None, message=message, success=Log.STATUS_FAIL, target_type=Log.TARGET_ALARM_ALERT, target_id=self.id)
            # Make sure the alarm is still active (assuming it isn't a pseudo-alarm
            elif self.alarm_id >= 0 and not (self.pump.alarms_status & (1 << self.alarm_id)):
                message = 'Pump %s no longer has alarm active' % (self.pump.unique_id,)
                Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=None, message=message, success=Log.STATUS_FAIL, target_type=Log.TARGET_ALARM_ALERT, target_id=self.id)
            else:

                # Find all users who are members of the pump's group and have alarm notifications enabled
                user_profiles = self.pump.customer.userprofile_set.filter(enable_alarm_alerts=1, email_confirmed=True, user__is_active=True).all()

                for up in user_profiles:
                    '''Don't send emails to people who didn't want this alarm'''
                    if not up.user.alarmpreference_set.filter(alarm_id=self.alarm_id, send_email=False).exists():
                        self.generate_alarm_email(up)

                self.actually_sent_at = time_now
                sent = True

        finally:
            # When all matching users have been notified, mark the work as done, event if there was an exception so that everybody else
            # in the group doesn't get a billion emails
            self.done = True
            self.save()

        return sent

    @staticmethod
    def service_all(time_now):
        '''Service all waiting alarm work'''
        out_message = ''  # For concatenation, NOT for the Log object

        alarm_work_list = AlarmWork.objects.filter(done=False)

        for work in alarm_work_list:
            try:
                sent = work.service(time_now)
                message = 'Pump %s Alarm %d' % (work.pump.unique_id, work.alarm_id)
                success = Log.STATUS_SUCCESS if sent else Log.STATUS_FAIL
                Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=None, message=message, success=success, target_type=Log.TARGET_ALARM_ALERT, target_id=work.pk)
            except Exception as e:
                print('Alarm alert exception: %s' % (repr(e),))
                out_message += 'Error servicing alarm alert %d\n' % (work.pk,)
                Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                                   origin_id='cron',
                                   action=Log.ACTION_SEND,
                                   event_type=Log.EVENT_EMAIL,
                                   user_actor=None,
                                   message='Pump %s Alarm %d Send Exception: %s' % (work.pump.unique_id, work.alarm_id, repr(e),),
                                   success=Log.STATUS_FAIL,
                                   target_type=Log.TARGET_ALARM_ALERT,
                                   target_id=work.pk)

        if len(out_message) == 0:
            out_message = 'AlarmAlertOK'

        return out_message


class Notification(models.Model):
    """Notifications, both periodic and spot, for sending to users when selected pump events occur"""
    id = models.AutoField(primary_key=True)
    SUBJECT_TYPES = ['Unknown',
                     'Pump Service',
                     'Motor Service',
                     'Battery Service',
                     'Chemical Tank Refill',
                     'Multiwell Service',
                     'Custom',
                     ]

    SUBJECT_OTHER_VALUE = 'Custom'

    SUBJECT_MAX_LENGTH = 128
    SUBJECT_REGEX = r'[^-a-zA-Z0-9\s.,!?@#$%)(+|_/&]'

    CRITERIA_TYPES = ['Unknown',
                      'Periodic',
                      'Alarm']

    PERIOD_MIN = -1
    PERIOD_CUSTOM_MIN = 1
    PERIOD_MAX = 60

    PERIOD_OTHER_VALUE = 'Custom'

    IMMEDIATE = 'Immediate'

    user = models.ForeignKey(User, null=True, on_delete=models.SET_NULL)
    pump = models.ForeignKey(Pump, null=True, on_delete=models.SET_NULL)
    period = models.IntegerField(default=0)  # Months
    time_next_due = models.DateTimeField(null=True, auto_now_add=False, auto_now=False)
    time_last_sent = models.DateTimeField(null=True, auto_now_add=False, auto_now=False)

    criteria = models.IntegerField(default=0)

    subject_text = models.CharField(null=True, default=None, max_length=128)

    sent_since_last_set = models.BooleanField(default=False)  # Has the notification been sent since the notifying event last occurred?
    is_enabled = models.BooleanField(default=True)

    def service(self, time_basis):
        """Service this notification using the given timestamp as a basis"""
        # Defer notification if the user doesn't want any sent
        if not self.user.userprofile.enable_notifications:
            message = 'User has notifications disabled on notification %d' % (self.id,)
            raise Exception(message)

        # Mailgun will complain if the email address is bad. Do a quick sanity check on it.
        if not re.match(UserProfile.EMAIL_REGEX, self.user.email):
            message = 'Bad email address on notification %d' % (self.id,)
            raise Exception(message)

        if not self.user.userprofile.email_confirmed:
            message = 'User\'s email address is unconfirmed on notification %d' % (self.id,)
            raise Exception(message)

        pump_name = self.pump.unique_id
        if len(self.pump.pretty_name) > 0:
            pump_name = self.pump.pretty_name

        notification_subject = re.sub(self.SUBJECT_REGEX, '', self.subject_text)

        domain = SITE_DOMAIN
        email_origin_address = settings.EMAIL_ORIGIN_ADDRESS
        secure = 's' if USE_HTTPS else ''

        # URL where the user can configure notifications
        absolute_url = reverse('user_settings_default', args=[])
        notification_setup_url = 'http%s://%s%s' % (secure, domain, absolute_url)

        # Generate an email appropriate for this notification, and insert
        mail_template_text = get_template('emails/generic_notification.txt')
        mail_template_html = get_template('emails/generic_notification.html')
        mail_context = Context({'username': self.user.userprofile.get_username(),
                                'first_name': self.user.first_name,
                                'pump_name': pump_name,
                                'notification_subject': notification_subject,
                                'notification_setup_url': notification_setup_url,
                                'period': self.period, })

        mail_text = mail_template_text.render(mail_context)
        mail_html = mail_template_html.render(mail_context)

        subject = 'Maintenance Reminder About Your Graco Chemical Injection Pump - %s' % (pump_name,)
        from_email = 'Graco <%s>' % (email_origin_address,)
        to_email = self.user.email
        send_mail(subject, mail_text, from_email, [to_email], html_message=mail_html)

        # Change the period if this was a test email
        if self.period < 1:
            self.period = 1

        # Mark this one as sent and schedule the next notification
        self.time_last_sent = datetime.utcnow().replace(tzinfo=pytz.utc)
        self.time_next_due = time_basis + relativedelta(months=self.period)
        self.save()

        return True

    @staticmethod
    def creation_helper(user, pump, period, subject_text, criteria):
        '''
        Helper for the creation of a notification. Does range validation on the parameters and calculates the first due date.

        Returns the new Notification object if successful, or None if not.'''

        # Allow immediate notifications for testing
        if period == Notification.IMMEDIATE:
            period = -1
        else:
            period = int(period)

        if criteria <= 0 or criteria >= len(Notification.CRITERIA_TYPES):
            raise ValueError('Invalid criteria')

        subject_text_filtered = re.sub(Notification.SUBJECT_REGEX, '', subject_text)
        if len(subject_text_filtered) < 1:
            raise ValueError('Invalid subject')

        if period < Notification.PERIOD_MIN or period > Notification.PERIOD_MAX:
            raise ValueError('Invalid period')

        notif_obj = Notification.objects.create(user=user,
                                                pump=pump,
                                                period=period,
                                                subject_text=subject_text_filtered,
                                                criteria=criteria)

        if notif_obj.period != 0:
            notif_obj.time_next_due = datetime.utcnow().replace(tzinfo=pytz.utc) + relativedelta(months=notif_obj.period)
            notif_obj.is_enabled = True

        notif_obj.save()

        return notif_obj

    @staticmethod
    def service_all(time_basis):
        """Check all notifications for ones that need servicing based on the given timestamp"""
        message = ''

        # Fetch all Notifications that are enabled, have a time_next_due in the past, have a time_next_due after their time_last_sent
        notifications = Notification.objects.filter(is_enabled=True, time_next_due__lt=time_basis)

        for notif_obj in notifications:
            try:
                result = Log.STATUS_SUCCESS if notif_obj.service(time_basis) else Log.STATUS_FAIL
                Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=notif_obj.user, message='', success=result, target_type=Log.TARGET_NOTIF, target_id=notif_obj.pk)
            except Exception as e:
                print('Notification exception: %s' % (repr(e),))
                message += 'Error servicing notification %d\n' % (notif_obj.pk,)
                Log.objects.create(origin_type=Log.ORIGIN_SERVER, origin_id='cron', action=Log.ACTION_SEND, event_type=Log.EVENT_EMAIL, user_actor=notif_obj.user, message='Send Exception: %s' % (repr(e),), success=Log.STATUS_FAIL, target_type=Log.TARGET_NOTIF, target_id=notif_obj.pk)

        if len(message) == 0:
            message = 'NotificationsOK'

        return message
    
