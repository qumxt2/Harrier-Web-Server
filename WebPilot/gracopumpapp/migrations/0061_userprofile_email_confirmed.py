# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0060_auto_20150609_1619'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='email_confirmed',
            field=models.BooleanField(default=True),
            preserve_default=True,
        ),
    ]
