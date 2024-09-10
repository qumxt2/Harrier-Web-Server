# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0058_auto_20150606_2035'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='activation_key_last_changed',
            field=models.DateTimeField(null=True, default=None),
            preserve_default=True,
        ),
    ]
