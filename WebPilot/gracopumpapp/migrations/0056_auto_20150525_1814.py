# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0055_auto_20150504_0003'),
    ]

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='timestamp',
            field=models.DateTimeField(auto_now=True),
        ),
    ]
