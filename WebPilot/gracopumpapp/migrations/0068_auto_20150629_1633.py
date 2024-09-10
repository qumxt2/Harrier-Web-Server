# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0067_log_entry_format'),
    ]

    operations = [
        migrations.AlterField(
            model_name='log',
            name='entry_format',
            field=models.IntegerField(default=2),
        ),
    ]
