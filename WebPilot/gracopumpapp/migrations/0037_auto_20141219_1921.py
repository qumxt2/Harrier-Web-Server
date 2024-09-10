# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import datetime

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0036_userprofile_last_activity'),
    ]

    operations = [
        migrations.AlterField(
            model_name='userprofile',
            name='last_activity',
            field=models.DateTimeField(default=datetime.datetime(2014, 12, 19, 19, 21, 28, 811549)),
        ),
    ]
