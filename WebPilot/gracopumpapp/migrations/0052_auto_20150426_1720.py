# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0051_notification'),
    ]

    operations = [
        migrations.AlterField(
            model_name='userprofile',
            name='notifications_enabled',
            field=models.IntegerField(default=1),
        ),
    ]
