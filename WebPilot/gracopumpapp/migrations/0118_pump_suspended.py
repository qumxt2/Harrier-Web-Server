# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0117_sitepreference'),
    ]

    operations = [
        migrations.AddField(
            model_name='pump',
            name='suspended',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
    ]
