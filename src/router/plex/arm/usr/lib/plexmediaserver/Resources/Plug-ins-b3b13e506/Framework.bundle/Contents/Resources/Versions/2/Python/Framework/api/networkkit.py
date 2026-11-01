#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import xmlrpclib

from base import BaseKit
from Framework.components.networking import GLOBAL_DEFAULT_TIMEOUT


class FrameworkTransport(xmlrpclib.Transport):
  
  def __init__(self, sandbox):
    self._sandbox = sandbox
    xmlrpclib.Transport.__init__(self, use_datetime=True)
    
  @property
  def user_agent(self):
    header = 'User-Agent'
    return self._sandbox.custom_headers.get(header, self._sandbox._core.networking.default_headers.get(header))



class BaseHTTPKit(BaseKit):
  
  @property
  def _opener(self):
    return self._context.opener
    

  @property
  def _custom_headers(self):
    # TODO: Fix this hack once we have per-user PMS data
    return self._context.http_headers if self._context.proxy_user_data else self._sandbox.custom_headers
    

  def _add_headers(self, headers={}):
    all_headers = dict(self._custom_headers)
    all_headers.update(headers)
    return all_headers
    

  def _http_request(self, url, *args, **kwargs):
    # Insert the current sandbox into the kwargs dict
    kwargs['sandbox'] = self._sandbox
      
    # If a cached response for this URL is stored in the context, insert it into
    # the object
    if self._context and url in self._context.cached_http_responses:
      
      # Remove the 'immediate' argument from kwargs if present
      if 'immediate' in kwargs:
        del kwargs['immediate']

      # Get a HTTPRequest object from the networking component
      req = self._core.networking.http_request(url, *args, **kwargs)

      self._core.log.debug('Using the cached body for "%s" from this context', url)
      response = self._context.cached_http_responses[url]
      req._data = response.body
      req._headers = dict(response.headers)
      
    else:
      # Get a HTTPRequest object from the networking component
      req = self._core.networking.http_request(url, *args, **kwargs)

    return req



class XMLRPCKit(BaseKit):
  
  def Proxy(self, url, encoding=None):
    """
      Returns an :class:`xmlrpclib.ServerProxy` object for the given *url*. The framework's
      default HTTP headers will be used.
    """
    if url[0:8] == 'https://':
      raise Framework.exceptions.FrameworkException('HTTPS URLs are not currently supported')
    return xmlrpclib.ServerProxy(url, FrameworkTransport(self._sandbox), encoding, use_datetime=True)
  


