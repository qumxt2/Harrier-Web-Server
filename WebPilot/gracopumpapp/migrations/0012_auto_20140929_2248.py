# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0011_auto_20140929_2236'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='pump',
            name='owner',
        ),
        migrations.AddField(
            model_name='pump',
            name='customer',
            field=models.ForeignKey(default=1, to='gracopumpapp.Customer', on_delete=models.SET_NULL),
            preserve_default=True,
        ),
    ]
