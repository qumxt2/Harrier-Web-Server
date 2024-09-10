# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0053_auto_20150426_1724'),
    ]

    operations = [
        migrations.RenameField(
            model_name='notification',
            old_name='note_type',
            new_name='subject',
        ),
    ]
