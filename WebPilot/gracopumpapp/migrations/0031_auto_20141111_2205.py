# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0030_auto_20141111_1523'),
    ]

    operations = [
        migrations.AlterField(
            model_name='history',
            name='timestamp',
            field=models.DateTimeField(),
        ),
    ]
