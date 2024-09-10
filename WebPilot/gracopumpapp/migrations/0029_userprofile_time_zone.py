# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0028_userprofile_show_inactive_pumps'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='time_zone',
            field=models.CharField(max_length='100', default='UTC'),
            preserve_default=True,
        ),
    ]
