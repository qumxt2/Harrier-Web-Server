# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import gracopumpapp


def add_mqtt_system_users(apps, schema_editor):
    '''Add the MQTT auth and ACL entries for the system users'''

    '''
    Can't use the normal syntax for accessing consistent models in the migration, since that
    doesn't work for non-ORM methods
    '''
    # MqttAuth = apps.get_model("gracopumpapp", 'MqttAuth')
    MqttAuth = gracopumpapp.models.MqttAuth

    MqttAuth.set_password(username='gracoconsole', password='88K6CraMSWdn', superuser=True)
    MqttAuth.set_password(username='gracoweb', password='g9UdjawKEvZ7yUAN', superuser=True)

    '''
    Also add an entry for the MQTT user used by the currently connected pumps, since the
    credentials don't get re-fetched until the pump power cycles.
    '''
    MqttAuth.set_password(username='gracopump', password='48AGwPSWSZTte4g5', superuser=True)


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0109_auto_20160304_1428'),
    ]

    operations = [
        migrations.RunPython(add_mqtt_system_users)
    ]
