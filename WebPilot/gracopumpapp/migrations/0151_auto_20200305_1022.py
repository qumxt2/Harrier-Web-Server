# -*- coding: utf-8 -*-
# Generated by Django 1.10.2 on 2020-03-05 16:22
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0150_auto_20200305_1003'),
    ]

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='ain_flow_rate_high',
            field=models.FloatField(default=0),
        ),
        migrations.AlterField(
            model_name='pump',
            name='ain_flow_rate_low',
            field=models.FloatField(default=0),
        ),
    ]
