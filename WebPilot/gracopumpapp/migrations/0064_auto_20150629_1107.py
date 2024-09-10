# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.conf import settings
from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
        ('gracopumpapp', '0063_userprofile_verification_key'),
    ]

    operations = [
        migrations.AddField(
            model_name='log',
            name='attribute',
            field=models.CharField(max_length=100, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='log',
            name='new_value',
            field=models.CharField(max_length=100, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='log',
            name='old_value',
            field=models.CharField(max_length=100, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='log',
            name='origin_ip',
            field=models.CharField(max_length=100, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='log',
            name='target_id',
            field=models.CharField(max_length=100, default=''),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='log',
            name='target_type',
            field=models.IntegerField(default=0),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='log',
            name='user_actor',
            field=models.ForeignKey(on_delete=django.db.models.deletion.SET_DEFAULT, to=settings.AUTH_USER_MODEL, default=None, null=True),
            preserve_default=True,
        ),
    ]
