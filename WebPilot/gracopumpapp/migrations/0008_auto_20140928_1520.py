# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0007_auto_20140928_1111'),
    ]

    operations = [
        migrations.RenameField(
            model_name='log',
            old_name='unique_id',
            new_name='origin_id',
        ),
    ]
