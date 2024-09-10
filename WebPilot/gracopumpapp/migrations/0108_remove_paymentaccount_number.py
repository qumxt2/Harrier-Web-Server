# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0107_auto_20151006_1856'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='paymentaccount',
            name='number',
        ),
    ]
