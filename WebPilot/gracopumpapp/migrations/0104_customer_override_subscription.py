# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0103_auto_20150930_2130'),
    ]

    operations = [
        migrations.AddField(
            model_name='customer',
            name='override_subscription',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
    ]
