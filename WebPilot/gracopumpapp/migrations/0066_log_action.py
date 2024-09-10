# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0065_log_success'),
    ]

    operations = [
        migrations.AddField(
            model_name='log',
            name='action',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
