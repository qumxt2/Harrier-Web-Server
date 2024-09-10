# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0056_auto_20150525_1814'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='activation_key',
            field=models.CharField(max_length=30, default=''),
            preserve_default=True,
        ),
    ]
