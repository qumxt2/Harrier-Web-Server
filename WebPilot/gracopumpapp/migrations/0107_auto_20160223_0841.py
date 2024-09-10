# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0106_auto_20160212_1212'),
    ]

    operations = [
        migrations.RenameField(
            model_name='mqttauth',
            old_name='pw',
            new_name='pw_hashed',
        ),
        migrations.AddField(
            model_name='mqttacl',
            name='pump',
            field=models.ForeignKey(null=True, blank=True, to='gracopumpapp.Pump', on_delete=models.SET_NULL),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='mqttauth',
            name='pump',
            field=models.ForeignKey(null=True, blank=True, to='gracopumpapp.Pump', on_delete=models.SET_NULL),
            preserve_default=True,
        ),
    ]
