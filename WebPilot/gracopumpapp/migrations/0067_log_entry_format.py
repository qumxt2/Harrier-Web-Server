# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0066_log_action'),
    ]

    operations = [
        migrations.AddField(
            model_name='log',
            name='entry_format',
            field=models.IntegerField(default=1),
            preserve_default=True,
        ),
    ]
