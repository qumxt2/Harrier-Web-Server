# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0023_auto_20141025_1347'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='location_reported',
            field=models.CharField(max_length=200, default=''),
            preserve_default=True,
        ),
    ]
