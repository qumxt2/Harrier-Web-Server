# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0094_auto_20150730_1651'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='notification',
            name='subject',
        ),
    ]
