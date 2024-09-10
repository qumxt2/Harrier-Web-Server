# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


def create_default_customer(apps, schema_editor):
    '''Create a default customer for new pumps'''
    Customer = apps.get_model('gracopumpapp', 'Customer')

    default_customer = Customer.objects.create(organization_name='None')
    default_customer.save()

class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0026_auto_20141026_1425'),
    ]

    operations = [
                  migrations.RunPython(create_default_customer)
    ]
