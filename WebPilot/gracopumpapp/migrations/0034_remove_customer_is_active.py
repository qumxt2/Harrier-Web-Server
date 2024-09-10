# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0033_remove_userprofile_is_active'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='customer',
            name='is_active',
        ),
    ]