class HTTPKit(BaseHTTPKit):
  
  # It'd be nicer to have separate cache times for the API and the internal components, but we have
  # to set it in the networking component so the same setting gets used by all HTTP-accessing APIs. 
  @property
  def CacheTime(self):
    """
      The default cache time (in seconds) used for all HTTP requests without a specific cache time set. By default, this value is 0.
      
      :type: float
    """
    return self._core.networking.cache_time
    

  @CacheTime.setter
  def CacheTime(self, value):
    self._core.networking.cache_time = value
    

  @property
  def Headers(self):
    """
      A dictionary containing the default HTTP headers that should be issued with requests::
      
        HTTP.Headers['User-Agent'] = 'My Plug-in'
    """
    return self._sandbox.custom_headers
    

  #Deprecated
  @BaseKit._exclude_from(Framework.policies.ModernPolicy)
  def SetCacheTime(self, cacheTime):
    self._core.log.warn('The HTTP.SetCacheTime() function is deprecated. Use the HTTP.CacheTime property instead.')
    self._core.networking.cache_time = cacheTime
    

  #Deprecated
  @BaseKit._exclude_from(Framework.policies.ModernPolicy)
  def SetHeader(self, header, value):
    self._core.log.warn('The HTTP.SetHeader() function is deprecated. Use HTTP.Headers[] to get and set headers instead.')
    self.Headers[header] = value
    

  #Deprecated
  @BaseKit._exclude_from(Framework.policies.ModernPolicy)
  def SetTimeout(self, timeout):
    self._core.log.warn('The HTTP.SetTimeout() function is deprecated. Use the Network.Timeout property instead.')
    self._core.networking.default_timeout = timeout
    

  def Request(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, immediate=False, sleep=0, data=None, follow_redirects=True, method=None):
    """
      Creates and returns a new :class:`HTTPRequest` object.
      
      :arg url: The URL to use for the request.
      :type url: str
      
      :arg values: Keys and values to be URL encoded and provided as the request's POST body.
      :type values: dict
      
      :arg headers: Any custom HTTP headers that should be added to this request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in second) of cached data before it should be considered invalid.
      :type cacheTime: int
      
      :arg encoding: The string encoding to use for the downloaded data. If no encoding is provided, the framework will attempt to guess the encoding.
      :type encoding: str
      
      :arg errors: The error handling method to use. If *errors* is `'strict'` (the default), a `ValueError` is raised on errors, while a value of `'ignore'` causes errors to be silently ignored, and a value of `'replace'` causes the official Unicode replacement character, U+FFFD, to be used to replace input characters which cannot be decoded.
      :type errors: str
      
      :arg timeout: The maximum amount of time (in seconds) to wait for the request to return a response before timing out.
      :type timeout: float
      
      :arg immediate: If set to ``True``, the HTTP request will be made immediately when the object is created (by default, requests are delayed until the data they return is requested).
      :type immediate: bool
      
      :arg sleep: The amount of time (in seconds) that the current thread should be paused for after issuing a HTTP request. This is to ensure undue burden is not placed on the server. If the data is retrieved from the cache, this value is ignored.
      :type sleep: float
      
      :arg data: The raw POST data that should be sent with the request. This attribute cannot be used in conjunction with *values*.
      :type data: str

      :arg follow_redirects: Specifies whether redirects should be followed, or if an exception should be raised. If False, the framework will raise a RedirectError when encountering a redirected response.
      :type follow_redirects: bool
    """
    
    # Update the cache time
    self._context.cache_time = cacheTime if (values == None and data == None) else 0
            
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
    all_headers = self._add_headers(headers)
    
    return self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout = timeout,
      immediate = immediate,
      sleep=sleep,
      data=data,
      opener=self._opener,
      follow_redirects=follow_redirects,
      method=method,
    )
    

  def CookiesForURL(self, url):
    """
      Returns the cookies associated with the given URL.
    """
    cookie_jar = self._context.cookie_jar
    cookies = self._core.networking.get_cookies_for_url(url, cookie_jar=cookie_jar)
    if cookies:
      return cookies
    elif cookie_jar:
      return self._core.networking.get_cookies_for_url(url)
    

  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def GetCookiesForURL(self, url):
    self._core.log.warn("HTTP.GetCookiesForURL() is deprecated - please use HTTP.CookiesForURL() instead.")
    return self.CookiesForURL(url)
    

  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def SetPassword(self, url, username, password, realm=None):
    return self._core.networking.set_http_password(url, username, password, realm)
    

  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def PreCache(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None):
    """
      Instructs the framework to pre-cache the result of a given HTTP request in a background
      thread. This method returns nothing - it is designed to ensure that cached data is
      available for future calls to *HTTP.Request*.
    """
    self._core.runtime.create_thread(self._precache, url=url, values=values, headers=headers, cacheTime=cacheTime, encoding=encoding, errors=errors)
    
  def _precache(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None):
    self.Request(url=url, values=values, headers=headers, cacheTime=cacheTime, encoding=encoding, errors=errors, immediate=True)
    

  @property
  def Cookies(self):
    """
      Returns an iterable object containing all cookies
    """
    return self._core.networking._cookie_jar
    

  def ClearCookies(self):
    """
      Clears the plug-in's HTTP cookies.
    """
    self._core.networking.clear_cookies(cookie_jar=self._context.cookie_jar)
    

  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def ClearCache(self):
    """
      Clears the plug-in's HTTP cache.
    """
    self._core.networking.clear_http_cache()
    

  def RandomizeUserAgent(self, browser=None):
    """
      Random user agents are not supported. This function should no longer be used.
    """
    self._core.log.error("Randomized user agent strings are no longer supported.")

  


class NetworkKit(BaseKit):
  
  _included_policies = [
    Framework.policies.CodePolicy,
  ]

  _children = [
    HTTPKit,
    XMLRPCKit,
  ]
    
  @property
  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def Timeout(self):
    return self._core.networking.default_timeout

  @Timeout.setter
  def Timeout(self, value):
    self._core.networking.default_timeout = value
    
  @property
  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def Address(self):
    """
      Returns the server's IP address. Unless the server is connected directly to the internet,
      this will usually be a local IP address.
      
      :rtype: str
    """
    return self._core.networking.address
  
  @property
  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def PublicAddress(self):
    """
      Returns the public IP address of the network the server is currently connected to.
      
      :rtype: str
    """
    return self._core.networking.http_request("http://www.plexapp.com/ip.php", cacheTime=7200).content.strip()
  
  @property
  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def Hostname(self):
    """
      Returns the server's hostname.
      
      :rtype: str
    """
    return self._core.networking.hostname
    
  @BaseKit._exclude_from(Framework.policies.CloudPolicy)
  def Socket(self):
    """
      Creates a new *socket* object.
      
      :rtype: `socket <http://docs.python.org/library/socket.html#socket-objects>`_
    """
    return self._core.networking.socket()
