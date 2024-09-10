'''
Backend for Mailgun

Based on django_mailgun, which appears to no longer be supported. Modified for python 3.3

License:
Copyright (c) 2011 Bradley Whittington
 
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
 
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

'''

from django.conf import settings
from django.core.mail.backends.base import BaseEmailBackend
from django.core.mail.message import sanitize_address
import requests


try:
    from io import StringIO
except ImportError:
    try:
        from cStringIO import StringIO
    except ImportError:
        from StringIO import StringIO


class MailgunAPIError(Exception):
    pass


class MailgunBackend(BaseEmailBackend):
    """A Django Email backend that uses mailgun.
    """

    def __init__(self, fail_silently=False, *args, **kwargs):
        access_key, server_name = (kwargs.pop('access_key', None),
                                   kwargs.pop('server_name', None))

        super(MailgunBackend, self).__init__(
                        fail_silently=fail_silently,
                        *args, **kwargs)

        try:
            self._access_key = access_key or getattr(settings, 'MAILGUN_ACCESS_KEY')
            self._server_name = server_name or getattr(settings, 'MAILGUN_SERVER_NAME')
        except AttributeError:
            if fail_silently:
                self._access_key, self._server_name = None
            else:
                raise

        self._api_url = "https://api.mailgun.net/v2/%s/" % self._server_name

    def open(self):
        """Stub for open connection, all sends are done over HTTP POSTs
        """
        pass

    def close(self):
        """Close any open HTTP connections to the API server.
        """
        pass

    def _send(self, email_message):
        """A helper method that does the actual sending."""
        if not email_message.recipients():
            return False
        from_email = sanitize_address(email_message.from_email, email_message.encoding)
        recipients = [sanitize_address(addr, email_message.encoding)
                      for addr in email_message.recipients()]

        try:
            r = requests.\
                post(self._api_url + "messages.mime",
                     auth=("api", self._access_key),
                     data={
                            "to": ", ".join(recipients),
                            "from": from_email,
                         },
                     files={
                            "message": StringIO(email_message.message().as_string()),
                         }
                     )
        except:
            if not self.fail_silently:
                raise
            return False

        if r.status_code != 200:
            if not self.fail_silently:
                raise MailgunAPIError(r)
            return False

        return True

    def send_messages(self, email_messages):
        """Sends one or more EmailMessage objects and returns the number of
        email messages sent.
        """
        if not email_messages:
            return

        num_sent = 0
        for message in email_messages:
            if self._send(message):
                num_sent += 1

        return num_sent
