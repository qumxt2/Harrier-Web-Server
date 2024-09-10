# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0014_pump_firmware_version'),
    ]

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='totalizer_grand',
            field=models.FloatField(default=0),
        ),
        migrations.AlterField(
            model_name='pump',
            name='totalizer_resetable',
            field=models.FloatField(default=0),
        ),
    ]
