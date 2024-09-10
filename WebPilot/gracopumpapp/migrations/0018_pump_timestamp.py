# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import datetime

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0017_userprofile'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='timestamp',
            field=models.DateTimeField(default=datetime.datetime.now, auto_now_add=True),
            preserve_default=True,
        ),
    ]
