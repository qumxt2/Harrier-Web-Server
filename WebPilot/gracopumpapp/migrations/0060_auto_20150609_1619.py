# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.conf import settings
from django.db import models, migrations
import gracopumpapp.models


class Migration(migrations.Migration):

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
        ('gracopumpapp', '0059_pump_activation_key_last_changed'),
    ]

    '''
    Explanation for hard-coded "1" as default customer:

    When merging the payments branch into the master branch around migration 112, an issue arose with this
    older migration. The method get_none_customer couldn't be called because the Customer class couldn't be
    instantiated because the Customer class relies on a field (override_subscription) that doesn't exist
    when this migration (60) is run. It gets created much later during the payment-related migrations.

    Therefore, in order to get this migration and migration 62 to run cleanly on a new server, the default
    customer needed to be defined in a way that doesn't require the Customer class to be instantiated, hence
    the hard-coded Customer ID of 1. On a new server, this should actually be the None customer's ID, but on
    an existing server that won't strictly be true, so this kludge is undone in migration 116.

    The whole "none customer" implementation turned out to be a pain, and I wish I would have simply used a
    a null value (NoneType) to represent no customer affiliation instead of having a special "real" customer
    represent that state. Bummer.
    '''

    operations = [
        migrations.AddField(
            model_name='customer',
            name='manager',
            field=models.ForeignKey(to=settings.AUTH_USER_MODEL, blank=True, null=True, on_delete=models.SET_NULL),
            preserve_default=True,
        ),
        migrations.AlterField(
            model_name='userprofile',
            name='customer',
            field=models.ForeignKey(to='gracopumpapp.Customer', default=1, on_delete=models.SET_NULL),  # gracopumpapp.models.get_none_customer),
        ),
    ]
