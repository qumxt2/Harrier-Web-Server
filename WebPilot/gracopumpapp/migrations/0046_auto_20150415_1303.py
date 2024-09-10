# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0045_auto_20150409_2050'),
    ]

    operations = [
        migrations.AlterField(
            model_name='userprofile',
            name='show_inactive_pumps',
            field=models.IntegerField(default=1),
        ),
    ]
