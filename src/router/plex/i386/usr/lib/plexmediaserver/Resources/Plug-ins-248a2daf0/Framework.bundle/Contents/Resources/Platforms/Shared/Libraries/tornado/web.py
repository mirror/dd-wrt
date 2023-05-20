#!/usr/bin/env python
#
# Copyright 2009 Facebook
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

"""The Tornado web framework.

The Tornado web framework looks a bit like web.py (http://webpy.org/) or
Google's webapp (http://code.google.com/appengine/docs/python/tools/webapp/),
but with additional tools and optimizations to take advantage of the
Tornado non-blocking web server and tools.

Here is the canonical "Hello, world" example app:

    import tornado.httpserver
    import tornado.ioloop
    import tornado.web

    class MainHandler(tornado.web.RequestHandler):
        def get(self):
            self.write("Hello, world")

    if __name__ == "__main__":
        application = tornado.web.Application([
            (r"/", MainHandler),
        ])
        http_server = tornado.httpserver.HTTPServer(application)
        http_server.listen(8888)
        tornado.ioloop.IOLoop.instance().start()

See the Tornado walkthrough on GitHub for more details and a good
getting started guide.
"""

from __future__ import with_statement

import Cookie
import base64
import binascii
import cStringIO
import calendar
import contextlib
import datetime
import email.utils
import functools
import gzip
import hashlib
import hmac
import httplib
import logging
import mimetypes
import os.path
import re
import stat
import sys
import time
import types
import urllib
import urlparse
import uuid

from tornado import escape
from tornado import locale
from tornado import stack_context
from tornado import template

