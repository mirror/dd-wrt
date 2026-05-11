#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import urllib
import cookielib
import socket
import os
import weakref
import select
import random
import time
import sys
import base64
import urllib2
import httplib
import ssl

from base import BaseComponent

GLOBAL_DEFAULT_TIMEOUT = socket._GLOBAL_DEFAULT_TIMEOUT

import cStringIO as StringIO

import cerealizer
cerealizer.register(cookielib.Cookie)


class HeaderDictionary(dict):
  def __init__(self, *args, **kwargs):
     dict.__init__(self, *args, **kwargs)
     for key in list(self.keys()):
       value = self[key]
       del self[key]
       self[key] = value
  
  def __setitem__(self, name, value):
    parts = name.split('-')
    i = 0
    while i < len(parts):
      parts[i] = parts[i][0].upper() + parts[i][1:]
      i += 1
    name = '-'.join(parts)
    dict.__setitem__(self, name, value)

class CookieObject(Framework.objects.XMLObject):
  def __init__(self, *args, **kwargs):
    Framework.objects.XMLObject.__init__(self, *args, **kwargs)
    self.tagName = 'Cookie'

class HTTPHeaderProxy(object):
  def __init__(self, headers):
    if headers:
      self._headers = dict(headers)
    else:
      self._headers = {}
    
  def __getitem__(self, name):
    return self._headers[name.lower()]

  def __iter__(self):
    return self._headers.__iter__()
    
  def __repr__(self):
    return repr(self._headers)


class RedirectHandler(urllib2.HTTPRedirectHandler):
  def http_error_301(self, req, fp, code, msg, headers):
    if hasattr(req, 'follow_redirects') and req.follow_redirects == False:
      raise Framework.exceptions.RedirectError(code, headers)
    result = urllib2.HTTPRedirectHandler.http_error_301(self, req, fp, code, msg, headers)              
    return result                                       

  def http_error_302(self, req, fp, code, msg, headers):
    if hasattr(req, 'follow_redirects') and req.follow_redirects == False:
      raise Framework.exceptions.RedirectError(code, headers)
    result = urllib2.HTTPRedirectHandler.http_error_302(self, req, fp, code, msg, headers)
    return result

  def http_error_303(self, req, fp, code, msg, headers):
    if hasattr(req, 'follow_redirects') and req.follow_redirects == False:
      raise Framework.exceptions.RedirectError(code, headers)
    result = urllib2.HTTPRedirectHandler.http_error_303(self, req, fp, code, msg, headers)
    return result

  def http_error_307(self, req, fp, code, msg, headers):
    if hasattr(req, 'follow_redirects') and req.follow_redirects == False:
      raise Framework.exceptions.RedirectError(code, headers)
    result = urllib2.HTTPRedirectHandler.http_error_307(self, req, fp, code, msg, headers)
    return result


class Request(urllib2.Request):
  def __init__(self, *args, **kwargs):
    urllib2.Request.__init__(self, *args, **kwargs)
    self.follow_redirects = False


