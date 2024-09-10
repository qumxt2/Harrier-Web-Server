# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0041_pump_signal_strength'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='system_publication_period',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
    ]
