# -*- coding: utf-8 -*-
'''
First step of migration of user->customer association: record the ID of the existing customer for each user

'''

from __future__ import unicode_literals

from django.db import models, migrations


def record_customer_ids(apps, schema_editor):
    UserProfile = apps.get_model('gracopumpapp', 'UserProfile')

    ups = UserProfile.objects.all()

    for up in ups:
        customer_id = up.customer_old.id
        up.cstmrid = customer_id
        up.save()


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0081_userprofile_cstmrid'),
    ]

    operations = [
        migrations.RunPython(record_customer_ids)
    ]

