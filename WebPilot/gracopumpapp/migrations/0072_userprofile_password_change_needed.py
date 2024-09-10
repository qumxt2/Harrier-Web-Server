# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0071_remove_userprofile_email_pending'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='password_change_needed',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
    ]
