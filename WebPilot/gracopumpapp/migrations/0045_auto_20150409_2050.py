# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


_none_customer_id = None


def get_none_customer_retro(apps, schema_editor):
    Customer = apps.get_model('gracopumpapp', 'Customer')

    customer_obj = Customer.objects.get(organization_name='None')
    _none_customer_id = customer_obj.pk


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0044_auto_20150409_2043'),
    ]

    operations = [
        migrations.RunPython(get_none_customer_retro),
        migrations.AlterField(
            model_name='pump',
            name='customer',
            field=models.ForeignKey(default=_none_customer_id, to='gracopumpapp.Customer', on_delete=models.SET_NULL),
        ),
    ]