class RequestHandler(object):
    """Subclass this class and define get() or post() to make a handler.

    If you want to support more methods than the standard GET/HEAD/POST, you
    should override the class variable SUPPORTED_METHODS in your
    RequestHandler class.
    """
    SUPPORTED_METHODS = ("GET", "HEAD", "POST", "DELETE", "PUT")

    def __init__(self, application, request, **kwargs):
        self.application = application
        self.request = request
        self._headers_written = False
        self._finished = False
        self._auto_finish = True
        self._transforms = None  # will be set in _execute
        self.ui = _O((n, self._ui_method(m)) for n, m in
                     application.ui_methods.iteritems())
        self.ui["modules"] = _O((n, self._ui_module(n, m)) for n, m in
                                application.ui_modules.iteritems())
        self.clear()
        # Check since connection is not available in WSGI
        if hasattr(self.request, "connection"):
            self.request.connection.stream.set_close_callback(
                self.on_connection_close)
        self.initialize(**kwargs)

    def initialize(self):
        """Hook for subclass initialization.

        A dictionary passed as the third argument of a url spec will be
        supplied as keyword arguments to initialize().

        Example:
            class ProfileHandler(RequestHandler):
                def initialize(self, database):
                    self.database = database

                def get(self, username):
                    ...

            app = Application([
                (r'/user/(.*)', ProfileHandler, dict(database=database)),
                ])
        """
        pass

    @property
    def settings(self):
        return self.application.settings

    def head(self, *args, **kwargs):
        raise HTTPError(405)

    def get(self, *args, **kwargs):
        raise HTTPError(405)

    def post(self, *args, **kwargs):
        raise HTTPError(405)

    def delete(self, *args, **kwargs):
        raise HTTPError(405)

    def put(self, *args, **kwargs):
        raise HTTPError(405)

    def prepare(self):
        """Called before the actual handler method.

        Useful to override in a handler if you want a common bottleneck for
        all of your requests.
        """
        pass

    def on_connection_close(self):
        """Called in async handlers if the client closed the connection.

        You may override this to clean up resources associated with
        long-lived connections.

        Note that the select()-based implementation of IOLoop does not detect
        closed connections and so this method will not be called until
        you try (and fail) to produce some output.  The epoll- and kqueue-
        based implementations should detect closed connections even while
        the request is idle.

        Proxies may keep a connection open for a time (perhaps
        indefinitely) after the client has gone away, so this method
        may not be called promptly after the end user closes their
        connection.
        """
        pass

    def clear(self):
        """Resets all headers and content for this response."""
        self._headers = {
            "Server": "TornadoServer/1.0",
            "Content-Type": "text/html; charset=UTF-8",
        }
        if not self.request.supports_http_1_1():
            if self.request.headers.get("Connection") == "Keep-Alive":
                self.set_header("Connection", "Keep-Alive")
        self._write_buffer = []
        self._status_code = 200

    def set_status(self, status_code):
        """Sets the status code for our response."""
        assert status_code in httplib.responses
        self._status_code = status_code

    def set_header(self, name, value):
        """Sets the given response header name and value.

        If a datetime is given, we automatically format it according to the
        HTTP specification. If the value is not a string, we convert it to
        a string. All header values are then encoded as UTF-8.
        """
        if isinstance(value, datetime.datetime):
            t = calendar.timegm(value.utctimetuple())
            value = email.utils.formatdate(t, localtime=False, usegmt=True)
        elif isinstance(value, int) or isinstance(value, long):
            value = str(value)
        else:
            value = _utf8(value)
            # If \n is allowed into the header, it is possible to inject
            # additional headers or split the request. Also cap length to
            # prevent obviously erroneous values.
            safe_value = re.sub(r"[\x00-\x1f]", " ", value)[:4000]
            if safe_value != value:
                raise ValueError("Unsafe header value %r", value)
        self._headers[name] = value

    _ARG_DEFAULT = []
    def get_argument(self, name, default=_ARG_DEFAULT, strip=True):
        """Returns the value of the argument with the given name.

        If default is not provided, the argument is considered to be
        required, and we throw an HTTP 404 exception if it is missing.

        If the argument appears in the url more than once, we return the
        last value.

        The returned value is always unicode.
        """
        args = self.get_arguments(name, strip=strip)
        if not args:
            if default is self._ARG_DEFAULT:
                raise HTTPError(404, "Missing argument %s" % name)
            return default
        return args[-1]

    def get_arguments(self, name, strip=True):
        """Returns a list of the arguments with the given name.

        If the argument is not present, returns an empty list.

        The returned values are always unicode.
        """
        values = self.request.arguments.get(name, [])
        # Get rid of any weird control chars
        values = [re.sub(r"[\x00-\x08\x0e-\x1f]", " ", x) for x in values]
        values = [_unicode(x) for x in values]
        if strip:
            values = [x.strip() for x in values]
        return values


    @property
    def cookies(self):
        """A dictionary of Cookie.Morsel objects."""
        if not hasattr(self, "_cookies"):
            self._cookies = Cookie.BaseCookie()
            if "Cookie" in self.request.headers:
                try:
                    self._cookies.load(self.request.headers["Cookie"])
                except:
                    self.clear_all_cookies()
        return self._cookies

    def get_cookie(self, name, default=None):
        """Gets the value of the cookie with the given name, else default."""
        if name in self.cookies:
            return self.cookies[name].value
        return default

    def set_cookie(self, name, value, domain=None, expires=None, path="/",
                   expires_days=None, **kwargs):
        """Sets the given cookie name/value with the given options.

        Additional keyword arguments are set on the Cookie.Morsel
        directly.
        See http://docs.python.org/library/cookie.html#morsel-objects
        for available attributes.
        """
        name = _utf8(name)
        value = _utf8(value)
        if re.search(r"[\x00-\x20]", name + value):
            # Don't let us accidentally inject bad stuff
            raise ValueError("Invalid cookie %r: %r" % (name, value))
        if not hasattr(self, "_new_cookies"):
            self._new_cookies = []
        new_cookie = Cookie.BaseCookie()
        self._new_cookies.append(new_cookie)
        new_cookie[name] = value
        if domain:
            new_cookie[name]["domain"] = domain
        if expires_days is not None and not expires:
            expires = datetime.datetime.utcnow() + datetime.timedelta(
                days=expires_days)
        if expires:
            timestamp = calendar.timegm(expires.utctimetuple())
            new_cookie[name]["expires"] = email.utils.formatdate(
                timestamp, localtime=False, usegmt=True)
        if path:
            new_cookie[name]["path"] = path
        for k, v in kwargs.iteritems():
            new_cookie[name][k] = v

    def clear_cookie(self, name, path="/", domain=None):
        """Deletes the cookie with the given name."""
        expires = datetime.datetime.utcnow() - datetime.timedelta(days=365)
        self.set_cookie(name, value="", path=path, expires=expires,
                        domain=domain)

    def clear_all_cookies(self):
        """Deletes all the cookies the user sent with this request."""
        for name in self.cookies.iterkeys():
            self.clear_cookie(name)

    def set_secure_cookie(self, name, value, expires_days=30, **kwargs):
        """Signs and timestamps a cookie so it cannot be forged.

        You must specify the 'cookie_secret' setting in your Application
        to use this method. It should be a long, random sequence of bytes
        to be used as the HMAC secret for the signature.

        To read a cookie set with this method, use get_secure_cookie().
        """
        timestamp = str(int(time.time()))
        value = base64.b64encode(value)
        signature = self._cookie_signature(name, value, timestamp)
        value = "|".join([value, timestamp, signature])
        self.set_cookie(name, value, expires_days=expires_days, **kwargs)

    def get_secure_cookie(self, name, include_name=True, value=None):
        """Returns the given signed cookie if it validates, or None.

        In older versions of Tornado (0.1 and 0.2), we did not include the
        name of the cookie in the cookie signature. To read these old-style
        cookies, pass include_name=False to this method. Otherwise, all
        attempts to read old-style cookies will fail (and you may log all
        your users out whose cookies were written with a previous Tornado
        version).
        """
        if value is None: value = self.get_cookie(name)
        if not value: return None
        parts = value.split("|")
        if len(parts) != 3: return None
        if include_name:
            signature = self._cookie_signature(name, parts[0], parts[1])
        else:
            signature = self._cookie_signature(parts[0], parts[1])
        if not _time_independent_equals(parts[2], signature):
            logging.warning("Invalid cookie signature %r", value)
            return None
        timestamp = int(parts[1])
        if timestamp < time.time() - 31 * 86400:
            logging.warning("Expired cookie %r", value)
            return None
        if timestamp > time.time() + 31 * 86400:
            # _cookie_signature does not hash a delimiter between the
            # parts of the cookie, so an attacker could transfer trailing
            # digits from the payload to the timestamp without altering the
            # signature.  For backwards compatibility, sanity-check timestamp
            # here instead of modifying _cookie_signature.
            logging.warning("Cookie timestamp in future; possible tampering %r", value)
            return None
        if parts[1].startswith("0"):
            logging.warning("Tampered cookie %r", value)
        try:
            return base64.b64decode(parts[0])
        except:
            return None

    def _cookie_signature(self, *parts):
        self.require_setting("cookie_secret", "secure cookies")
        hash = hmac.new(self.application.settings["cookie_secret"],
                        digestmod=hashlib.sha1)
        for part in parts: hash.update(part)
        return hash.hexdigest()

    def redirect(self, url, permanent=False):
        """Sends a redirect to the given (optionally relative) URL."""
        if self._headers_written:
            raise Exception("Cannot redirect after headers have been written")
        self.set_status(301 if permanent else 302)
        # Remove whitespace
        url = re.sub(r"[\x00-\x20]+", "", _utf8(url))
        self.set_header("Location", urlparse.urljoin(self.request.uri, url))
        self.finish()

    def write(self, chunk):
        """Writes the given chunk to the output buffer.

        To write the output to the network, use the flush() method below.

        If the given chunk is a dictionary, we write it as JSON and set
        the Content-Type of the response to be text/javascript.
        """
        assert not self._finished
        if isinstance(chunk, dict):
            chunk = escape.json_encode(chunk)
            self.set_header("Content-Type", "text/javascript; charset=UTF-8")
        chunk = _utf8(chunk)
        self._write_buffer.append(chunk)

    def render(self, template_name, **kwargs):
        """Renders the template with the given arguments as the response."""
        html = self.render_string(template_name, **kwargs)

        # Insert the additional JS and CSS added by the modules on the page
        js_embed = []
        js_files = []
        css_embed = []
        css_files = []
        html_heads = []
        html_bodies = []
        for module in getattr(self, "_active_modules", {}).itervalues():
            embed_part = module.embedded_javascript()
            if embed_part: js_embed.append(_utf8(embed_part))
            file_part = module.javascript_files()
            if file_part:
                if isinstance(file_part, basestring):
                    js_files.append(file_part)
                else:
                    js_files.extend(file_part)
            embed_part = module.embedded_css()
            if embed_part: css_embed.append(_utf8(embed_part))
            file_part = module.css_files()
            if file_part:
                if isinstance(file_part, basestring):
                    css_files.append(file_part)
                else:
                    css_files.extend(file_part)
            head_part = module.html_head()
            if head_part: html_heads.append(_utf8(head_part))
            body_part = module.html_body()
            if body_part: html_bodies.append(_utf8(body_part))
        if js_files:
            # Maintain order of JavaScript files given by modules
            paths = []
            unique_paths = set()
            for path in js_files:
                if not path.startswith("/") and not path.startswith("http:"):
                    path = self.static_url(path)
                if path not in unique_paths:
                    paths.append(path)
                    unique_paths.add(path)
            js = ''.join('<script src="' + escape.xhtml_escape(p) +
                         '" type="text/javascript"></script>'
                         for p in paths)
            sloc = html.rindex('</body>')
            html = html[:sloc] + js + '\n' + html[sloc:]
        if js_embed:
            js = '<script type="text/javascript">\n//<![CDATA[\n' + \
                '\n'.join(js_embed) + '\n//]]>\n</script>'
            sloc = html.rindex('</body>')
            html = html[:sloc] + js + '\n' + html[sloc:]
        if css_files:
            paths = set()
            for path in css_files:
                if not path.startswith("/") and not path.startswith("http:"):
                    paths.add(self.static_url(path))
                else:
                    paths.add(path)
            css = ''.join('<link href="' + escape.xhtml_escape(p) + '" '
                          'type="text/css" rel="stylesheet"/>'
                          for p in paths)
            hloc = html.index('</head>')
            html = html[:hloc] + css + '\n' + html[hloc:]
        if css_embed:
            css = '<style type="text/css">\n' + '\n'.join(css_embed) + \
                '\n</style>'
            hloc = html.index('</head>')
            html = html[:hloc] + css + '\n' + html[hloc:]
        if html_heads:
            hloc = html.index('</head>')
            html = html[:hloc] + ''.join(html_heads) + '\n' + html[hloc:]
        if html_bodies:
            hloc = html.index('</body>')
            html = html[:hloc] + ''.join(html_bodies) + '\n' + html[hloc:]
        self.finish(html)

    def render_string(self, template_name, **kwargs):
        """Generate the given template with the given arguments.

        We return the generated string. To generate and write a template
        as a response, use render() above.
        """
        # If no template_path is specified, use the path of the calling file
        template_path = self.get_template_path()
        if not template_path:
            frame = sys._getframe(0)
            web_file = frame.f_code.co_filename
            while frame.f_code.co_filename == web_file:
                frame = frame.f_back
            template_path = os.path.dirname(frame.f_code.co_filename)
        if not getattr(RequestHandler, "_templates", None):
            RequestHandler._templates = {}
        if template_path not in RequestHandler._templates:
            loader = self.application.settings.get("template_loader") or\
              template.Loader(template_path)
            RequestHandler._templates[template_path] = loader
        t = RequestHandler._templates[template_path].load(template_name)
        args = dict(
            handler=self,
            request=self.request,
            current_user=self.current_user,
            locale=self.locale,
            _=self.locale.translate,
            static_url=self.static_url,
            xsrf_form_html=self.xsrf_form_html,
            reverse_url=self.application.reverse_url
        )
        args.update(self.ui)
        args.update(kwargs)
        return t.generate(**args)

    def flush(self, include_footers=False):
        """Flushes the current output buffer to the network."""
        if self.application._wsgi:
            raise Exception("WSGI applications do not support flush()")

        chunk = "".join(self._write_buffer)
        self._write_buffer = []
        if not self._headers_written:
            self._headers_written = True
            for transform in self._transforms:
                self._headers, chunk = transform.transform_first_chunk(
                    self._headers, chunk, include_footers)
            headers = self._generate_headers()
        else:
            for transform in self._transforms:
                chunk = transform.transform_chunk(chunk, include_footers)
            headers = ""

        # Ignore the chunk and only write the headers for HEAD requests
        if self.request.method == "HEAD":
            if headers: self.request.write(headers)
            return

        if headers or chunk:
            self.request.write(headers + chunk)

    def finish(self, chunk=None):
        """Finishes this response, ending the HTTP request."""
        assert not self._finished
        if chunk is not None: self.write(chunk)

        # Automatically support ETags and add the Content-Length header if
        # we have not flushed any content yet.
        if not self._headers_written:
            if "Content-Length" not in self._headers:
                content_length = sum(len(part) for part in self._write_buffer)
                self.set_header("Content-Length", content_length)

        if hasattr(self.request, "connection"):
            # Now that the request is finished, clear the callback we
            # set on the IOStream (which would otherwise prevent the
            # garbage collection of the RequestHandler when there
            # are keepalive connections)
            self.request.connection.stream.set_close_callback(None)

        if not self.application._wsgi:
            self.flush(include_footers=True)
            self.request.finish()
            self._log()
        self._finished = True

    def send_error(self, status_code=500, **kwargs):
        """Sends the given HTTP error code to the browser.

        We also send the error HTML for the given error code as returned by
        get_error_html. Override that method if you want custom error pages
        for your application.
        """
        if self._headers_written:
            logging.error("Cannot send error response after headers written")
            if not self._finished:
                self.finish()
            return
        self.clear()
        self.set_status(status_code)
        message = self.get_error_html(status_code, **kwargs)
        self.finish(message)

    def get_error_html(self, status_code, **kwargs):
        """Override to implement custom error pages.

        If this error was caused by an uncaught exception, the
        exception object can be found in kwargs e.g. kwargs['exception']
        """
        return "<html><title>%(code)d: %(message)s</title>" \
               "<body>%(code)d: %(message)s</body></html>" % {
            "code": status_code,
            "message": httplib.responses[status_code],
        }

    @property
    def locale(self):
        """The local for the current session.

        Determined by either get_user_locale, which you can override to
        set the locale based on, e.g., a user preference stored in a
        database, or get_browser_locale, which uses the Accept-Language
        header.
        """
        if not hasattr(self, "_locale"):
            self._locale = self.get_user_locale()
            if not self._locale:
                self._locale = self.get_browser_locale()
                assert self._locale
        return self._locale

    def get_user_locale(self):
        """Override to determine the locale from the authenticated user.

        If None is returned, we use the Accept-Language header.
        """
        return None

    def get_browser_locale(self, default="en_US"):
        """Determines the user's locale from Accept-Language header.

        See http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.4
        """
        if "Accept-Language" in self.request.headers:
            languages = self.request.headers["Accept-Language"].split(",")
            locales = []
            for language in languages:
                parts = language.strip().split(";")
                if len(parts) > 1 and parts[1].startswith("q="):
                    try:
                        score = float(parts[1][2:])
                    except (ValueError, TypeError):
                        score = 0.0
                else:
                    score = 1.0
                locales.append((parts[0], score))
            if locales:
                locales.sort(key=lambda (l, s): s, reverse=True)
                codes = [l[0] for l in locales]
                return locale.get(*codes)
        return locale.get(default)

    @property
    def current_user(self):
        """The authenticated user for this request.

        Determined by either get_current_user, which you can override to
        set the user based on, e.g., a cookie. If that method is not
        overridden, this method always returns None.

        We lazy-load the current user the first time this method is called
        and cache the result after that.
        """
        if not hasattr(self, "_current_user"):
            self._current_user = self.get_current_user()
        return self._current_user

    def get_current_user(self):
        """Override to determine the current user from, e.g., a cookie."""
        return None

    def get_login_url(self):
        """Override to customize the login URL based on the request.

        By default, we use the 'login_url' application setting.
        """
        self.require_setting("login_url", "@tornado.web.authenticated")
        return self.application.settings["login_url"]

    def get_template_path(self):
        """Override to customize template path for each handler.

        By default, we use the 'template_path' application setting.
        Return None to load templates relative to the calling file.
        """
        return self.application.settings.get("template_path")

    @property
    def xsrf_token(self):
        """The XSRF-prevention token for the current user/session.

        To prevent cross-site request forgery, we set an '_xsrf' cookie
        and include the same '_xsrf' value as an argument with all POST
        requests. If the two do not match, we reject the form submission
        as a potential forgery.

        See http://en.wikipedia.org/wiki/Cross-site_request_forgery
        """
        if not hasattr(self, "_xsrf_token"):
            token = self.get_cookie("_xsrf")
            if not token:
                token = binascii.b2a_hex(uuid.uuid4().bytes)
                expires_days = 30 if self.current_user else None
                self.set_cookie("_xsrf", token, expires_days=expires_days)
            self._xsrf_token = token
        return self._xsrf_token

    def check_xsrf_cookie(self):
        """Verifies that the '_xsrf' cookie matches the '_xsrf' argument.

        To prevent cross-site request forgery, we set an '_xsrf' cookie
        and include the same '_xsrf' value as an argument with all POST
        requests. If the two do not match, we reject the form submission
        as a potential forgery.

        See http://en.wikipedia.org/wiki/Cross-site_request_forgery
        """
        if self.request.headers.get("X-Requested-With") == "XMLHttpRequest":
            return
        token = self.get_argument("_xsrf", None)
        if not token:
            raise HTTPError(403, "'_xsrf' argument missing from POST")
        if self.xsrf_token != token:
            raise HTTPError(403, "XSRF cookie does not match POST argument")

    def xsrf_form_html(self):
        """An HTML <input/> element to be included with all POST forms.

        It defines the _xsrf input value, which we check on all POST
        requests to prevent cross-site request forgery. If you have set
        the 'xsrf_cookies' application setting, you must include this
        HTML within all of your HTML forms.

        See check_xsrf_cookie() above for more information.
        """
        return '<input type="hidden" name="_xsrf" value="' + \
            escape.xhtml_escape(self.xsrf_token) + '"/>'

    def static_url(self, path):
        """Returns a static URL for the given relative static file path.

        This method requires you set the 'static_path' setting in your
        application (which specifies the root directory of your static
        files).

        We append ?v=<signature> to the returned URL, which makes our
        static file handler set an infinite expiration header on the
        returned content. The signature is based on the content of the
        file.

        If this handler has a "include_host" attribute, we include the
        full host for every static URL, including the "http://". Set
        this attribute for handlers whose output needs non-relative static
        path names.
        """
        self.require_setting("static_path", "static_url")
        if not hasattr(RequestHandler, "_static_hashes"):
            RequestHandler._static_hashes = {}
        hashes = RequestHandler._static_hashes
        if path not in hashes:
            try:
                f = open(os.path.join(
                    self.application.settings["static_path"], path))
                hashes[path] = hashlib.md5(f.read()).hexdigest()
                f.close()
            except:
                logging.error("Could not open static file %r", path)
                hashes[path] = None
        base = self.request.protocol + "://" + self.request.host \
            if getattr(self, "include_host", False) else ""
        static_url_prefix = self.settings.get('static_url_prefix', '/static/')
        if hashes.get(path):
            return base + static_url_prefix + path + "?v=" + hashes[path][:5]
        else:
            return base + static_url_prefix + path

    def async_callback(self, callback, *args, **kwargs):
        """Wrap callbacks with this if they are used on asynchronous requests.

        Catches exceptions and properly finishes the request.
        """
        if callback is None:
            return None
        if args or kwargs:
            callback = functools.partial(callback, *args, **kwargs)
        def wrapper(*args, **kwargs):
            try:
                return callback(*args, **kwargs)
            except Exception, e:
                if self._headers_written:
                    logging.error("Exception after headers written",
                                  exc_info=True)
                else:
                    self._handle_request_exception(e)
        return wrapper

    def require_setting(self, name, feature="this feature"):
        """Raises an exception if the given app setting is not defined."""
        if not self.application.settings.get(name):
            raise Exception("You must define the '%s' setting in your "
                            "application to use %s" % (name, feature))

    def reverse_url(self, name, *args):
        return self.application.reverse_url(name, *args)

    @contextlib.contextmanager
    def _stack_context(self):
        try:
            yield
        except Exception, e:
            self._handle_request_exception(e)

    def _execute(self, transforms, *args, **kwargs):
        """Executes this request with the given output transforms."""
        self._transforms = transforms
        with stack_context.StackContext(self._stack_context):
            if self.request.method not in self.SUPPORTED_METHODS:
                raise HTTPError(405)
            # If XSRF cookies are turned on, reject form submissions without
            # the proper cookie
            if self.request.method == "POST" and \
               self.application.settings.get("xsrf_cookies"):
                self.check_xsrf_cookie()
            self.prepare()
            if not self._finished:
                getattr(self, self.request.method.lower())(*args, **kwargs)
                if self._auto_finish and not self._finished:
                    self.finish()

    def _generate_headers(self):
        lines = [self.request.version + " " + str(self._status_code) + " " +
                 httplib.responses[self._status_code]]
        lines.extend(["%s: %s" % (n, v) for n, v in self._headers.iteritems()])
        for cookie_dict in getattr(self, "_new_cookies", []):
            for cookie in cookie_dict.values():
                lines.append("Set-Cookie: " + cookie.OutputString(None))
        return "\r\n".join(lines) + "\r\n\r\n"

    def _log(self):
        if self._status_code < 400:
            log_method = logging.info
        elif self._status_code < 500:
            log_method = logging.warning
        else:
            log_method = logging.error
        request_time = 1000.0 * self.request.request_time()
        log_method("%d %s %.2fms", self._status_code,
                   self._request_summary(), request_time)

    def _request_summary(self):
        return self.request.method + " " + self.request.uri + " (" + \
            self.request.remote_ip + ")"

    def _handle_request_exception(self, e):
        if isinstance(e, HTTPError):
            if e.log_message:
                format = "%d %s: " + e.log_message
                args = [e.status_code, self._request_summary()] + list(e.args)
                logging.warning(format, *args)
            if e.status_code not in httplib.responses:
                logging.error("Bad HTTP status code: %d", e.status_code)
                self.send_error(500, exception=e)
            else:
                self.send_error(e.status_code, exception=e)
        else:
            logging.error("Uncaught exception %s\n%r", self._request_summary(),
                          self.request, exc_info=e)
            self.send_error(500, exception=e)

    def _ui_module(self, name, module):
        def render(*args, **kwargs):
            if not hasattr(self, "_active_modules"):
                self._active_modules = {}
            if name not in self._active_modules:
                self._active_modules[name] = module(self)
            rendered = self._active_modules[name].render(*args, **kwargs)
            return rendered
        return render

    def _ui_method(self, method):
        return lambda *args, **kwargs: method(self, *args, **kwargs)


