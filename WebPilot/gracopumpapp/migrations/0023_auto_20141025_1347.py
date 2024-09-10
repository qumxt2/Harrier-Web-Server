# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0022_auto_20141021_1206'),
    ]

    operations = [
        migrations.RenameField(
            model_name='pump',
            old_name='location',
            new_name='location_name',
        ),
    ]
