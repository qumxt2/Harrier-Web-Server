# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0121_subscription_admin_overrode_plan_selection'),
    ]

    operations = [
        migrations.AddField(
            model_name='plan',
            name='max_pumps',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='plan',
            name='min_pumps',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
