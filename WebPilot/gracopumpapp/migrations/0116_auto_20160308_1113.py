# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.db.models.deletion
import gracopumpapp.models


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0115_auto_20160308_1113'),
    ]

    '''
    This migration should in theory be a no-op, but it's needed to ensure that the default customer for new pumps is still the
    actual None customer instead of the default of "1" that was set back in migrations 60 and 62. On new systems, the None
    customer will probably be "1", but on existing systems, it's possible that the None customer will have a different ID.
    
    See migration 60 for an explanation for why it was hard-coded in the first place.
    '''

    operations = [
        migrations.AlterField(
            model_name='pump',
            name='customer',
            field=models.ForeignKey(default=gracopumpapp.models.get_none_customer, to='gracopumpapp.Customer', on_delete=django.db.models.deletion.SET_DEFAULT),
        ),
    ]
