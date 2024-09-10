# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0008_auto_20140928_1520'),
    ]

    operations = [
        migrations.CreateModel(
            name='History',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True, serialize=False)),
                ('pump_id', models.CharField(default='', max_length=100)),
                ('timestamp', models.DateTimeField(auto_now_add=True)),
                ('attribute', models.CharField(default='', max_length=200)),
                ('value', models.CharField(default='', max_length=200)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
