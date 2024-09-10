# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0072_userprofile_password_change_needed'),
    ]

    operations = [
        migrations.CreateModel(
            name='TermsOfService',
            fields=[
                ('id', models.AutoField(serialize=False, auto_created=True, verbose_name='ID', primary_key=True)),
                ('content', models.TextField(null=True, default=None)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.AddField(
            model_name='userprofile',
            name='tos_agreement_date',
            field=models.DateTimeField(null=True, default=None),
            preserve_default=True,
        ),
    ]
