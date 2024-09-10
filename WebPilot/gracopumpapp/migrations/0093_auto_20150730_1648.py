# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0092_notification_custom_subject'),
    ]

    operations = [
        migrations.RenameField(
            model_name='notification',
            old_name='custom_subject',
            new_name='subject_text',
        ),
    ]
