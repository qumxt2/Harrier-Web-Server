# -*- coding: utf-8 -*-
# Generated by Django 1.10.1 on 2017-01-11 06:23
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0129_plan_user_selectable'),
    ]

    operations = [
        migrations.AddField(
            model_name='subscription',
            name='trial_end',
            field=models.DateTimeField(default=None, null=True),
        ),
        migrations.AddField(
            model_name='subscription',
            name='trial_start',
            field=models.DateTimeField(default=None, null=True),
        ),
    ]
