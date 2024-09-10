# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0097_userprofile_alarm_notifications'),
    ]

    operations = [
        migrations.CreateModel(
            name='AlarmWork',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True, serialize=False)),
                ('alarm_id', models.IntegerField(default=-1)),
                ('done', models.BooleanField(default=False)),
                ('pump', models.ForeignKey(to='gracopumpapp.Pump', null=True, on_delete=models.SET_NULL)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
