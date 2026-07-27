#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import threading
import sys
import urllib
import urlparse
import cookielib
import weakref

class flags(object):
  indirect = 'Indirect'
  syncable = 'Syncable'


class SimulatedRequest(object):
  def __init__(self, url):
    self.full_url = url
    
  def get_full_url(self):
    return self.full_url
    
  def get_host(self):
    return urlparse.urlparse(self.full_url)[1]
    
  def get_origin_req_host(self):
    return self.get_host()
    
  def is_unverifiable(self):
    return False



class ExecutionContext(threading.local):
  def __init__(self, sandbox):
    # Store a reference to the sandbox.
    self._sandbox = weakref.proxy(sandbox)

    # Initialize attributes.
    self._request = None
    self._cache_time = None
    self.prefix = None
    self.media_info = None
    self.opener = None
    self.cookie_jar = None
    self.cached_http_responses = {}
    self.response_status = None
    self.response_headers = {}
    self.http_headers = dict(self._sandbox.custom_headers)
    self.pref_values = {}
    self.session_data = {}
    self.log = []
    self.flags = []

    
  @property
  def _core(self):
    return self._sandbox._core


  def import_values(self, values):
    for key, value in values.iteritems():
      setattr(self, key, value)


  def export_values(self):
    return dict(
      request = self._request,
      cache_time = self._cache_time,
      prefix = self.prefix,
      cached_http_responses = dict(self.cached_http_responses),
      flags = list(self.flags)
    )


  def add_cached_response_cookies(self):
    if self.cookie_jar == None:
      self.cookie_jar = cookielib.MozillaCookieJar()
    for url in self.cached_http_responses:
      response = self.cached_http_responses[url]
      self._core.log.debug("Attempting to add cached response cookies from headers: %s" % str(response.headers))
      if 'Set-Cookie' in response.headers:
        self._core.log.debug("Found one or more Set-Cookie headers")
        simulated_request = SimulatedRequest(url)
        self.cookie_jar.extract_cookies(response, simulated_request)


  @property
  def request(self):
    return self._request


  @request.setter
  def request(self, request):
    # Store a reference to the request.
    self._request = request
    
    if request == None:
      return

    # Run all 'before request' functions.
    for group in [handler.before_all_functions for handler in self._core.runtime._handlers]:
      for func in group:
        func(self)
    
    
  def get_final_headers(self):
    # Copy the response headers dict
    response_headers = dict(self.response_headers)

    # Run all 'after request' functions.
    for group in [handler.after_all_functions for handler in self._core.runtime._handlers]:
      for func in group:
        func(self, response_headers)
    
    return response_headers


  def create_session_data(self):
    self.session_data = {}

    # Don't create session data if we're not in a request context, or if the request's URI points
    # to a private resource.
    #
    if self.request == None or self.request.uri.startswith('/:/'):
      return

    # Call the BeginSession function in the sandbox.
    try:
      if 'BeginSession' in self._sandbox.environment:
        self._sandbox.call_named_function('BeginSession')
    except:
      self._core.log_exception("Exception calling BeginSession function")


  def get_header(self, header, default=None):
    if self.request:
      return self.request.headers.get(header, default)


  @property
  def txn_id(self):
    return self.get_header(Framework.constants.header.transaction_id)


  @property
  def platform(self):
    return self.get_header(Framework.constants.header.client_platform, self.get_header(Framework.constants.header.client_platform_old, None))


  @property
  def token(self):
    return self.get_header(Framework.constants.header.token)


  @property
  def client_version(self):
    return self.get_header(Framework.constants.header.client_version, "0")


  @property
  def product(self):
    return self.get_header(Framework.constants.header.product)


  @property
  def proxy_user_data(self):
    return self._sandbox.policy.always_use_session_cookies or (self.request and (Framework.constants.header.proxy_cookies in self.request.headers or Framework.constants.header.token in self.request.headers or self._core.config.daemonized))

    
  @property
  def locale(self):
    return self.get_header(Framework.constants.header.language)


  @property
  def cache_time(self):
    return self._cache_time
    

  @cache_time.setter
  def cache_time(self, value):
    if value == None:
      self._cache_time = self._core.networking.cache_time
    elif self._cache_time == None or value < self._cache_time:
      self._cache_time = value
    
