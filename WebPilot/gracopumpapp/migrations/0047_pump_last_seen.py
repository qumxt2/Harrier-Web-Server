# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import datetime

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0046_auto_20150415_1303'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='last_seen',
            field=models.DateTimeField(default=datetime.datetime(2014, 1, 1, 0, 0, 1, 347812)),
            preserve_default=True,
        ),
    ]
