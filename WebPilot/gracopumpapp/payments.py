'''
Payment support functions
'''

from datetime import datetime
import re

from dateutil.relativedelta import relativedelta
from gracopumpapp.models import Plan, Subscription, PaymentAccount, Log, \
    sanitize
import recurly
from recurly.errors import NotFoundError


class Payments(object):

    @staticmethod
    def process_webhook(notif):
        notif_recognized = True

        if notif['type'] == 'new_account_notification':
            Payments.check_account(notif)
        elif notif['type'] == 'canceled_account_notification':
            Payments.check_account(notif)
        elif notif['type'] == 'reactivated_account_notification':
            Payments.check_account(notif)
        elif notif['type'] == 'billing_info_updated_notification':
            Payments.check_account(notif)
        elif notif['type'] == 'new_invoice_notification':
            Payments.check_invoice(notif)
        elif notif['type'] == 'processing_invoice_notification':
            Payments.check_invoice(notif)
        elif notif['type'] == 'closed_invoice_notification':
            Payments.check_invoice(notif)
        elif notif['type'] == 'past_due_invoice_notification':
            Payments.check_invoice(notif)
        elif notif['type'] == 'new_subscription_notification':
            Payments.check_subscription(notif)
        elif notif['type'] == 'updated_subscription_notification':
            Payments.check_subscription(notif)
        elif notif['type'] == 'canceled_subscription_notification':
            Payments.check_subscription(notif)
        elif notif['type'] == 'expired_subscription_notification':
            Payments.check_subscription(notif)
        elif notif['type'] == 'renewed_subscription_notification':
            Payments.check_subscription(notif)
        elif notif['type'] == 'scheduled_payment_notification':
            Payments.check_payment(notif)
        elif notif['type'] == 'processing_payment_notification':
            Payments.check_payment(notif)
        elif notif['type'] == 'successful_payment_notification':
            Payments.check_payment(notif)
        elif notif['type'] == 'failed_payment_notification':
            Payments.check_payment(notif)
        elif notif['type'] == 'successful_refund_notification':
            Payments.check_payment(notif)
        elif notif['type'] == 'void_payment_notification':
            Payments.check_payment(notif)
        else:
            notif_recognized = False

        return notif_recognized

    @staticmethod
    def check_account(notif):
        status = Log.STATUS_SUCCESS

        # Verify and update
        try:
            # Extract, but don't trust
            acct_num = notif['account'].account_code

            account = recurly.Account.get(acct_num)
            local_account = PaymentAccount.objects.get(account_code=account.account_code)

            account_balance = account.account_balance()
            local_account.balance = account_balance.balance_in_cents[Plan.CURRENCY_CODE]
            local_account.save()
        except:
            status = Log.STATUS_FAIL

        message = 'Recurly account status'
        Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                           event_type=Log.EVENT_PAYMENTS,
                           action=Log.ACTION_WEBHOOK,
                           success=status,
                           message=message,
                           target_type=Log.TARGET_PAYMENT,
                           attribute=sanitize(notif['type']),
                           )

    @staticmethod
    def check_invoice(notif):
        '''
        Update account balanaces, which usually occur with invoice webhooks instead of
        with account webhooks
        '''
        status = Log.STATUS_SUCCESS

        # Verify
        try:
            invoice_num = notif['invoice'].invoice_number
            invoice = recurly.Invoice.get(invoice_num)

            account = invoice.account()
            local_account = PaymentAccount.objects.get(account_code=account.account_code)

            account_balance = account.account_balance()
            local_account.balance = account_balance.balance_in_cents[Plan.CURRENCY_CODE]
            local_account.save()
        except:
            status = Log.STATUS_FAIL

        message = 'Recurly invoice'
        Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                           event_type=Log.EVENT_PAYMENTS,
                           action=Log.ACTION_WEBHOOK,
                           success=status,
                           message=message,
                           target_type=Log.TARGET_PAYMENT,
                           attribute=sanitize(notif['type']),
                           )

    @staticmethod
    def check_subscription(notif):
        # Extract, but don't trust
        acct_num = notif['account'].account_code
        subsc_num = notif['subscription'].uuid
        handled = False

        '''
        Query API to verify account and subscription info -- never trust without verifying.

        These will raise exceptions if the lookups fail 
        '''
        account = recurly.Account.get(acct_num)
        subscription = recurly.Subscription.get(subsc_num)

        if notif['type'] == 'canceled_subscription_notification':
            try:
                local_subscription = Subscription.objects.get(recurly_uuid=subscription.uuid)
                if subscription.state == 'canceled':
                    local_subscription.status = Subscription.STATUS_CANCELED
                    local_subscription.save()
                    handled = True
            except Subscription.DoesNotExist:
                # If we get a notice about a canceled subscription we don't recognize, pretend all is well
                handled = True
        elif notif['type'] == 'expired_subscription_notification':
            try:
                local_subscription = Subscription.objects.get(recurly_uuid=subscription.uuid)
                if subscription.state == 'expired':
                    local_subscription.status = Subscription.STATUS_EXPIRED
                    local_subscription.save()
                    handled = True
            except Subscription.DoesNotExist:
                # If we get a notice about a canceled subscription we don't recognize, pretend all is well
                handled = True
        elif notif['type'] == 'updated_subscription_notification' or notif['type'] == 'renewed_subscription_notification':
            Payments.pick_up_subscription_changes(subscription)
            handled = True

        if not handled:
            message = 'Unhandled Recurly subscription hook'
            Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                               event_type=Log.EVENT_PAYMENTS,
                               action=Log.ACTION_WEBHOOK,
                               success=Log.STATUS_FAIL,
                               message=message,
                               target_type=Log.TARGET_PAYMENT,
                               attribute=sanitize(notif['type']),
                               )

    @staticmethod
    def check_payment(notif):
        # Extract, but don't trust
        acct_num = notif['account'].account_code
        subsc_num = notif['transaction'].subscription_id
        transaction_num = notif['transaction'].id

        # Verify
        account = recurly.Account.get(acct_num)
        subscription = recurly.Subscription.get(subsc_num)
        transaction = recurly.Transaction.get(transaction_num)

        # TODO: Actually check the values returned by the verified calls. Since we're not doing anything except logging the event right now, doesn't matter at the moment.

        message = 'Unhandled Recurly payment hook'
        Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                           event_type=Log.EVENT_PAYMENTS,
                           action=Log.ACTION_WEBHOOK,
                           success=Log.STATUS_FAIL,
                           message=message,
                           target_type=Log.TARGET_PAYMENT,
                           attribute=sanitize(notif['type']),
                           )

        # We don't actually track this locally, so don't do anything
        pass

    @staticmethod
    def sync_plans():
        '''Synchronize the local plans with the remote plans'''
        codes_found = []

        plans = recurly.Plan.all()
        for plan_remote in plans:
            codes_found.append(plan_remote.plan_code)

            plan_local = Plan.objects.filter(code=plan_remote.plan_code).first()

            if not plan_local:
                plan_local = Plan.objects.create(code=plan_remote.plan_code)

            plan_local.name = plan_remote.name
            plan_local.price_cents = plan_remote.unit_amount_in_cents[Plan.CURRENCY_CODE]
            plan_local.setup_cents = plan_remote.setup_fee_in_cents[Plan.CURRENCY_CODE]
            plan_local.plan_interval = plan_remote.plan_interval_length
            plan_local.plan_units = plan_remote.plan_interval_unit
            plan_local.trial_interval = plan_remote.trial_interval_length
            plan_local.trial_units = plan_remote.trial_interval_unit
            plan_local.is_active = True

            # Infer low and high limits
            decoded = re.match(r'^(?P<low_count>\d+)_(?P<high_count>\d+)_pumps$', plan_local.code)
            if decoded:
                plan_local.min_pumps = int(decoded.group('low_count'))
                plan_local.max_pumps = int(decoded.group('high_count'))
            else:
                plan_local.min_pumps = 0
                plan_local.max_pumps = 0

            # Determine if the plan can be selected by users
            user_selectable_search = re.match(r'^.+_selectable$', plan_local.code)
            if user_selectable_search:
                plan_local.user_selectable = True

            plan_local.save()

        # Any plans that no longer exist on Recurly should be marked unavailable locally
        orphaned_plans = Plan.objects.exclude(code__in=codes_found)
        for plan in orphaned_plans:
            plan.is_available = False
            plan.save()

    @staticmethod
    def update_subscription(token_id, plan, customer, user):
        '''Change an existing subscription'''
        if not customer.has_valid_subscription():
            raise ValueError('Can update only currently valid subscriptions')

        local_subscription = customer.get_current_subscription()

        local_account = local_subscription.account
        recurly_account = recurly.Account.get(local_account.account_code)

        # Update the recurly account with the current user info
        recurly_account.email = user.email
        recurly_account.first_name = user.first_name
        recurly_account.last_name = user.last_name
        recurly_account.save()

        # Allow the billing information to be updated (e.g., user switches to new card
        if token_id != str(PaymentAccount.USE_EXISTING_CARD):
            recurly_billing_info = recurly.BillingInfo(token_id=token_id)
            recurly_account.update_billing_info(recurly_billing_info)
            recurly_account.save()

        existing_plan = local_subscription.plan
        new_pump_count = customer.active_pump_count()

        # Note the differences between this and the new-subscription plan search
        if plan is None:
            raise ValueError('Plan not allowed')

        recurly_plan = recurly.Plan.get(plan.code)

        recurly_subscription = recurly.Subscription.get(local_subscription.recurly_uuid)
        recurly_subscription.plan_code = plan.code

        recurly_subscription.timeframe = 'now'

        recurly_subscription.quantity = new_pump_count

        recurly_subscription.save()

        # Update local copies of the subscription
        # Don't update the expiration date; that will happen upon renewal
        local_subscription.plan = plan
        local_subscription.quantity = new_pump_count
        local_subscription.save()

        return local_subscription

    @staticmethod
    def new_subscription(token_id, plan, customer, user):
        # Create an account on Recurly, if needed, for this user to match the local one, if needed.
        if customer.has_valid_subscription():
            raise ValueError('Trying to make new subscription when a valid one exists')

        account_code = PaymentAccount.generate_account_code(user, customer)

        try:
            recurly_account = recurly.Account.get(account_code)
        except NotFoundError:
            recurly_account = recurly.Account(account_code=account_code)

        # Populate/update the recurly account with the user info
        recurly_account.email = user.email
        recurly_account.first_name = user.first_name
        recurly_account.last_name = user.last_name
        recurly_account.save()

        if token_id != str(PaymentAccount.USE_EXISTING_CARD):
            recurly_billing_info = recurly.BillingInfo(token_id=token_id)
            recurly_account.update_billing_info(recurly_billing_info)
            recurly_account.save()

        # Create subscription and payment on Recurly
        local_payment_account = PaymentAccount.objects.filter(account_code=recurly_account.account_code).first()
        if not local_payment_account:
            local_payment_account = PaymentAccount.objects.create(account_code=recurly_account.account_code, user=user, customer=customer, is_active=True)

        if plan is None:
            raise ValueError('Invalid plan')

        recurly_plan = recurly.Plan.get(plan.code)

        recurly_subscription = recurly.Subscription()
        recurly_subscription.plan_code = plan.code
        recurly_subscription.currency = Plan.CURRENCY_CODE
        recurly_subscription.account = recurly_account
        recurly_subscription.quantity = customer.active_pump_count()
        recurly_subscription.save()

        # Create local copies of the subscription and payment
        subscription_starts = datetime.utcnow()
        if plan.plan_units != 'months':
            raise ValueError('Invalid plan interval units')
        subscription_expires = subscription_starts + relativedelta(months=plan.plan_interval)

        local_subscription = Subscription.objects.create(account=local_payment_account,
                                                         plan=plan,
                                                         status=Subscription.STATUS_ACTIVE,
                                                         start=subscription_starts,
                                                         expiration=subscription_expires,
                                                         recurly_uuid=recurly_subscription.uuid)
        local_subscription.quantity = recurly_subscription.quantity
        local_subscription.customer = customer
        # Update trial information
        if recurly_subscription.trial_started_at:
            local_subscription.trial_start = recurly_subscription.trial_started_at

        if recurly_subscription.trial_ends_at:
            local_subscription.trial_end = recurly_subscription.trial_ends_at
        local_subscription.save()

        return local_subscription

    @staticmethod
    def recalculate(subscription, token=str(PaymentAccount.USE_EXISTING_CARD)):
        '''
        Recompute the proper plan, change the subscription to that plan (if changed),
        and change the number of pumps to be charged (if needed).
        '''
        if subscription is None:
            return

        pump_count = subscription.customer.active_pump_count()

        message = ''
        action = None

        new_plan = subscription.plan

        if pump_count != subscription.quantity:
            user = subscription.account.user
            if subscription.customer.has_valid_subscription():
                if pump_count > 0:
                    message = 'Changed to plan %s and count %u' % (new_plan.code, pump_count)
                    action = Log.ACTION_UPDATE
                    Payments.update_subscription(token, new_plan, subscription.customer, user)
                else:
                    '''
                    Recurly does not support subscriptions that have a quantity of zero, so we're
                    forced to cancel if we get to that point
                    '''
                    message = 'No active pumps remaining; canceling subscription'
                    action = Log.ACTION_DELETE
                    subscription.terminate()
            else:
                # This branch also used for users who had suspended/deleted all of their pumps and are now coming back
                if pump_count > 0:
                    message = 'Pump count now > 0; Creating new subscription'
                    action = Log.ACTION_CREATE
                    Payments.new_subscription(token, new_plan, subscription.customer, user)

        if action is not None:
            Log.objects.create(message=message,
                               event_type=Log.EVENT_PAYMENTS,
                               origin_type=Log.ORIGIN_WEB,
                               target_type=Log.TARGET_CUSTOMER,
                               target_id=subscription.customer.id,
                               action=action,
                               success=Log.STATUS_UNKNOWN)

    @staticmethod
    def pick_up_subscription_changes(recurly_subscription_obj):
        '''
        Pick up any reported changes in the given Recurly subscription object and change the local
        corresponding Subscription object to match.
        '''
        message = 'Subscription change webhook'
        update_required = False

        local_subscription = Subscription.objects.get(recurly_uuid=recurly_subscription_obj.uuid)

        if recurly_subscription_obj.state == 'active':
            local_subscription.status = Subscription.STATUS_ACTIVE
            update_required = True
        elif recurly_subscription_obj.state == 'live':
            local_subscription.status = Subscription.STATUS_LIVE
            update_required = True
        elif recurly_subscription_obj.state == 'past_due':
            local_subscription.status = Subscription.STATUS_PAST_DUE
            update_required = True
        elif recurly_subscription_obj.state == 'in_trial':
            local_subscription.status = Subscription.STATUS_IN_TRIAL
            update_required = True
        else:
            # The other states (e.g., canceled, expired) will be handled in other webhook calls
            message = 'Got update hook that will be ignored: state %s, uuid %s' % (recurly_subscription_obj.state, recurly_subscription_obj.uuid)

        if update_required:
            local_subscription.plan = Plan.objects.get(code=recurly_subscription_obj.plan_code)

        # Always update the expiration date
        local_subscription.expiration = recurly_subscription_obj.current_period_ends_at

        # Update trial information
        if recurly_subscription_obj.trial_started_at:
            local_subscription.trial_start = recurly_subscription_obj.trial_started_at

        if recurly_subscription_obj.trial_ends_at:
            local_subscription.trial_end = recurly_subscription_obj.trial_ends_at

        local_subscription.save()

        Log.objects.create(origin_type=Log.ORIGIN_SERVER,
                           event_type=Log.EVENT_PAYMENTS,
                           action=Log.ACTION_WEBHOOK,
                           success=Log.STATUS_SUCCESS if update_required else Log.STATUS_FAIL,
                           message='%s %s' % (message, recurly_subscription_obj.state),
                           target_type=Log.TARGET_CUSTOMER,
                           target_id=local_subscription.customer.id
                           )
