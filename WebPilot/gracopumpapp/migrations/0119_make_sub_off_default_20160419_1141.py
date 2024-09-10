# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


def make_subscription_off_by_default(apps, schema_editor):
    SitePreference = apps.get_model('gracopumpapp', 'SitePreference')

    global_disable_key = 'disable_all_subscriptions'

    if not SitePreference.objects.filter(key=global_disable_key).exists():
        SitePreference.objects.create(key=global_disable_key, value='1')


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0118_pump_suspended'),
    ]

    operations = [
        migrations.RunPython(make_subscription_off_by_default)
    ]
