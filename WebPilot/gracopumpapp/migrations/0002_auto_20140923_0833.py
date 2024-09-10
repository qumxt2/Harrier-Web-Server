# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0001_initial'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='pump',
            name='user',
        ),
        migrations.DeleteModel(
            name='User',
        ),
        migrations.AddField(
            model_name='pump',
            name='alarms_status',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='connection',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='flow_rate',
            field=models.FloatField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='flow_units',
            field=models.CharField(max_length=50, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='owner',
            field=models.CharField(max_length=200, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='pump_topology',
            field=models.CharField(max_length=50, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='status',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='totalizer_grand',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='pump',
            name='totalizer_resetable',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AlterField(
            model_name='pump',
            name='location',
            field=models.CharField(max_length=200, default=''),
        ),
        migrations.AlterField(
            model_name='pump',
            name='pretty_name',
            field=models.CharField(max_length=200, default=''),
        ),
        migrations.AlterField(
            model_name='pump',
            name='unique_id',
            field=models.CharField(max_length=100, default=''),
        ),
    ]
