# -*- coding: utf-8 -*-
# Generated by Django 1.10.2 on 2017-08-14 17:59
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0135_pump_last_tank_level_totalizer'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='analog_input_mode',
            field=models.IntegerField(default=0),
        ),
    ]
