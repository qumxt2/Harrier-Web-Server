# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0012_auto_20140929_2248'),
    ]

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='customer',
            field=models.ForeignKey(default=1, to='gracopumpapp.Customer', on_delete=models.SET_NULL),
        ),
    ]
