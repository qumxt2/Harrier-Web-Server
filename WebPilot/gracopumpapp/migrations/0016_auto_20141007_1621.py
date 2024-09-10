# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0015_auto_20141007_1317'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='metering_off_time',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='metering_on_cycles',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='metering_on_time',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
