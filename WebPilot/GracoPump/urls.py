from GracoPump.settings import EMAIL_ORIGIN_ADDRESS
import django
from django.urls import include, path
from django.contrib.auth import views as da
from gracopumpapp import views, api
import gracopumpapp
from gracopumpapp.pw_reset import SetPasswordWithRestrictionsForm

urlpatterns = [
    path('', views.index, name='index'),
    path('login/', views._login, name='login'),

    path('admin/', views.admin, name='admin'),
    path('admin/events/', views.admin_event_log, name='admin_event_log'),
    path('admin/history/download/', views.admin_history_log_download, name='admin_history_log_download'),
    path('admin/testing', views.admin_testing, name='admin_testing'),
    path('admin/database_backup_download/', views.admin_database_backup_download, name='admin_database_backup_download'),

    path('user/list/', views.user_list, name='user_list'),
    path('user/settings/<user_id>/alarmprefs/', views.user_alarm_prefs, name='user_alarm_prefs'),
    path('user/settings/<user_id>/', views.user_settings, name='user_settings'),
    path('user/settings/', views.user_settings, name='user_settings_default'),
    path('user/create/', views.user_create, name='user_create'),
    path('user/register/', views.user_register, name='user_register'),
    path('user/register/done/', views.user_register_done, name='user_register_done'),
    path('user/verify/register/(<verification_key>/', views.user_register_verify, name='user_register_verify'),
    path('user/verify/email/<verification_key>)/', views.user_email_verify, name='user_email_verify'),
    path('user/verify/resend/', views.user_verification_resend, name='user_verification_resend'),
    path('user/invitation/<invitation_code>/', views.user_invitation, name='user_invitation'),
    path('user/invitation/register/<invitation_code>/', views.user_invitation_register, name='user_invitation_register'),
    path('user/tos/', views.user_tos, name='user_tos'),

    path('group/list/', views.customer_list, name='customer_list'),
    path('group/settings/<customer_id>/', views.customer_settings, name='customer_settings'),
    path('group/settings/', views.customer_settings, name='customer_settings_default'),
    path('group/users/add/<customer_id>/', views.customer_user_add, name='customer_user_add'),
    path('group/users/<customer_id>/', views.customer_users, name='customer_users'),
    path('group/users/', views.customer_users, name='customer_users_default'),
    path('group/pumps/add/<customer_id>/', views.customer_pump_add, name='customer_pump_add'),
    path('group/pumps/<customer_id>/', views.customer_pumps, name='customer_pumps'),
    path('group/pumps/', views.customer_pumps, name='customer_pumps_default'),
    path('group/create/', views.customer_create, name='customer_create'),
    path('group/invite/<customer_id>/', views.customer_invite, name='customer_invite'),
    path('group/subscription/<customer_id>/', views.customer_subscription, name='customer_subscription'),
    path('group/subscription/config/<customer_id>/', views.customer_subscription_config, name='customer_subscription_config'),
    path('group/subscription/invoices/<customer_id>/', views.customer_subscription_invoices, name='customer_subscription_invoices'),

    path('invoice/<invoice_id>.pdf', views.invoice_pdf, name='invoice_pdf'),

    path('pump/list/', views.pump_list, name='pump_list'),
    path('pump/details/<pump_id>/', views.pump_details, name='pump_details'),
    path('pump/details/', views.pump_details, name='pump_details_default'),
    path('pump/alarm_custom/<pump_id>/', views.pump_alarm_custom, name='pump_alarm_custom'),
    path('pump/history/download/<pump_id>/', views.pump_history_download, name='pump_history_download'),
    path('pump/history/<pump_id>/', views.pump_history, name='pump_history'),
    path('pump/aeris/<pump_id>/', views.pump_aeris, name='pump_aeris'),
    path('pump/pump_flow_rates/<pump_id>/', views.pump_multiwell_flow_rates, name='pump_flow_rates'),
    path('pump/pump_totalizers/<pump_id>/', views.pump_multiwell_totalizers, name='pump_totalizers'),
    path('pump/pump_injection_point_custom/<pump_id>/', views.pump_injection_point_custom, name='pump_injection_point_custom'),
    path('pump/analog_in_settings/<pump_id>/', views.analog_in_settings, name='analog_in_settigs'),

    path('help/', views.help_page, name='help'),
    path('logout/', views._logout, name='logout'),

    path('notification/list/', views.notification_list, name='notification_list_all'),
    path('notification/pump/<pump_id>/', views.notification_list, name='notification_pump'),
    path('notification/pump/', views.notification_list, name='notification_pump_default'),
    path('notification/user/<user_id>/', views.notification_list, name='notification_user'),
    path('notification/create/pump/(<pump_id>/', views.notification_create, name='notification_create'),

    path('cron/', views.cron, name='cron'),

    path('recurly_webhook/', views.recurly_webhook, name='recurly_webhook'),
]

