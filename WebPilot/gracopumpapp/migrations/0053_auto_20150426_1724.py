# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0052_auto_20150426_1720'),
    ]

    operations = [
        migrations.RenameField(
            model_name='userprofile',
            old_name='notifications_enabled',
            new_name='enable_notifications',
        ),
    ]
