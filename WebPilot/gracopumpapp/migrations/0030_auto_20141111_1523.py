# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0029_userprofile_time_zone'),
    ]

    # Changed this migration to set a different initial default in the previous migration (0029), making this one unnecessary
    operations = [
    ]
