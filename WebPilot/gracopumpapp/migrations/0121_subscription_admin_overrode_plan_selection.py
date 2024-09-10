# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0120_subscription_quantity'),
    ]

    operations = [
        migrations.AddField(
            model_name='subscription',
            name='admin_overrode_plan_selection',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
    ]
