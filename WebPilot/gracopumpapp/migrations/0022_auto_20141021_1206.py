# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


def add_groups(apps, schema_editor):
    from django.contrib.auth.models import User, Group

    admin_group = Group(name="Admins")
    admin_group.save()

class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0021_pump_metering_on_timeout'),
    ]

    operations = [
                  migrations.RunPython(add_groups)
    ]
