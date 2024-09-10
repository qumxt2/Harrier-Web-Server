# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0005_auto_20140925_1029'),
    ]

    operations = [
        migrations.CreateModel(
            name='Log',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True, serialize=False)),
                ('origin_type', models.IntegerField(default=0)),
                ('unique_id', models.CharField(default='', max_length=100)),
                ('timestamp', models.DateField(auto_now_add=True)),
                ('event_type', models.IntegerField(default=0)),
                ('message', models.TextField(default='')),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.AddField(
            model_name='pump',
            name='metering_mode',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
    ]
