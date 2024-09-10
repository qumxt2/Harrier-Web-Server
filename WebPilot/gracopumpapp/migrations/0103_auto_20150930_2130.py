# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0102_auto_20150930_1604'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='paymentaccount',
            name='customer',
        ),
        migrations.AddField(
            model_name='customer',
            name='subscription',
            field=models.ForeignKey(on_delete=django.db.models.deletion.SET_NULL, to='gracopumpapp.Subscription', null=True, blank=True),
            preserve_default=True,
        ),
    ]
