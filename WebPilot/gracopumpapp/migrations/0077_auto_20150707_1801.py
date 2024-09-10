# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0076_auto_20150707_1111'),
    ]

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='last_seen',
            field=models.DateTimeField(null=True, default=None),
        ),
    ]
