# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0080_auto_20150721_2154'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='cstmrid',
            field=models.IntegerField(null=True, default=None),
            preserve_default=True,
        ),
    ]
