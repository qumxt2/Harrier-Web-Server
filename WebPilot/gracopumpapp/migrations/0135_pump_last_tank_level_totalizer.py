# -*- coding: utf-8 -*-
# Generated by Django 1.10.2 on 2017-08-06 17:53
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0134_auto_20170726_0926'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='last_tank_level_totalizer',
            field=models.IntegerField(default=-1),
        ),
    ]
