# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0042_pump_system_publication_period'),
    ]

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='status',
            field=models.IntegerField(default=0),
        ),
    ]
