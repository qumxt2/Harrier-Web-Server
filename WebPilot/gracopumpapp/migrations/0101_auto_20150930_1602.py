# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0100_auto_20150929_1918'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='invoice',
            name='account',
        ),
        migrations.DeleteModel(
            name='Invoice',
        ),
    ]
