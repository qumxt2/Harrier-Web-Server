# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0101_auto_20150930_1602'),
    ]

    operations = [
        migrations.AlterField(
            model_name='paymentaccount',
            name='customer',
            field=models.ForeignKey(to='gracopumpapp.Customer', on_delete=django.db.models.deletion.SET_NULL, null=True),
        ),
        migrations.AlterField(
            model_name='subscription',
            name='account',
            field=models.ForeignKey(to='gracopumpapp.PaymentAccount', on_delete=django.db.models.deletion.SET_NULL, null=True),
        ),
    ]
