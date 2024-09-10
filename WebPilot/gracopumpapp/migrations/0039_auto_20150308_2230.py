# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0038_auto_20141219_1923'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='battery_voltage',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='pressure_level',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
    ]
