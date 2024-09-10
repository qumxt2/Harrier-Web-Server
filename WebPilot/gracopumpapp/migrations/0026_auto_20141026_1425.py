# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


def assign_all_to_graco(apps, schema_editor):
    '''Assign all existing users to Graco and create user profiles'''
    from django.contrib.auth.models import User, Group
    from gracopumpapp.models import UserProfile, Customer
#     UserProfile = apps.get_model("gracopumpapp", "UserProfile")
#     Customer = apps.get_model("gracopumpapp", "Customer")

    graco = Customer.objects.get(organization_name='Graco')

    all_users = User.objects.all()

    for user in all_users:
        print(repr(user))
        if not UserProfile.objects.filter(user=user).exists():
            new_up = UserProfile.objects.create(user=user, customer=graco)
            new_up.save()


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0025_remove_userprofile_full_name'),
    ]

    operations = [
# Disabled because Graco won't exist on virgin installations. Uncomment the following line for non-virgin installs
#                  migrations.RunPython(assign_all_to_graco)
    ]
