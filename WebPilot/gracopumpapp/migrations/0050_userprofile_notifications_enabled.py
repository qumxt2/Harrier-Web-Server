# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0049_auto_20150424_1830'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='notifications_enabled',
            field=models.BooleanField(default=True),
            preserve_default=True,
        ),
    ]
