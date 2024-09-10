# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0048_userprofile_display_units'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='pump',
            name='flow_units',
        ),
        migrations.RemoveField(
            model_name='pump',
            name='flow_units_time',
        ),
    ]
