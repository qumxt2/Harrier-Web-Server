# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import datetime

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0035_auto_20141209_2106'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='last_activity',
            field=models.DateTimeField(default=datetime.datetime(2014, 12, 19, 10, 32, 23, 347812)),
            preserve_default=True,
        ),
    ]