def asynchronous(method):
    """Wrap request handler methods with this if they are asynchronous.

    If this decorator is given, the response is not finished when the
    method returns. It is up to the request handler to call self.finish()
    to finish the HTTP request. Without this decorator, the request is
    automatically finished when the get() or post() method returns.

       class MyRequestHandler(web.RequestHandler):
           @web.asynchronous
           def get(self):
              http = httpclient.AsyncHTTPClient()
              http.fetch("http://friendfeed.com/", self._on_download)

           def _on_download(self, response):
              self.write("Downloaded!")
              self.finish()

    """
    @functools.wraps(method)
    def wrapper(self, *args, **kwargs):
        if self.application._wsgi:
            raise Exception("@asynchronous is not supported for WSGI apps")
        self._auto_finish = False
        return method(self, *args, **kwargs)
    return wrapper


def removeslash(method):
    """Use this decorator to remove trailing slashes from the request path.

    For example, a request to '/foo/' would redirect to '/foo' with this
    decorator. Your request handler mapping should use a regular expression
    like r'/foo/*' in conjunction with using the decorator.
    """
    @functools.wraps(method)
    def wrapper(self, *args, **kwargs):
        if self.request.path.endswith("/"):
            if self.request.method == "GET":
                uri = self.request.path.rstrip("/")
                if self.request.query: uri += "?" + self.request.query
                self.redirect(uri)
                return
            raise HTTPError(404)
        return method(self, *args, **kwargs)
    return wrapper


