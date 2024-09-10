# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0082_auto_20150722_1154'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='userprofile',
            name='customer_old',
        ),
    ]
