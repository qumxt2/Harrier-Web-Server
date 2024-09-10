# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0062_auto_20150622_1646'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='verification_key',
            field=models.CharField(null=True, default=None, max_length='128'),
            preserve_default=True,
        ),
    ]