def addslash(method):
    """Use this decorator to add a missing trailing slash to the request path.

    For example, a request to '/foo' would redirect to '/foo/' with this
    decorator. Your request handler mapping should use a regular expression
    like r'/foo/?' in conjunction with using the decorator.
    """
    @functools.wraps(method)
    def wrapper(self, *args, **kwargs):
        if not self.request.path.endswith("/"):
            if self.request.method == "GET":
                uri = self.request.path + "/"
                if self.request.query: uri += "?" + self.request.query
                self.redirect(uri)
                return
            raise HTTPError(404)
        return method(self, *args, **kwargs)
    return wrapper


class Application(object):
    """A collection of request handlers that make up a web application.

    Instances of this class are callable and can be passed directly to
    HTTPServer to serve the application:

        application = web.Application([
            (r"/", MainPageHandler),
        ])
        http_server = httpserver.HTTPServer(application)
        http_server.listen(8080)
        ioloop.IOLoop.instance().start()

    The constructor for this class takes in a list of URLSpec objects
    or (regexp, request_class) tuples. When we receive requests, we
    iterate over the list in order and instantiate an instance of the
    first request class whose regexp matches the request path.

    Each tuple can contain an optional third element, which should be a
    dictionary if it is present. That dictionary is passed as keyword
    arguments to the contructor of the handler. This pattern is used
    for the StaticFileHandler below:

        application = web.Application([
            (r"/static/(.*)", web.StaticFileHandler, {"path": "/var/www"}),
        ])

    We support virtual hosts with the add_handlers method, which takes in
    a host regular expression as the first argument:

        application.add_handlers(r"www\.myhost\.com", [
            (r"/article/([0-9]+)", ArticleHandler),
        ])

    You can serve static files by sending the static_path setting as a
    keyword argument. We will serve those files from the /static/ URI
    (this is configurable with the static_url_prefix setting),
    and we will serve /favicon.ico and /robots.txt from the same directory.
    """
    def __init__(self, handlers=None, default_host="", transforms=None,
                 wsgi=False, **settings):
        if transforms is None:
            self.transforms = []
            if settings.get("gzip"):
                self.transforms.append(GZipContentEncoding)
            self.transforms.append(ChunkedTransferEncoding)
        else:
            self.transforms = transforms
        self.handlers = []
        self.named_handlers = {}
        self.default_host = default_host
        self.settings = settings
        self.ui_modules = {}
        self.ui_methods = {}
        self._wsgi = wsgi
        self._load_ui_modules(settings.get("ui_modules", {}))
        self._load_ui_methods(settings.get("ui_methods", {}))
        if self.settings.get("static_path"):
            path = self.settings["static_path"]
            handlers = list(handlers or [])
            static_url_prefix = settings.get("static_url_prefix",
                                             "/static/")
            handlers = [
                (re.escape(static_url_prefix) + r"(.*)", StaticFileHandler,
                 dict(path=path)),
                (r"/(favicon\.ico)", StaticFileHandler, dict(path=path)),
                (r"/(robots\.txt)", StaticFileHandler, dict(path=path)),
            ] + handlers
        if handlers: self.add_handlers(".*$", handlers)

        # Automatically reload modified modules
        if self.settings.get("debug") and not wsgi:
            import autoreload
            autoreload.start()

    def add_handlers(self, host_pattern, host_handlers):
        """Appends the given handlers to our handler list."""
        if not host_pattern.endswith("$"):
            host_pattern += "$"
        handlers = []
        # The handlers with the wildcard host_pattern are a special
        # case - they're added in the constructor but should have lower
        # precedence than the more-precise handlers added later.
        # If a wildcard handler group exists, it should always be last
        # in the list, so insert new groups just before it.
        if self.handlers and self.handlers[-1][0].pattern == '.*$':
            self.handlers.insert(-1, (re.compile(host_pattern), handlers))
        else:
            self.handlers.append((re.compile(host_pattern), handlers))

        for spec in host_handlers:
            if type(spec) is type(()):
                assert len(spec) in (2, 3)
                pattern = spec[0]
                handler = spec[1]
                if len(spec) == 3:
                    kwargs = spec[2]
                else:
                    kwargs = {}
                spec = URLSpec(pattern, handler, kwargs)
            handlers.append(spec)
            if spec.name:
                if spec.name in self.named_handlers:
                    logging.warning(
                        "Multiple handlers named %s; replacing previous value",
                        spec.name)
                self.named_handlers[spec.name] = spec

    def add_transform(self, transform_class):
        """Adds the given OutputTransform to our transform list."""
        self.transforms.append(transform_class)

    def _get_host_handlers(self, request):
        host = request.host.lower().split(':')[0]
        for pattern, handlers in self.handlers:
            if pattern.match(host):
                return handlers
        # Look for default host if not behind load balancer (for debugging)
        if "X-Real-Ip" not in request.headers:
            for pattern, handlers in self.handlers:
                if pattern.match(self.default_host):
                    return handlers
        return None

    def _load_ui_methods(self, methods):
        if type(methods) is types.ModuleType:
            self._load_ui_methods(dict((n, getattr(methods, n))
                                       for n in dir(methods)))
        elif isinstance(methods, list):
            for m in methods: self._load_ui_methods(m)
        else:
            for name, fn in methods.iteritems():
                if not name.startswith("_") and hasattr(fn, "__call__") \
                   and name[0].lower() == name[0]:
                    self.ui_methods[name] = fn

    def _load_ui_modules(self, modules):
        if type(modules) is types.ModuleType:
            self._load_ui_modules(dict((n, getattr(modules, n))
                                       for n in dir(modules)))
        elif isinstance(modules, list):
            for m in modules: self._load_ui_modules(m)
        else:
            assert isinstance(modules, dict)
            for name, cls in modules.iteritems():
                try:
                    if issubclass(cls, UIModule):
                        self.ui_modules[name] = cls
                except TypeError:
                    pass

    def __call__(self, request):
        """Called by HTTPServer to execute the request."""
        transforms = [t(request) for t in self.transforms]
        handler = None
        args = []
        kwargs = {}
        handlers = self._get_host_handlers(request)
        if not handlers:
            handler = RedirectHandler(
                request, "http://" + self.default_host + "/")
        else:
            for spec in handlers:
                match = spec.regex.match(request.path)
                if match:
                    # None-safe wrapper around urllib.unquote to handle
                    # unmatched optional groups correctly
                    def unquote(s):
                        if s is None: return s
                        return urllib.unquote(s)
                    handler = spec.handler_class(self, request, **spec.kwargs)
                    # Pass matched groups to the handler.  Since
                    # match.groups() includes both named and unnamed groups,
                    # we want to use either groups or groupdict but not both.
                    kwargs = dict((k, unquote(v))
                                  for (k, v) in match.groupdict().iteritems())
                    if kwargs:
                        args = []
                    else:
                        args = [unquote(s) for s in match.groups()]
                    break
            if not handler:
                handler = ErrorHandler(self, request, 404)

        # In debug mode, re-compile templates and reload static files on every
        # request so you don't need to restart to see changes
        if self.settings.get("debug"):
            if getattr(RequestHandler, "_templates", None):
                map(lambda loader: loader.reset(),
                    RequestHandler._templates.values())
            RequestHandler._static_hashes = {}

        handler._execute(transforms, *args, **kwargs)
        return handler

    def reverse_url(self, name, *args):
        """Returns a URL path for handler named `name`

        The handler must be added to the application as a named URLSpec
        """
        if name in self.named_handlers:
            return self.named_handlers[name].reverse(*args)
        raise KeyError("%s not found in named urls" % name)


