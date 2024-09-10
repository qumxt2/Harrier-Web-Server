# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0102_alarmwork_actually_sent_at'),
    ]

    operations = [
        migrations.CreateModel(
            name='MqttACL',
            fields=[
                ('id', models.AutoField(primary_key=True, auto_created=True, serialize=False, verbose_name='ID')),
                ('username', models.CharField(max_length=100)),
                ('topic', models.CharField(max_length=255)),
                ('rw', models.BooleanField(default=False)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='MqttAuth',
            fields=[
                ('id', models.AutoField(primary_key=True, auto_created=True, serialize=False, verbose_name='ID')),
                ('username', models.CharField(max_length=100)),
                ('pw', models.CharField(max_length=255)),
                ('super', models.BooleanField(default=False)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
