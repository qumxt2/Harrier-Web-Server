# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0091_auto_20150728_2217'),
    ]

    operations = [
        migrations.AddField(
            model_name='notification',
            name='custom_subject',
            field=models.CharField(null=True, max_length=128, default=None),
            preserve_default=True,
        ),
    ]
