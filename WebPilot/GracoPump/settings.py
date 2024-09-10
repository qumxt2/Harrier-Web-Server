"""
Django settings for GracoPump project.

For more information on this file, see
https://docs.djangoproject.com/en/1.7/topics/settings/

For the full list of settings and their values, see
https://docs.djangoproject.com/en/1.7/ref/settings/
"""

# Build paths inside the project like this: os.path.join(BASE_DIR, ...)
import os

from django.conf.global_settings import STATICFILES_DIRS, LOGIN_REDIRECT_URL, \
    EMAIL_BACKEND, SESSION_COOKIE_SECURE, \
    SESSION_COOKIE_AGE, SESSION_SAVE_EVERY_REQUEST
import recurly

BASE_DIR = os.path.dirname(os.path.dirname(__file__))


# Quick-start development settings - unsuitable for production
# See https://docs.djangoproject.com/en/1.7/howto/deployment/checklist/

# SECURITY WARNING: keep the secret key used in production secret!
SECRET_KEY = 'o7g&*2dj05)h2ptziw%x#_iahzgf(%dvomsq1h9%wkg@0^dxy5'

# SECURITY WARNING: don't run with debug turned on in production!
if os.name == 'nt':
    DEBUG = True
else:
    DEBUG = False

ALLOWED_HOSTS = ['.graniteriver.com', '.graco.com',]


# Application definition

INSTALLED_APPS = (
    'django.contrib.admin',
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.messages',
    'django.contrib.staticfiles',
    'gracopumpapp'
)

MIDDLEWARE = (
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
    # 'django.contrib.auth.middleware.SessionAuthenticationMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware',
    'django.middleware.clickjacking.XFrameOptionsMiddleware',
    'gracopumpapp.middleware.TimezoneMiddleware',
    'gracopumpapp.middleware.UpdateLastActivityMiddleware',
    'gracopumpapp.middleware.RESTMiddleware',
    'gracopumpapp.middleware.OverrideIECompatibilityView',
)

ROOT_URLCONF = 'GracoPump.urls'

WSGI_APPLICATION = 'GracoPump.wsgi.application'


TEMPLATES = [
    {
        'BACKEND': 'django.template.backends.django.DjangoTemplates',
        'DIRS': [
            # insert your TEMPLATE_DIRS here
        ],
        'APP_DIRS': True,
        'OPTIONS': {
            'context_processors': [
                # Insert your TEMPLATE_CONTEXT_PROCESSORS here or use this
                # list if you haven't customized them:
                "gracopumpapp.context_processors.is_admin",
                "gracopumpapp.context_processors.is_distributor",
                "gracopumpapp.context_processors.get_version",
                "gracopumpapp.context_processors.is_logged_in",
                'django.contrib.auth.context_processors.auth',
                'django.template.context_processors.debug',
                'django.template.context_processors.i18n',
                'django.template.context_processors.media',
                'django.template.context_processors.static',
                'django.template.context_processors.tz',
                'django.contrib.messages.context_processors.messages',
            ],
        },
    },
]

LOGIN_REDIRECT_URL = '/pump/list'
LOGIN_URL = '/'
LOGOUT_URL = '/logout'

# Database
# https://docs.djangoproject.com/en/1.7/ref/settings/#databases

if DEBUG:
    DATABASES = {
        'default': {
            'ENGINE': 'django.db.backends.sqlite3',
            'NAME': os.path.join(BASE_DIR, 'db.sqlite3'),
        }
    }
else:
    DATABASES = {
        'default': {
            'NAME': 'gracopump',
            'ENGINE': 'django.db.backends.mysql',
            'USER': 'gracoweb',
            'PASSWORD': 'lKJSDfkljhkwjQ$2',
            'OPTIONS': {
                'autocommit': True,
                'init_command': "SET sql_mode='STRICT_TRANS_TABLES'",
            },
        },
    }

# Internationalization
# https://docs.djangoproject.com/en/1.7/topics/i18n/

LANGUAGE_CODE = 'en-us'

TIME_ZONE = 'UTC'

USE_I18N = True

USE_L10N = True

USE_TZ = True

# Email setup (requires django-mailgun)
EMAIL_BACKEND = 'gracopumpapp.email_processors.MailgunBackend'

# These are DEFAULTS and are likely overriden in the local_settings.py file
MAILGUN_ACCESS_KEY = 'key-60ff7fea95846460bd9140afda6bce23'
MAILGUN_SERVER_NAME = 'harrier.graco.com'
EMAIL_ORIGIN_ADDRESS = 'oilandgas@graco.com'

if not DEBUG:
    SESSION_COOKIE_SECURE = True
    CSRF_COOKIE_SECURE = True

CSRF_FAILURE_VIEW = 'gracopumpapp.views.csrf_failure'

# Default session timeout is two weeks; change it to a week of inactivity
# Never mind the performance hit from writing the db on every request, since
# we're already doing that to track the last user activity time
SESSION_COOKIE_AGE = 86400 * 7
SESSION_SAVE_EVERY_REQUEST = True

# Static files (CSS, JavaScript, Images)
# https://docs.djangoproject.com/en/1.7/howto/static-files/

STATIC_URL = '/static/'

STATICFILES_DIRS += (os.path.join(BASE_DIR, 'GracoPump', 'static'),)

# Recurly
recurly.SUBDOMAIN = 'gracoharrier'
recurly.API_KEY = '2080dd597f2d42c281545e1bb9440d8f'
recurly.DEFAULT_CURRENCY = 'USD'

'''
Work around Aeris API issue, but only relevant to the server. The Aeris server uses a weak DH prime, so we
need to tell the API client not to even try DH
'''
if os.name != 'nt':
    import urllib3
    urllib3.util.ssl_.DEFAULT_CIPHERS += ':!DH:!aNULL'

try:
    from GracoPump.local_settings import *
except ImportError:
    pass
