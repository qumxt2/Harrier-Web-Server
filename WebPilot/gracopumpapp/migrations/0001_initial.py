# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Pump',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True, serialize=False)),
                ('unique_id', models.CharField(max_length=100)),
                ('pretty_name', models.CharField(max_length=200)),
                ('location', models.CharField(max_length=200)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='User',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True, serialize=False)),
                ('username', models.CharField(max_length=100)),
                ('salt', models.CharField(max_length=50)),
                ('pw_hash', models.CharField(max_length=100)),
                ('is_admin', models.BooleanField(default=False)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.AddField(
            model_name='pump',
            name='user',
            field=models.ForeignKey(to='gracopumpapp.User', on_delete=models.SET_NULL),
            preserve_default=True,
        ),
    ]
