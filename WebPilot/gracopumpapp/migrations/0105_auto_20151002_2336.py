# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0104_customer_override_subscription'),
    ]

    operations = [
        migrations.AddField(
            model_name='plan',
            name='code',
            field=models.CharField(default=None, max_length=100, null=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='plan',
            name='plan_interval',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='plan',
            name='plan_units',
            field=models.CharField(default=None, max_length=100, null=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='plan',
            name='setup_cents',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='plan',
            name='trial_interval',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='plan',
            name='trial_units',
            field=models.CharField(default=None, max_length=100, null=True),
            preserve_default=True,
        ),
    ]