class HTTPError(Exception):
    """An exception that will turn into an HTTP error response."""
    def __init__(self, status_code, log_message=None, *args):
        self.status_code = status_code
        self.log_message = log_message
        self.args = args

    def __str__(self):
        message = "HTTP %d: %s" % (
            self.status_code, httplib.responses[self.status_code])
        if self.log_message:
            return message + " (" + (self.log_message % self.args) + ")"
        else:
            return message


class ErrorHandler(RequestHandler):
    """Generates an error response with status_code for all requests."""
    def __init__(self, application, request, status_code):
        RequestHandler.__init__(self, application, request)
        self.set_status(status_code)

    def prepare(self):
        raise HTTPError(self._status_code)


class RedirectHandler(RequestHandler):
    """Redirects the client to the given URL for all GET requests.

    You should provide the keyword argument "url" to the handler, e.g.:

        application = web.Application([
            (r"/oldpath", web.RedirectHandler, {"url": "/newpath"}),
        ])
    """
    def __init__(self, application, request, url, permanent=True):
        RequestHandler.__init__(self, application, request)
        self._url = url
        self._permanent = permanent

    def get(self):
        self.redirect(self._url, permanent=self._permanent)


class StaticFileHandler(RequestHandler):
    """A simple handler that can serve static content from a directory.

    To map a path to this handler for a static data directory /var/www,
    you would add a line to your application like:

        application = web.Application([
            (r"/static/(.*)", web.StaticFileHandler, {"path": "/var/www"}),
        ])

    The local root directory of the content should be passed as the "path"
    argument to the handler.

    To support aggressive browser caching, if the argument "v" is given
    with the path, we set an infinite HTTP expiration header. So, if you
    want browsers to cache a file indefinitely, send them to, e.g.,
    /static/images/myimage.png?v=xxx.
    """
    def __init__(self, application, request, path):
        RequestHandler.__init__(self, application, request)
        self.root = os.path.abspath(path) + os.path.sep

    def head(self, path):
        self.get(path, include_body=False)

    def get(self, path, include_body=True):
        abspath = os.path.abspath(os.path.join(self.root, path))
        if not abspath.startswith(self.root):
            raise HTTPError(403, "%s is not in root static directory", path)
        if not os.path.exists(abspath):
            raise HTTPError(404)
        if not os.path.isfile(abspath):
            raise HTTPError(403, "%s is not a file", path)

        stat_result = os.stat(abspath)
        modified = datetime.datetime.fromtimestamp(stat_result[stat.ST_MTIME])

        self.set_header("Last-Modified", modified)
        if "v" in self.request.arguments:
            self.set_header("Expires", datetime.datetime.utcnow() + \
                                       datetime.timedelta(days=365*10))
            self.set_header("Cache-Control", "max-age=" + str(86400*365*10))
        else:
            self.set_header("Cache-Control", "public")
        mime_type, encoding = mimetypes.guess_type(abspath)
        if mime_type:
            self.set_header("Content-Type", mime_type)

        self.set_extra_headers(path)

        # Check the If-Modified-Since, and don't send the result if the
        # content has not been modified
        ims_value = self.request.headers.get("If-Modified-Since")
        if ims_value is not None:
            date_tuple = email.utils.parsedate(ims_value)
            if_since = datetime.datetime.fromtimestamp(time.mktime(date_tuple))
            if if_since >= modified:
                self.set_status(304)
                return

        if not include_body:
            return
        self.set_header("Content-Length", stat_result[stat.ST_SIZE])
        file = open(abspath, "rb")
        try:
            self.write(file.read())
        finally:
            file.close()

    def set_extra_headers(self, path):
        """For subclass to add extra headers to the response"""
        pass


