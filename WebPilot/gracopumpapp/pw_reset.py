"""
Module to tweak the behavior of Django's password reset module
"""
from django.contrib.auth.forms import SetPasswordForm
from django.core.exceptions import ValidationError
from gracopumpapp.models import UserProfile


class SetPasswordWithRestrictionsForm(SetPasswordForm):
    """
    Inherited form that lets a user change set his/her password without
    entering the old password while validating password requirements
    """
    def clean_new_password1(self):
        password1 = self.cleaned_data.get('new_password1')
        if not UserProfile.check_password_requirements(password1):
            error_reason = UserProfile.check_password_requirements(password1, return_reason=True)
            raise ValidationError(error_reason)
        return password1
