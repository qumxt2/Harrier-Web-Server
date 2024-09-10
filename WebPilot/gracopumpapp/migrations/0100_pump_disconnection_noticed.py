# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0099_auto_20150918_1505'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='disconnection_noticed',
            field=models.BooleanField(default=True),
            preserve_default=True,
        ),
    ]
