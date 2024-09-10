# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0104_auto_20160211_1630'),
    ]

    operations = [
        migrations.AlterField(
            model_name='mqttacl',
            name='rw',
            field=models.IntegerField(default=0),
        ),
    ]
