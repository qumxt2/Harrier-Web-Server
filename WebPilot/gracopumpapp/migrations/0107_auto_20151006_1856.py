# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0106_remove_plan_term_months'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='customer',
            name='subscription',
        ),
        migrations.AddField(
            model_name='subscription',
            name='customer',
            field=models.ForeignKey(on_delete=django.db.models.deletion.SET_NULL, to='gracopumpapp.Customer', null=True),
            preserve_default=True,
        ),
    ]
