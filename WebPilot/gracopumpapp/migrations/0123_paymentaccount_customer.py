# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0122_auto_20160426_1825'),
    ]

    operations = [
        migrations.AddField(
            model_name='paymentaccount',
            name='customer',
            field=models.ForeignKey(blank=True, on_delete=django.db.models.deletion.SET_NULL, to='gracopumpapp.Customer', null=True),
            preserve_default=True,
        ),
    ]
