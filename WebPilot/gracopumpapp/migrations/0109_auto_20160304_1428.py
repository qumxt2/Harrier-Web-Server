# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0108_auto_20160223_0844'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='mqttacl',
            name='pump',
        ),
        migrations.RemoveField(
            model_name='mqttauth',
            name='pump',
        ),
        migrations.AddField(
            model_name='mqttacl',
            name='mqtt_auth',
            field=models.ForeignKey(to='gracopumpapp.MqttAuth', null=True, blank=True, on_delete=models.SET_NULL),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='mqtt_auth',
            field=models.ForeignKey(on_delete=django.db.models.deletion.SET_DEFAULT, to='gracopumpapp.MqttAuth', default=None, null=True),
            preserve_default=True,
        ),
    ]
