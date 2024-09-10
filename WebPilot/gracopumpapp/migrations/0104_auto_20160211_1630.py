# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0103_mqttacl_mqttauth'),
    ]

    operations = [
        migrations.RenameField(
            model_name='mqttauth',
            old_name='super',
            new_name='superuser',
        ),
    ]
