# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0039_auto_20150308_2230'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='high_pressure_trigger',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='low_battery_trigger',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='low_pressure_trigger',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
    ]
