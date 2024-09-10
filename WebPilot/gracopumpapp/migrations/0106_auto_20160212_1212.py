# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0105_auto_20160212_1204'),
    ]

    operations = [
        migrations.AlterField(
            model_name='mqttauth',
            name='username',
            field=models.CharField(max_length=24),
        ),
        migrations.AlterField(
            model_name='pump',
            name='mqtt_pw_clear',
            field=models.CharField(default='', max_length=24),
        ),
        migrations.AlterField(
            model_name='pump',
            name='mqtt_username',
            field=models.CharField(default='', max_length=24),
        ),
    ]
