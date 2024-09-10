# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0043_auto_20150329_2210'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='mqtt_pw_clear',
            field=models.CharField(max_length=100, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='mqtt_username',
            field=models.CharField(max_length=100, default=''),
            preserve_default=True,
        ),
    ]
