# -*- coding: utf-8 -*-
# Generated by Django 1.10.1 on 2016-09-30 21:40
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0123_paymentaccount_customer'),
    ]

    operations = [
        migrations.AlterField(
            model_name='userprofile',
            name='time_zone',
            field=models.CharField(default='UTC', max_length=100),
        ),
        migrations.AlterField(
            model_name='userprofile',
            name='verification_key',
            field=models.CharField(default=None, max_length=128, null=True),
        ),
    ]
