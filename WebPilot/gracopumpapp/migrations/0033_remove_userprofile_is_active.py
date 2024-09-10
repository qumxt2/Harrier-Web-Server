# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0032_auto_20141205_1851'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='userprofile',
            name='is_active',
        ),
    ]
