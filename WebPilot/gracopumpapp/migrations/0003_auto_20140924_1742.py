# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0002_auto_20140923_0833'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='flow_units_time',
            field=models.CharField(default='day', max_length=50),
            preserve_default=True,
        ),
        migrations.AlterField(
            model_name='pump',
            name='flow_units',
            field=models.CharField(default='gal', max_length=50),
        ),
    ]
