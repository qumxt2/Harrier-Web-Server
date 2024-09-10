# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0034_remove_customer_is_active'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='pump',
            name='clear_alarm_status',
        ),
        migrations.RemoveField(
            model_name='pump',
            name='reset_totalizer',
        ),
        migrations.RemoveField(
            model_name='pump',
            name='set_pump_status',
        ),
        migrations.AlterField(
            model_name='customer',
            name='organization_name',
            field=models.CharField(unique=True, max_length=200),
        ),
    ]
