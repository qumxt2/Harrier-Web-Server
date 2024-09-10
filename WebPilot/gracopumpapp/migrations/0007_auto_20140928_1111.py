# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0006_auto_20140928_1022'),
    ]

    operations = [
        migrations.AlterField(
            model_name='log',
            name='timestamp',
            field=models.DateTimeField(auto_now_add=True),
        ),
    ]
