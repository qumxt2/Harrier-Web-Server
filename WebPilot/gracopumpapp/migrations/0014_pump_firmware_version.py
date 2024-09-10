# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0013_auto_20140929_2254'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='firmware_version',
            field=models.CharField(max_length=20, default=''),
            preserve_default=True,
        ),
    ]
