# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0083_remove_userprofile_customer_old'),
    ]

    operations = [
        migrations.AddField(
            model_name='userprofile',
            name='customers',
            field=models.ManyToManyField(to='gracopumpapp.Customer'),
            preserve_default=True,
        ),
    ]
