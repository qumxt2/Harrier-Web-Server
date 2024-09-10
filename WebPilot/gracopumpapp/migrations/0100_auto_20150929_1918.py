# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.conf import settings
from django.db import models, migrations
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
        ('gracopumpapp', '0099_auto_20150918_1505'),
    ]

    operations = [
        migrations.CreateModel(
            name='Invoice',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('number', models.IntegerField()),
                ('state', models.CharField(max_length=50)),
                ('date', models.DateTimeField()),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='PaymentAccount',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('number', models.IntegerField()),
                ('is_active', models.BooleanField(default=True)),
                ('customer', models.ForeignKey(to='gracopumpapp.Customer', on_delete=models.SET_NULL)),
                ('user', models.ForeignKey(null=True, to=settings.AUTH_USER_MODEL, on_delete=django.db.models.deletion.SET_NULL, blank=True)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Plan',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('name', models.CharField(max_length=100)),
                ('price_cents', models.IntegerField(default=0)),
                ('term_months', models.IntegerField(default=0)),
                ('is_available', models.BooleanField(default=True)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Subscription',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('status', models.IntegerField(default=0)),
                ('start', models.DateTimeField(null=True, default=None)),
                ('expiration', models.DateTimeField(null=True, default=None)),
                ('recurly_uuid', models.CharField(max_length=200, default=None, null=True)),
                ('account', models.ForeignKey(to='gracopumpapp.PaymentAccount', on_delete=models.SET_NULL)),
                ('plan', models.ForeignKey(null=True, to='gracopumpapp.Plan', on_delete=django.db.models.deletion.SET_NULL, blank=True)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.AddField(
            model_name='invoice',
            name='account',
            field=models.ForeignKey(to='gracopumpapp.PaymentAccount', on_delete=models.SET_NULL),
            preserve_default=True,
        ),
    ]
