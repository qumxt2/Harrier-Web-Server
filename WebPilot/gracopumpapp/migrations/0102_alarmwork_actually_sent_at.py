# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0101_alarmwork_created_at'),
    ]

    operations = [
        migrations.AddField(
            model_name='alarmwork',
            name='actually_sent_at',
            field=models.DateTimeField(null=True),
            preserve_default=True,
        ),
    ]
