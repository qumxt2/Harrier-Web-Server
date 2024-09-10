# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0100_pump_disconnection_noticed'),
    ]

    operations = [
        migrations.AddField(
            model_name='alarmwork',
            name='created_at',
            field=models.DateTimeField(null=True),
            preserve_default=True,
        ),
    ]
