# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0088_auto_20150724_0922'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='power_save_mode',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
    ]
