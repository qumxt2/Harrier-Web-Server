# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0064_auto_20150629_1107'),
    ]

    operations = [
        migrations.AddField(
            model_name='log',
            name='success',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
