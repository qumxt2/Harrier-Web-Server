# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0105_auto_20151002_2336'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='plan',
            name='term_months',
        ),
    ]
