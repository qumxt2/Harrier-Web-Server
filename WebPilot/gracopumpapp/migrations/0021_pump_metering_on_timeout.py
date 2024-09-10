# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0020_auto_20141008_2135'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='metering_on_timeout',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
