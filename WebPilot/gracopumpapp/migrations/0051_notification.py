# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.conf import settings
from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
        ('gracopumpapp', '0050_userprofile_notifications_enabled'),
    ]

    operations = [
        migrations.CreateModel(
            name='Notification',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('period', models.IntegerField(default=0)),
                ('time_next_due', models.DateTimeField(null=True)),
                ('time_last_sent', models.DateTimeField(null=True)),
                ('note_type', models.IntegerField(default=0)),
                ('criteria', models.IntegerField(default=0)),
                ('sent_since_last_set', models.BooleanField(default=False)),
                ('is_enabled', models.BooleanField(default=True)),
                ('pump', models.ForeignKey(to='gracopumpapp.Pump', null=True, on_delete=models.SET_NULL)),
                ('user', models.ForeignKey(to=settings.AUTH_USER_MODEL, null=True, on_delete=models.SET_NULL)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
