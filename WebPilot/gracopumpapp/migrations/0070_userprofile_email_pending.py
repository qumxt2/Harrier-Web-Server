# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0069_pump_battery_warning_trigger'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='email_pending',
            field=models.CharField(null=True, default=None, max_length='128'),
            preserve_default=True,
        ),
    ]
