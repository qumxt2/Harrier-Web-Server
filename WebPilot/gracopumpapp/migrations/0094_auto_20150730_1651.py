# -*- coding: utf-8 -*-
'''
Convert old indexed notifications into text
'''

from __future__ import unicode_literals

from django.db import models, migrations


def convert_notifications(apps, schema_editor):
    Notification = apps.get_model('gracopumpapp', 'Notification')

    subject_key = ['Unknown',
                 'Pump Service',
                 'Motor Service',
                 'Battery Service',
                 'Chemical Tank Refill',
                 'Custom',
                 ]

    notifications = Notification.objects.all()
    
    for notification in notifications:
        if notification.subject < len(subject_key):
            subject_text = subject_key[notification.subject]
        else:
            subject_text = 'Unknown'

        notification.subject_text = subject_text
        notification.save()


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0093_auto_20150730_1648'),
    ]

    operations = [
        migrations.RunPython(convert_notifications)
    ]

