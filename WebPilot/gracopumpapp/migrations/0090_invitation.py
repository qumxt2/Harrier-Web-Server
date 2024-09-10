# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.conf import settings
from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
        ('gracopumpapp', '0089_pump_power_save_mode'),
    ]

    operations = [
        migrations.CreateModel(
            name='Invitation',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('issued_at', models.DateTimeField(auto_now_add=True)),
                ('invited_email', models.CharField(max_length=255, default='')),
                ('accepted_email', models.CharField(max_length=255, default='')),
                ('invitation_code', models.CharField(max_length=255, default='')),
                ('invitation_state', models.IntegerField(default=0)),
                ('customer_invited', models.ForeignKey(to='gracopumpapp.Customer', null=True, default=None, on_delete=models.SET_NULL)),
                ('invite_accepted_by', models.ForeignKey(related_name='+', to=settings.AUTH_USER_MODEL, null=True, default=None, on_delete=models.SET_NULL)),
                ('invite_from', models.ForeignKey(related_name='invitations_created', to=settings.AUTH_USER_MODEL, null=True, default=None, on_delete=models.SET_NULL)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