class FallbackHandler(RequestHandler):
    """A RequestHandler that wraps another HTTP server callback.

    The fallback is a callable object that accepts an HTTPRequest,
    such as an Application or tornado.wsgi.WSGIContainer.  This is most
    useful to use both tornado RequestHandlers and WSGI in the same server.
    Typical usage:
        wsgi_app = tornado.wsgi.WSGIContainer(
            django.core.handlers.wsgi.WSGIHandler())
        application = tornado.web.Application([
            (r"/foo", FooHandler),
            (r".*", FallbackHandler, dict(fallback=wsgi_app),
        ])
    """
    def __init__(self, app, request, fallback):
        RequestHandler.__init__(self, app, request)
        self.fallback = fallback

    def prepare(self):
        self.fallback(self.request)
        self._finished = True


class OutputTransform(object):
    """A transform modifies the result of an HTTP request (e.g., GZip encoding)

    A new transform instance is created for every request. See the
    ChunkedTransferEncoding example below if you want to implement a
    new Transform.
    """
    def __init__(self, request):
        pass

    def transform_first_chunk(self, headers, chunk, finishing):
        return headers, chunk

    def transform_chunk(self, chunk, finishing):
        return chunk


class GZipContentEncoding(OutputTransform):
    """Applies the gzip content encoding to the response.

    See http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.11
    """
    CONTENT_TYPES = set([
        "text/plain", "text/html", "text/css", "text/xml",
        "application/x-javascript", "application/xml", "application/atom+xml",
        "text/javascript", "application/json", "application/xhtml+xml"])
    MIN_LENGTH = 5

    def __init__(self, request):
        self._gzipping = request.supports_http_1_1() and \
            "gzip" in request.headers.get("Accept-Encoding", "")

    def transform_first_chunk(self, headers, chunk, finishing):
        if self._gzipping:
            ctype = headers.get("Content-Type", "").split(";")[0]
            self._gzipping = (ctype in self.CONTENT_TYPES) and \
                (not finishing or len(chunk) >= self.MIN_LENGTH) and \
                (finishing or "Content-Length" not in headers) and \
                ("Content-Encoding" not in headers)
        if self._gzipping:
            headers["Content-Encoding"] = "gzip"
            self._gzip_value = cStringIO.StringIO()
            self._gzip_file = gzip.GzipFile(mode="w", fileobj=self._gzip_value)
            self._gzip_pos = 0
            chunk = self.transform_chunk(chunk, finishing)
            if "Content-Length" in headers:
                headers["Content-Length"] = str(len(chunk))
        return headers, chunk

    def transform_chunk(self, chunk, finishing):
        if self._gzipping:
            self._gzip_file.write(chunk)
            if finishing:
                self._gzip_file.close()
            else:
                self._gzip_file.flush()
            chunk = self._gzip_value.getvalue()
            if self._gzip_pos > 0:
                chunk = chunk[self._gzip_pos:]
            self._gzip_pos += len(chunk)
        return chunk


