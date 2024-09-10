# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0095_remove_notification_subject'),
    ]

    operations = [
        migrations.CreateModel(
            name='AlarmCustomization',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, primary_key=True, auto_created=True)),
                ('alarm_id', models.IntegerField(default=-1)),
                ('custom_alarm_name', models.CharField(max_length=100, default='')),
                ('pump', models.ForeignKey(null=True, to='gracopumpapp.Pump', on_delete=models.SET_NULL)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
