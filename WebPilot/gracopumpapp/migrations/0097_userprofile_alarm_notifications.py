# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0096_alarmcustomization'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='alarm_notifications',
            field=models.IntegerField(default=1),
            preserve_default=True,
        ),
    ]
