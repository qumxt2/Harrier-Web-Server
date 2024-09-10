# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0098_alarmwork'),
    ]

    operations = [
        migrations.RenameField(
            model_name='userprofile',
            old_name='alarm_notifications',
            new_name='enable_alarm_alerts',
        ),
    ]
