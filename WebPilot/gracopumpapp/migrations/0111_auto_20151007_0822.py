# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0110_paymentaccount_number_hybrid'),
    ]

    operations = [
        migrations.RenameField(
            model_name='paymentaccount',
            old_name='number_hybrid',
            new_name='account_code',
        ),
    ]