class ChunkedTransferEncoding(OutputTransform):
    """Applies the chunked transfer encoding to the response.

    See http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.6.1
    """
    def __init__(self, request):
        self._chunking = request.supports_http_1_1()

    def transform_first_chunk(self, headers, chunk, finishing):
        if self._chunking:
            # No need to chunk the output if a Content-Length is specified
            if "Content-Length" in headers or "Transfer-Encoding" in headers:
                self._chunking = False
            else:
                headers["Transfer-Encoding"] = "chunked"
                chunk = self.transform_chunk(chunk, finishing)
        return headers, chunk

    def transform_chunk(self, block, finishing):
        if self._chunking:
            # Don't write out empty chunks because that means END-OF-STREAM
            # with chunked encoding
            if block:
                block = ("%x" % len(block)) + "\r\n" + block + "\r\n"
            if finishing:
                block += "0\r\n\r\n"
        return block


def authenticated(method):
    """Decorate methods with this to require that the user be logged in."""
    @functools.wraps(method)
    def wrapper(self, *args, **kwargs):
        if not self.current_user:
            if self.request.method == "GET":
                url = self.get_login_url()
                if "?" not in url:
                    if urlparse.urlsplit(url).scheme:
                        # if login url is absolute, make next absolute too
                        next_url = self.request.full_url()
                    else:
                        next_url = self.request.uri
                    url += "?" + urllib.urlencode(dict(next=next_url))
                self.redirect(url)
                return
            raise HTTPError(403)
        return method(self, *args, **kwargs)
    return wrapper


