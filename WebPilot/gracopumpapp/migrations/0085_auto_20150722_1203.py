# -*- coding: utf-8 -*-
'''
Nearly final step for converting from one-to-many to many-to-many relationship for users and customers:

Reassociate the old customer for each user in the customers group for each user

'''

from __future__ import unicode_literals

from django.db import models, migrations


def add_customer_back_to_user(apps, schema_editor):
    UserProfile = apps.get_model('gracopumpapp', 'UserProfile')
    Customer = apps.get_model('gracopumpapp', 'Customer')

    ups = UserProfile.objects.all()

    for up in ups:
        customer = Customer.objects.get(id=up.cstmrid)
        up.customers.add(customer)
        up.save()


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0084_userprofile_customers'),
    ]

    operations = [
        migrations.RunPython(add_customer_back_to_user)
    ]
