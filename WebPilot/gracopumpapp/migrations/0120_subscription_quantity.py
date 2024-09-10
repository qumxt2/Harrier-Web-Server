# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0119_make_sub_off_default_20160419_1141'),
    ]

    operations = [
        migrations.AddField(
            model_name='subscription',
            name='quantity',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
