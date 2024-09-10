# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0073_auto_20150707_1102'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='tos_agreed',
            field=models.ForeignKey(default=None, to='gracopumpapp.TermsOfService', null=True, on_delete=django.db.models.deletion.SET_DEFAULT),
            preserve_default=True,
        ),
    ]
