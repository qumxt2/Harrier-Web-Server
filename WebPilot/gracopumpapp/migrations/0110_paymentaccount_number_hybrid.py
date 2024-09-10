# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0109_auto_20151007_0820'),
    ]

    operations = [
        migrations.AddField(
            model_name='paymentaccount',
            name='number_hybrid',
            field=models.CharField(max_length=50, null=True),
            preserve_default=True,
        ),
    ]
