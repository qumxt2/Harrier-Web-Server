# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0070_userprofile_email_pending'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='userprofile',
            name='email_pending',
        ),
    ]
