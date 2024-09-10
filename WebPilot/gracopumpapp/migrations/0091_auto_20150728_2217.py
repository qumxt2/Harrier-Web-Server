# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0090_invitation'),
    ]

    operations = [
        migrations.RenameField(
            model_name='invitation',
            old_name='invitation_code',
            new_name='code',
        ),
        migrations.RenameField(
            model_name='invitation',
            old_name='invitation_state',
            new_name='state',
        ),
    ]
