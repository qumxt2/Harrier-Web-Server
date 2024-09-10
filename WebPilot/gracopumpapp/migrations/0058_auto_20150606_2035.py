# -*- coding: utf-8 -*-
'''
This migration was going to set up initial activation keys for all pre-existing pumps, but I 
decided to take a different approach.
'''

from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0057_pump_activation_key'),
    ]

    operations = [
    ]
