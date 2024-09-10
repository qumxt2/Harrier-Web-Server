# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0068_auto_20150629_1633'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='battery_warning_trigger',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
    ]
