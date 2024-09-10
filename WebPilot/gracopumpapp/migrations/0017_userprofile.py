# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.conf import settings
from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
        ('gracopumpapp', '0016_auto_20141007_1621'),
    ]

    operations = [
        migrations.CreateModel(
            name='UserProfile',
            fields=[
                ('id', models.AutoField(serialize=False, auto_created=True, verbose_name='ID', primary_key=True)),
                ('full_name', models.CharField(max_length=200, default='')),
                ('customer', models.ForeignKey(to='gracopumpapp.Customer', default=1, on_delete=models.SET_NULL)),
                ('user', models.OneToOneField(to=settings.AUTH_USER_MODEL, on_delete=models.SET_DEFAULT, default=None)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
