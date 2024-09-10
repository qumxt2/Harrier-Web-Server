# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0047_pump_last_seen'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='display_units',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
