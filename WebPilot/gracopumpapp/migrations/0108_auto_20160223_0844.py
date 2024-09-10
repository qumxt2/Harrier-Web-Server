# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0107_auto_20160223_0841'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='pump',
            name='mqtt_pw_clear',
        ),
        migrations.RemoveField(
            model_name='pump',
            name='mqtt_username',
        ),
        migrations.AddField(
            model_name='mqttauth',
            name='pw_clear',
            field=models.CharField(max_length=255, null=True),
            preserve_default=True,
        ),
        migrations.AlterField(
            model_name='mqttauth',
            name='pw_hashed',
            field=models.CharField(max_length=255, null=True),
        ),
    ]
