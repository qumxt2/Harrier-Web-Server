from django.http import QueryDict
from django.http.multipartparser import MultiValueDict
from django.utils.deprecation import MiddlewareMixin
from django.utils import timezone
import pytz


class TimezoneMiddleware(MiddlewareMixin):
    def process_request(self, request):
        tzname = request.session.get('django_timezone')
        if tzname:
            timezone.activate(pytz.timezone(tzname))
        else:
            timezone.deactivate()


class UpdateLastActivityMiddleware(MiddlewareMixin):
    """Log a user's last activity on the site"""

    def process_view(self, request, view_func, view_args, view_kwargs):
        assert hasattr(request, 'user'), 'The UpdateLastActivityMiddleware requires authentication middleware to be installed.'
        if request.user.is_authenticated():
            request.user.userprofile.last_activity = timezone.now()
            request.user.userprofile.save()


# Slightly modified from https://gist.github.com/g00fy-/1161423
class RESTMiddleware(MiddlewareMixin):
    def process_request(self, request):
        request.PUT = QueryDict('')
        method = request.META.get('REQUEST_METHOD', '').upper()
        if method == 'PUT':
            self.handle_PUT(request)

    def handle_DELETE(self, request):
        request.DELETE, request._files = self.parse_request(request)

    def handle_PUT(self, request):
        request.PUT, request._files = self.parse_request(request)

    def parse_request(self, request):
        if request.META.get('CONTENT_TYPE', '').startswith('multipart'):
            return self.parse_multipart(request)
        else:
            return (self.parse_form(request), MultiValueDict())

    def parse_form(self, request):
        return QueryDict(request.body)

    def parse_multipart(self, request):
        return request.parse_file_upload(request.META, request)


class OverrideIECompatibilityView(MiddlewareMixin):
    """Override the Internet Explorer compatibility view. Apparently, this must be done at the
    header level if "use compatibility view for all sites" is selected, hence this code"""

    def process_response(self, request, response):

        # Allow this header to be overridden
        if response.get('X-UA-Compatible', None) is not None:
            return response

        response['X-UA-Compatible'] = 'IE=edge'

        return response
