# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0040_auto_20150323_2152'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='signal_strength',
            field=models.IntegerField(default=-1),
            preserve_default=True,
        ),
    ]
