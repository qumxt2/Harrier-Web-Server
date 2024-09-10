# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.conf import settings
from django.db import models, migrations
import django.db.models.deletion
import gracopumpapp.models


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0061_userprofile_email_confirmed'),
    ]

    '''
    These changes prevent cascading deletes when certain users or managers are deleted.

    See comments in migration 60 about why the Customer default for the Pump model was changed to a
    hard-coded "1" instead of dynamically fetching the None user's ID.
    '''

    operations = [
        migrations.AlterField(
            model_name='customer',
            name='manager',
            field=models.ForeignKey(on_delete=django.db.models.deletion.SET_NULL, to=settings.AUTH_USER_MODEL, blank=True, null=True),
        ),
        migrations.AlterField(
            model_name='pump',
            name='customer',
            field=models.ForeignKey(default=1, on_delete=django.db.models.deletion.SET_DEFAULT, to='gracopumpapp.Customer'),
        ),
        migrations.AlterField(
            model_name='userprofile',
            name='customer',
            field=models.ForeignKey(default=1, on_delete=django.db.models.deletion.SET_DEFAULT, to='gracopumpapp.Customer'),
        ),
    ]
