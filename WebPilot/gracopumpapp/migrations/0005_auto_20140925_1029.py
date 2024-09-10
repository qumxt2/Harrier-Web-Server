# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0004_merge'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='clear_alarm_status',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='reset_totalizer',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='set_pump_status',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
    ]
