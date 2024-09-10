from functools import wraps
import re

from django.contrib import messages
from django.urls import reverse
from django.shortcuts import redirect
from django.utils.cache import patch_cache_control
from gracopumpapp.models import TermsOfService, UserProfile


def check_tos(function):
    """Check if the user has not agreed to the TOS terms or has agreed only to old TOS terms.
    If so, force the user to agree to the current TOS terms"""

    def decorator(view_func):
        @wraps(view_func)
        def _wrapped_view(request, *args, **kwargs):
            need_tos = False

            newest_tos = TermsOfService.objects.order_by('id').last()
            user_tos = None

            if request.user and not request.user.is_anonymous():
                user_tos = request.user.userprofile.tos_agreed

            if newest_tos is not None and (user_tos is None or user_tos != newest_tos):
                need_tos = True

            if not need_tos:
                return view_func(request, *args, **kwargs)
            else:
                return redirect('user_tos')

        return _wrapped_view

    if function:
        return decorator(function)
    else:
        return decorator


def check_email(function):
    """Check if the user has an email address set up"""

    def decorator(view_func):
        @wraps(view_func)
        def _wrapped_view(request, *args, **kwargs):
            if request.user and not request.user.is_anonymous():
                has_email = re.match(UserProfile.EMAIL_REGEX, request.user.email)

            if not has_email:
                verification_url = reverse('user_settings_default', args=[])
                messages.warning(request, 'You do not have a valid email address associated with your account. Please set one on your <a href="%s">Account Settings</a> page.' % (verification_url,), extra_tags='safe')

            return view_func(request, *args, **kwargs)

        return _wrapped_view

    if function:
        return decorator(function)
    else:
        return decorator


def really_defeat_caching(function):
    """Really, really defeat caching. The built-in Django methods are insufficient"""

    def decorator(view_func):
        @wraps(view_func)
        def _wrapped_view(request, *args, **kwargs):
            response = view_func(request, *args, **kwargs)
            cache_opts = {'no_cache': True,
                          'no_store': True,
                          'must_revalidate': True,
                          'private': True,
                          }
            patch_cache_control(response, **cache_opts)
            response['Pragma'] = 'no-cache'
            response['Expires'] = '0'
            return response

        return _wrapped_view

    if function:
        return decorator(function)
    else:
        return decorator