class UIModule(object):
    """A UI re-usable, modular unit on a page.

    UI modules often execute additional queries, and they can include
    additional CSS and JavaScript that will be included in the output
    page, which is automatically inserted on page render.
    """
    def __init__(self, handler):
        self.handler = handler
        self.request = handler.request
        self.ui = handler.ui
        self.current_user = handler.current_user
        self.locale = handler.locale

    def render(self, *args, **kwargs):
        raise NotImplementedError()

    def embedded_javascript(self):
        """Returns a JavaScript string that will be embedded in the page."""
        return None

    def javascript_files(self):
        """Returns a list of JavaScript files required by this module."""
        return None

    def embedded_css(self):
        """Returns a CSS string that will be embedded in the page."""
        return None

    def css_files(self):
        """Returns a list of CSS files required by this module."""
        return None

    def html_head(self):
        """Returns a CSS string that will be put in the <head/> element"""
        return None

    def html_body(self):
        """Returns an HTML string that will be put in the <body/> element"""
        return None

    def render_string(self, path, **kwargs):
        return self.handler.render_string(path, **kwargs)

class URLSpec(object):
    """Specifies mappings between URLs and handlers."""
    def __init__(self, pattern, handler_class, kwargs={}, name=None):
        """Creates a URLSpec.

        Parameters:
        pattern: Regular expression to be matched.  Any groups in the regex
            will be passed in to the handler's get/post/etc methods as
            arguments.
        handler_class: RequestHandler subclass to be invoked.
        kwargs (optional): A dictionary of additional arguments to be passed
            to the handler's constructor.
        name (optional): A name for this handler.  Used by
            Application.reverse_url.
        """
        if not pattern.endswith('$'):
            pattern += '$'
        self.regex = re.compile(pattern)
        self.handler_class = handler_class
        self.kwargs = kwargs
        self.name = name
        self._path, self._group_count = self._find_groups()

    def _find_groups(self):
        """Returns a tuple (reverse string, group count) for a url.

        For example: Given the url pattern /([0-9]{4})/([a-z-]+)/, this method
        would return ('/%s/%s/', 2).
        """
        pattern = self.regex.pattern
        if pattern.startswith('^'):
            pattern = pattern[1:]
        if pattern.endswith('$'):
            pattern = pattern[:-1]

        if self.regex.groups != pattern.count('('):
            # The pattern is too complicated for our simplistic matching,
            # so we can't support reversing it.
            return (None, None)

        pieces = []
        for fragment in pattern.split('('):
            if ')' in fragment:
                paren_loc = fragment.index(')')
                if paren_loc >= 0:
                    pieces.append('%s' + fragment[paren_loc + 1:])
            else:
                pieces.append(fragment)

        return (''.join(pieces), self.regex.groups)

    def reverse(self, *args):
        assert self._path is not None, \
            "Cannot reverse url regex " + self.regex.pattern
        assert len(args) == self._group_count, "required number of arguments "\
            "not found"
        if not len(args):
            return self._path
        return self._path % tuple([str(a) for a in args])

url = URLSpec

def _utf8(s):
    if isinstance(s, unicode):
        return s.encode("utf-8")
    assert isinstance(s, str)
    return s


def _unicode(s):
    if isinstance(s, str):
        try:
            return s.decode("utf-8")
        except UnicodeDecodeError:
            raise HTTPError(400, "Non-utf8 argument")
    assert isinstance(s, unicode)
    return s


def _time_independent_equals(a, b):
    if len(a) != len(b):
        return False
    result = 0
    for x, y in zip(a, b):
        result |= ord(x) ^ ord(y)
    return result == 0


class _O(dict):
    """Makes a dictionary behave like an object."""
    def __getattr__(self, name):
        try:
            return self[name]
        except KeyError:
            raise AttributeError(name)

    def __setattr__(self, name, value):
        self[name] = value
