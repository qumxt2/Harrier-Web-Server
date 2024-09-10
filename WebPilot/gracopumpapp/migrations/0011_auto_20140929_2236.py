# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0010_customer'),
    ]

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='unique_id',
            field=models.CharField(unique=True, max_length=100),
        ),
    ]
