# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0027_auto_20141026_1510'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='show_inactive_pumps',
            field=models.BooleanField(default=True),
            preserve_default=True,
        ),
    ]