class HTTPRequest(object):
  def __init__(self, core, url, data, headers, cache, encoding, errors, timeout, immediate, sleep, opener, follow_redirects = True, method = None):
    self._core = core
    self._url = url
    self._request_headers = HeaderDictionary(headers)
    self._post_data = data
    self._cache = cache
    self._data = None
    self._headers = None
    self._encoding = encoding
    self._errors = errors
    self._timeout = timeout
    self._opener = opener
    self._sleep = sleep
    self._follow_redirects = follow_redirects
    self._method = method
    if immediate:
      self.load()

  def _content_type_allowed(self, content_type):
    for t in ['html', 'xml', 'json', 'javascript']:
      if t in content_type:
        return True
    return False

  def load(self):
    if not self._data:
      if self._cache != None and self._cache['content'] and not self._cache.expired:

        # If we've previously cached non-HTML/XML data in an agent plug-in, remove it
        content_type = self._cache.headers.get('content-type', '')
        if self._core.plugin_class == Framework.constants.plugin_class.agent and not self._content_type_allowed(content_type):
          self._core.log.debug("Removing cached data for '%s' (content type '%s' not cacheable in Agent plug-ins)", self._url, content_type)
          manager = self._cache._manager
          del manager[self._url]

        else:
          self._core.log.debug("Fetching '%s' from the HTTP cache", self._url)
          self._data = self._cache['content']
          self._headers = HTTPHeaderProxy(self._cache.headers)
          return

      self._core.log.debug("Requesting '%s'", self._url)

      f = None
      try:

        if 'PLEXTOKEN' in os.environ and len(os.environ['PLEXTOKEN']) > 0 and self._request_headers is not None and self._url.find('http://127.0.0.1') == 0:
          self._request_headers['X-Plex-Token'] = os.environ['PLEXTOKEN']
        
        req = Request(self._url, self._post_data, self._request_headers)
        req.follow_redirects = self._follow_redirects

        # If a method has been set, override the request's default method.
        if self._method:
          req.get_method = lambda: self._method

        f = self._opener.open(req, timeout=self._timeout)
        if f.headers.get('Content-Encoding') == 'gzip':
          self._data = self._core.data.archiving.gzip_decompress(f.read())
        else:
          self._data = f.read()

        #TODO: Move to scheduled save method when the background worker is finished
        self._core.networking._save()

        info = f.info()
        self._headers = HTTPHeaderProxy(info.dict)
        del info

        if self._cache != None:
          # Only allow caching XML, HTML or JSON in Agent plug-ins
          content_type = self._headers._headers.get('content-type', '')
          if self._core.plugin_class == Framework.constants.plugin_class.agent and not self._content_type_allowed(content_type):
            self._core.log.debug("Not caching '%s' (content type '%s' not cacheable in Agent plug-ins)", self._url, content_type)

          else:
            self._cache['content'] = self._data
            self._cache.headers = self._headers._headers

        if self._sleep > 0:
          time.sleep(self._sleep)

      except urllib2.HTTPError, e:
        content = None

        # Fetch the response body before closing the socket so exception handlers can access it
        if hasattr(e, 'read') and e.hdrs and e.hdrs.getheader('Content-Encoding') == 'gzip':
          content = self._core.data.archiving.gzip_decompress(e.read())
        elif hasattr(e, 'read'):
          content = e.read()
        
        e.__dict__['content'] = content if content else ''

        e.close()
        self._core.log.error("Error opening URL '%s'", self._url)
        raise

      finally:
        if f:
          f.fp._sock.recv = None  # Hack to stop us leaking file descriptors on errors.
          f.close()
          del f
        
  @property
  def headers(self):
    if self._headers:
      return self._headers
    else:
      self._core.log.debug("Fetching HTTP headers for '%s'", self._url)
      req = Request(self._url, self._post_data, self._request_headers)
      req.follow_redirects = self._follow_redirects
      req.get_method = lambda: 'HEAD'
      f = self._opener.open(req)
      info = f.info()
      self._headers = HTTPHeaderProxy(info.dict)
      return self._headers
  
  def __str__(self):
    self.load()
    if self._encoding:
      if self._errors:
        result = str(unicode(self._data, self._encoding, self._errors))
      else:
        result = str(unicode(self._data, self._encoding))
    else:
      result = self._data
    return result
    
  def __len__(self):
    self.load()
    return len(self._data)
    
  def __add__(self, other):
    return str(self) + other
    
  def __radd__(self, other):
    return other + str(self)
    
  @property
  def content(self):
    return self.__str__()

    

