# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0054_auto_20150427_1521'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='location_marked',
            field=models.CharField(default='', max_length=200),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='location_source',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