# API 1.0
urlpatterns += [
    path('api/v1.0/csrf/', api.csrf_api_1_0, name='csrf_api_1_0'),

    path('api/v1.0/admin/', api.admin_api_1_0, name='admin_api_1_0'),

    path('api/v1.0/events/', api.events_api_1_0, name='events_api_1_0'),

    path('api/v1.0/login/', api.login_api_1_0, name='login_api_1_0'),
    path('api/v1.0/logout/', api.logout_api_1_0, name='logout_api_1_0'),

    path('api/v1.0/mqttauth/', api.mqtt_auth_api_1_0, name='mqtt_auth_api_1_0'),

    path('api/v1.0/users/', api.users_api_1_0, name='users_api_1_0'),
    path('api/v1.0/users/(<user_id>/', api.users_api_1_0, name='users_spec_api_1_0'),
    path('api/v1.0/users/(<user_id>/notifications/', api.notifications_api_1_0, name='notifications_spec_user_api_1_0'),

    path('api/v1.0/customers/', api.customers_api_1_0, name='customers_api_1_0'),
    path('api/v1.0/customers/<customer_id>/', api.customers_api_1_0, name='customers_spec_api_1_0'),
    path('api/v1.0/customers/<customer_id>/pumps/', api.pumps_api_1_0, name='pumps_customer_api_1_0'),
    path('api/v1.0/customers/<customer_id>/users/', api.users_api_1_0, name='users_customer_api_1_0'),

    path('api/v1.0/payments/', api.payments_api_1_0, name='payments_api_1_0'),
    path('api/v1.0/payments/<payment_id>/', api.payments_api_1_0, name='payments_spec_api_1_0'),

    path('api/v1.0/pumps/', api.pumps_api_1_0, name='pumps_api_1_0'),
    path('api/v1.0/pumps/<pump_id>/', api.pumps_api_1_0, name='pumps_spec_api_1_0'),
    path('api/v1.0/pumps/<pump_id>/history/', api.pumps_history_api_1_0, name='pumps_history_api_1_0'),
    path('api/v1.0/pumps/<pump_id>/notifications/', api.notifications_api_1_0, name='notifications_spec_pump_api_1_0'),

    path('api/v1.0/aeris/', api.aeris_api_1_0, name='aeris_api_1_0'),
    path('api/v1.0/aeris/<pump_id>/', api.aeris_api_1_0, name='aeris_spec_api_1_0'),

    path('api/v1.0/notifications/', api.notifications_api_1_0, name='notifications_api_1_0'),
    path('api/v1.0/notifications/<notification_id>/', api.notifications_api_1_0, name='notifications_spec_notif_api_1_0'),

    path('api/v1.0/registration/', api.registration_api_1_0, name='registration_api_1_0'),

]

# Password reset
urlpatterns += [ 
    path('password_reset/',
        da.PasswordResetView.as_view(),
        {'post_reset_redirect': '/password_reset/mailed/',
         'template_name': 'pwreset_form.html',
         'subject_template_name': 'emails/pwreset_subject.txt',
         'email_template_name': 'emails/pwreset_email.txt',
         'html_email_template_name': 'emails/pwreset_email.html',
         'from_email': 'Graco <%s>' % EMAIL_ORIGIN_ADDRESS,
         },
        name="password_reset"
        ),
    path('password_reset/mailed/',
        da.PasswordResetDoneView.as_view(),
        {'template_name': 'pwreset_done.html'},
        name='pw_reset_mailed',
        ),
    path('accounts/password_reset/<uidb64>-<token>/',
        da.PasswordResetConfirmView.as_view(),
        {'post_reset_redirect': '/password_reset/complete/',
         'template_name': 'pwreset_confirm.html',
         'set_password_form': SetPasswordWithRestrictionsForm, },
        name='pw_reset_confirm',
        ),
    path('password_reset/complete/',
        da.PasswordResetCompleteView.as_view(),
        {'template_name': 'pwreset_complete.html'},
        name='pw_reset_complete',   
        ),
    ]

# Error pages

handler401 = gracopumpapp.views.handler401
handler403 = gracopumpapp.views.handler403
handler404 = gracopumpapp.views.handler404
handler500 = gracopumpapp.views.handler500
