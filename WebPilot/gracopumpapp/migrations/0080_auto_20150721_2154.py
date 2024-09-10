# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0079_auto_20150721_1709'),
    ]

    operations = [
        migrations.RenameField(
            model_name='userprofile',
            old_name='customer',
            new_name='customer_old',
        ),
    ]