class Networking(BaseComponent):
  def _init(self):

    self.default_headers = {
      'Accept-Encoding': 'gzip',
      'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_2) AppleWebKit/537.74.9 (KHTML, like Gecko) Version/7.0.2 Safari/537.74.9'
    }
    
    # Set up the cookie jar if global cookies are enabled
    if self._global_cookies_enabled:
      self._cookie_jar = cookielib.MozillaCookieJar()
      self._cookie_file_path = "%s/HTTPCookies" % self._core.storage.data_path
      if os.path.isfile(self._cookie_file_path):
        try:
          self._cookie_jar.load(self._cookie_file_path)
          self._core.log.debug('Loaded HTTP cookies')
        except:
          self._core.log_exception('Exception loading HTTP cookies')
      else:
        self._core.log.debug("No cookie jar found")
    else:
      self._cookie_jar = None

    self._cache_mgr = self._core.caching.get_cache_manager('HTTP', system=True) if self._http_caching_enabled else None
    self._password_mgr = urllib2.HTTPPasswordMgrWithDefaultRealm() if self._global_http_auth_enabled else None

    self.cache_time = 0
    self.default_timeout = self._core.config.default_network_timeout
    
    # On Windows make sure we don't try to load from its default CA paths as this is broken.
    # We provide our own CA path via the SSL_CERT_FILE environment variable.
    if sys.platform == 'win32':
      setattr(ssl.SSLContext, "_windows_cert_stores", ())

    # Build a global opener.
    self._opener = self.build_opener()

  def build_opener(self, cookie_jar=None):
    if cookie_jar == None:
      cookie_jar = self._cookie_jar

    opener_args = []

    if cookie_jar != None:
      opener_args.append(urllib2.HTTPCookieProcessor(cookie_jar))

    if self._password_mgr != None:
      opener_args.append(urllib2.HTTPBasicAuthHandler(self._password_mgr))

    opener_args.append(RedirectHandler)
    opener_args.append(urllib2.ProxyHandler)
    return urllib2.build_opener(*opener_args)


  def _save(self, cookie_jar=None):
    if self._global_cookies_enabled and (cookie_jar == None or cookie_jar == self._cookie_jar):
      self._cookie_jar.save(self._cookie_file_path)
    

  def http_request(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, immediate=False, sleep=0, data=None, opener=None, sandbox=None, follow_redirects=True, basic_auth=None, method=None):
    if cacheTime == None: cacheTime = self.cache_time
    
    # Strip off anchors from the end of URLs if provided, as it upsets certain servers if we send them with the request
    pos = url.rfind('#')
    if pos > 0:
      url = url[:pos]
    
    # If a value dictionary was provided instead of data for a POST body, urlencode it
    if values and not data:
      data = urllib.urlencode(values)
    
    # If POST data was provided, don't cache the response
    if data:
      cacheTime = 0
      immediate = True

    # If a custom opener was provided, or the cacheTime is 0, don't save data for this request to the HTTP cache
    url_cache = None
    if opener == None:
      opener = self._opener

    if self._http_caching_enabled:
      if cacheTime > 0:
        cache_mgr = self._cache_mgr
      
        # Check whether we should trim the HTTP cache
        if cache_mgr.item_count > self._core.config.http_cache_max_items + self._core.config.http_cache_max_items_grace:
          cache_mgr.trim(self._core.config.http_cache_max_size, self._core.config.http_cache_max_items)
      
        url_cache = cache_mgr[url]
        url_cache.set_expiry_interval(cacheTime)
        
      else:
        # Delete any cached data we have already
        del self._cache_mgr[url]
    
    # Create a combined dictionary of all headers
    h = dict(self.default_headers)
    if sandbox:
      h.update(sandbox.custom_headers)
    h.update(headers)

    if basic_auth != None:
      h['Authorization'] = self.generate_basic_auth_header(*basic_auth)
    
    return HTTPRequest(self._core, url, data, h, url_cache, encoding, errors, timeout, immediate, sleep, opener, follow_redirects, method)


  def set_http_password(self, url, username, password, realm=None, manager=None):
    if self._global_http_auth_enabled:
      # Strip http:// from the beginning of the url
      if url[0:7] == "http://":
        url = url[7:]
      if manager == None: manager = self._password_mgr
      manager.add_password(realm, url, username, password)
    

  def get_cookies_for_url(self, url, cookie_jar):
    if cookie_jar == None: cookie_jar = self._cookie_jar
    request = urllib2.Request(url)
    cookie_jar.add_cookie_header(request)
    if request.unredirected_hdrs.has_key('Cookie'):
      return request.unredirected_hdrs['Cookie']
    return None
    

  def cookie_container(self, **kwargs):
    container = Framework.objects.MediaContainer(self._core)
    
    for cookie in self._cookie_jar:
    
      should_append = True
      for name in kwargs:
        if hasattr(cookie, name) and getattr(cookie, name) != kwargs[name]:
          should_append = False
          break

      if should_append:
        container.Append(
          CookieObject(
            self._core,
            domain = cookie.domain,
            path = cookie.path,
            name = cookie.name,
            value = cookie.value,
            secure = cookie.secure,
          )
        )
    return container
    

  def clear_cookies(self, cookie_jar=None):
    if cookie_jar == None:
      cookie_jar = self._cookie_jar
    if cookie_jar:
      self._core.log.debug("Clearing HTTP cookies")
      cookie_jar.clear()
      self._core.storage.remove(self._cookie_file_path)
      self._save(cookie_jar)
    

  def clear_http_cache(self):
    if self._http_caching_enabled:
      self._cache_mgr.clear()
    

  @property
  def address(self):
    try:
      result = socket.gethostbyname(socket.gethostname())
      return result
    except:
      return None
      

  @property
  def hostname(self):
    return socket.gethostname()
    

  @property
  def default_timeout(self):
    return socket.getdefaulttimeout()
    

  @default_timeout.setter
  def default_timeout(self, timeout):
    self._core.log.debug('Setting the default network timeout to %.1f', float(timeout))
    return socket.setdefaulttimeout(timeout)
    

  #TODO: Allow args
  def socket(self):
    return socket.socket()
    

  def resolve_hostname_via_pms(self, hostname):
    url = 'http://127.0.0.1:32400/servers/resolve?name=%s' % hostname
    data = self.http_request(url, cacheTime=0).content
    xml = self._core.data.xml.from_string(data)
    return xml.xpath('//Address')[0].get('address')
    

  def resolve_hostname_if_required(self, hostname):
    # If running on Linux or Windows, and resolving a Bonjour host, request it via PMS.
    # This is faster on Windows (no two-second delay), and makes it possible on Linux 
    # (since gethostbyname doesn't work at all for mDNS names).
    #
    if (hostname.endswith('.local.') or hostname.endswith('.local')) and self._core.runtime.os in ('Linux', 'Windows'):
      return self.resolve_hostname_via_pms(hostname)
    # Otherwise, return the hostname unmodified, as the OS will be able to resolve it
    return hostname


  @property
  def _global_cookies_enabled(self):
    return self._core.policy.allow_global_cookies


  @property
  def _http_caching_enabled(self):
    return self._core.policy.enable_http_caching and not self._core.config.daemonized

  @property
  def _global_http_auth_enabled(self):
    return self._core.policy.allow_global_http_auth


  def generate_basic_auth_header(self, username, password):
    return "Basic  %s" % base64.b64encode('%s: %s' % (username, password))
    
