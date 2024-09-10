# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0024_pump_location_reported'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='userprofile',
            name='full_name',
        ),
    ]
