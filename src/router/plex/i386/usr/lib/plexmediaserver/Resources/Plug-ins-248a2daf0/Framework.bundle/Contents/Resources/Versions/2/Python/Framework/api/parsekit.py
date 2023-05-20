#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import plistlib

feedparser = Framework.LazyModule('feedparser')

from base import BaseKit
from networkkit import BaseHTTPKit

GLOBAL_DEFAULT_TIMEOUT = Framework.components.networking.GLOBAL_DEFAULT_TIMEOUT
DEFAULT_MAX_SIZE = 1024 * 1024 * 5

def check_size(data, max_size):
  if max_size == None:
    max_size = DEFAULT_MAX_SIZE
  
  # Make sure we don't try to parse anything greater than the given maximum size.
  if len(data) > max_size:
    raise Framework.exceptions.APIException("Data of size %d is greater than the maximum size %d" % (len(data), max_size))
    

class PlistKit(BaseHTTPKit):
  
  def ObjectFromString(self, string, max_size=None):
    """
      Returns an object representing the given Plist-formatted string.
    """
    check_size(string, max_size)
    return plistlib.readPlistFromString(string)
    
  def ObjectFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, sleep=0, follow_redirects=True, method=None, max_size=None):
    """
      Retrieves the content for a given HTTP request and parses it as a Plist using the above method.
      
      :arg url: The URL to retrieve content from.
      :type url: str
      
      :arg values: Values to pass as URL encoded content for a POST request.
      :type values: dict
      
      :arg headers: Custom HTTP headers to add to the request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in seconds) that cached data should still be considered valid.
      :type cacheTime: float
      
      :arg timeout: The maximum amount of time (in seconds) that the framework should wait for a response before aborting.
      :type timeout: float
      
      :arg sleep: The number of seconds the current thread should pause for if a network request was made, ensuring undue burden isn't placed on web servers. If cached data was used, this value is ignored.
      :type sleep: float

      :arg follow_redirects: Specifies whether redirects should be followed, or if an exception should be raised. If False, the framework will raise a RedirectError when encountering a redirected response.
      :type follow_redirects: bool
    """
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
    
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0

    all_headers = self._add_headers(headers)
    return self.ObjectFromString(self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout = timeout,
      immediate = True,
      sleep = sleep,
      opener = self._opener,
      follow_redirects=follow_redirects,
      method=method,
    ).content, max_size=max_size)
    
  def StringFromObject(self, obj):
    """
      Converts a given object to a Plist-formatted string representation.
    """
    return plistlib.writePlistToString(obj)



class JSONKit(BaseHTTPKit):
  
  def ObjectFromString(self, string, encoding=None, max_size=None):
    """
      Converts a JSON-formatted string into a Python object, usually a dictionary.
    """
    check_size(string, max_size)
    return self._core.data.json.from_string(string, encoding)
  
  def ObjectFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, sleep=0, follow_redirects=True, method=None, max_size=None):
    """
      Retrieves the content for a given HTTP request and parses it as JSON-formatted content using the above method.
      
      :arg url: The URL to retrieve content from.
      :type url: str
      
      :arg values: Values to pass as URL encoded content for a POST request.
      :type values: dict
      
      :arg headers: Custom HTTP headers to add to the request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in seconds) that cached data should still be considered valid.
      :type cacheTime: float
      
      :arg timeout: The maximum amount of time (in seconds) that the framework should wait for a response before aborting.
      :type timeout: float
      
      :arg sleep: The number of seconds the current thread should pause for if a network request was made, ensuring undue burden isn't placed on web servers. If cached data was used, this value is ignored.
      :type sleep: float

      :arg follow_redirects: Specifies whether redirects should be followed, or if an exception should be raised. If False, the framework will raise a RedirectError when encountering a redirected response.
      :type follow_redirects: bool
    """
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
    
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0
    
    all_headers = {'Accept': 'text/*, application/json'}
    all_headers.update(self._add_headers(headers))

    return self.ObjectFromString(self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout = timeout,
      immediate = True,
      sleep = sleep,
      opener = self._opener,
      follow_redirects=follow_redirects,
      method=method,
    ).content, encoding, max_size=max_size)
    
  def StringFromObject(self, obj):
    """
      Converts the given object to a JSON-formatted string representation.
    """
    return self._core.data.json.to_string(obj)
    
    

class RSSKit(BaseHTTPKit):

  def FeedFromString(self, string, max_size=None):
    """
      Parses the given string as an RSS, RDF or ATOM feed (automatically detected).
    """
    check_size(string, max_size)
    return feedparser.parse(string)

  def FeedFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, sleep=0, follow_redirects=True, method=None, max_size=None):
    """
      Retrieves the content for a given HTTP request and parses it as an RSS, RDF or ATOM feed using the above method.
      
      :arg url: The URL to retrieve content from.
      :type url: str
      
      :arg values: Values to pass as URL encoded content for a POST request.
      :type values: dict
      
      :arg headers: Custom HTTP headers to add to the request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in seconds) that cached data should still be considered valid.
      :type cacheTime: float
      
      :arg timeout: The maximum amount of time (in seconds) that the framework should wait for a response before aborting.
      :type timeout: float
      
      :arg sleep: The number of seconds the current thread should pause for if a network request was made, ensuring undue burden isn't placed on web servers. If cached data was used, this value is ignored.
      :type sleep: float

      :arg follow_redirects: Specifies whether redirects should be followed, or if an exception should be raised. If False, the framework will raise a RedirectError when encountering a redirected response.
      :type follow_redirects: bool
    """
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
  
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0
      
    all_headers = self._add_headers(headers)
    return self.FeedFromString(self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout = timeout,
      immediate = True,
      sleep = sleep,
      opener = self._opener,
      follow_redirects=follow_redirects,
      method=method,
    ).content, max_size=max_size)
    


class YAMLKit(BaseHTTPKit):
  
  def ObjectFromString(self, string, max_size=None):
    """
      Parses the given YAML-formatted string and returns the object it represents.
    """
    check_size(string, max_size)
    obj = yaml.load(string)
    return obj

  def ObjectFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, sleep=0, follow_redirects=True, method=None, max_size=None):
    """
      Retrieves the content for a given HTTP request and parses it as YAML-formatted content using the above method.
      
      :arg url: The URL to retrieve content from.
      :type url: str
      
      :arg values: Values to pass as URL encoded content for a POST request.
      :type values: dict
      
      :arg headers: Custom HTTP headers to add to the request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in seconds) that cached data should still be considered valid.
      :type cacheTime: float
      
      :arg timeout: The maximum amount of time (in seconds) that the framework should wait for a response before aborting.
      :type timeout: float
      
      :arg sleep: The number of seconds the current thread should pause for if a network request was made, ensuring undue burden isn't placed on web servers. If cached data was used, this value is ignored.
      :type sleep: float

      :arg follow_redirects: Specifies whether redirects should be followed, or if an exception should be raised. If False, the framework will raise a RedirectError when encountering a redirected response.
      :type follow_redirects: bool
    """
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
    
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0
    
    all_headers = self._add_headers(headers)  
    return self.ObjectFromString(self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout = timeout,
      immediate = True,
      sleep = sleep,
      opener = self._opener,
      follow_redirects=follow_redirects,
      method=method,
    ).content, max_size=max_size)
    
    
    
class XMLKit(BaseHTTPKit):
  def Element(self, name, text=None, **kwargs):
    """
      Returns a new XML element with the given name and text content. Any keyword arguments provided will be set as attributes.
      
      :arg name: The name of the new element.
      :type name: str
      
      :arg text: The text content of the new element.
      :type text: str
      
      :rtype: `_Element <http://lxml.de/api/lxml.etree._Element-class.html>`_
    """
    return self._core.data.xml.element(name, text, **kwargs)

  def StringFromElement(self, el, encoding='utf8'):
    """
      Converts the XML element object *el* to a string representation using the given encoding.
    """
    return self._core.data.xml.to_string(el, encoding)

  def ElementFromString(self, string, encoding=None, max_size=None):
    """
      Converts *string* to an XML element object.
      
      :rtype: `_Element <http://lxml.de/api/lxml.etree._Element-class.html>`_
    """
    check_size(string, max_size)
    return self._core.data.xml.from_string(string, encoding = encoding)

  def ElementFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, sleep=0, follow_redirects=True, method=None, max_size=None):
    """
      Retrieves the content for a given HTTP request and parses it as XML using the above method.
      
      :arg url: The URL to retrieve content from.
      :type url: str
      
      :arg values: Values to pass as URL encoded content for a POST request.
      :type values: dict
      
      :arg headers: Custom HTTP headers to add to the request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in seconds) that cached data should still be considered valid.
      :type cacheTime: float
      
      :arg timeout: The maximum amount of time (in seconds) that the framework should wait for a response before aborting.
      :type timeout: float
      
      :arg sleep: The number of seconds the current thread should pause for if a network request was made, ensuring undue burden isn't placed on web servers. If cached data was used, this value is ignored.
      :type sleep: float
    """
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
    
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0
    
    all_headers = self._add_headers(headers)  
    return self.ElementFromString(self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout=timeout,
      immediate = True,
      sleep = sleep,
      opener = self._opener,
      follow_redirects=follow_redirects,
      method=method,
    ).content, encoding=encoding, max_size=max_size)
    
  def ObjectFromString(self, string, max_size=None):
    """
      Parses *string* as XML-formatted content and attempts to build a Python object using the objectify library.
      
      :rtype: `ObjectifiedElement <http://lxml.de/api/lxml.objectify.ObjectifiedElement-class.html>`_
    """
    check_size(string, max_size)
    return self._core.data.xml.object_from_string(string)
    
  def StringFromObject(self, obj, encoding='utf-8'):
    """
      Attempts to create objectified XML from the given object.
    """
    return self._core.data.xml.object_to_string(obj, encoding)
    
  def ObjectFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, sleep=0, max_size=None):
    """
      Retrieves the content for a given HTTP request and parses it as objectified XML using the above method.
      
      :arg url: The URL to retrieve content from.
      :type url: str
      
      :arg values: Values to pass as URL encoded content for a POST request.
      :type values: dict
      
      :arg headers: Custom HTTP headers to add to the request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in seconds) that cached data should still be considered valid.
      :type cacheTime: float
      
      :arg timeout: The maximum amount of time (in seconds) that the framework should wait for a response before aborting.
      :type timeout: float
      
      :arg sleep: The number of seconds the current thread should pause for if a network request was made, ensuring undue burden isn't placed on web servers. If cached data was used, this value is ignored.
      :type sleep: float

      :arg follow_redirects: Specifies whether redirects should be followed, or if an exception should be raised. If False, the framework will raise a RedirectError when encountering a redirected response.
      :type follow_redirects: bool
    """
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
    
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0
    
    all_headers = self._add_headers(headers)  
    return self.ObjectFromString(self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout = timeout,
      immediate = True,
      sleep = sleep,
      opener = self._opener).content, max_size=max_size)
    
    
    
class HTMLKit(BaseHTTPKit):
  def Element(self, name, text=None, **kwargs):
    """
      Returns a new HTML element with the given name and text content. Any keyword arguments provided will be set as attributes.
      
      :arg name: The name of the new element.
      :type name: str
      
      :arg text: The text content of the new element.
      :type text: str
      
      :rtype: `HtmlElement <http://lxml.de/api/lxml.html.HtmlElement-class.html>`_
    """
    return self._core.data.xml.html_element(name, text, **kwargs)

  def StringFromElement(self, el, encoding='utf8'):
    """
      Converts the HTML element object *el* to a string representation using the given encoding.
    """
    return self._core.data.xml.to_string(el, encoding, 'html')

  def ElementFromString(self, string, max_size=None):
    """
      Converts *string* to a HTML element object.
      
      :rtype: `HtmlElement <http://lxml.de/api/lxml.html.HtmlElement-class.html>`_
    """
    check_size(string, max_size)
    return self._core.data.xml.from_string(string, isHTML=True)

  def ElementFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, timeout=GLOBAL_DEFAULT_TIMEOUT, sleep=0, follow_redirects=True, method=None, max_size=None):
    """
      Retrieves the content for a given HTTP request and parses it as HTML using the above method.
      
      :arg url: The URL to retrieve content from.
      :type url: str
      
      :arg values: Values to pass as URL encoded content for a POST request.
      :type values: dict
      
      :arg headers: Custom HTTP headers to add to the request.
      :type headers: dict
      
      :arg cacheTime: The maximum age (in seconds) that cached data should still be considered valid.
      :type cacheTime: float
      
      :arg timeout: The maximum amount of time (in seconds) that the framework should wait for a response before aborting.
      :type timeout: float
      
      :arg sleep: The number of seconds the current thread should pause for if a network request was made, ensuring undue burden isn't placed on web servers. If cached data was used, this value is ignored.
      :type sleep: float

      :arg follow_redirects: Specifies whether redirects should be followed, or if an exception should be raised. If False, the framework will raise a RedirectError when encountering a redirected response.
      :type follow_redirects: bool
    """
    if url.find(':32400/') > -1 and self._sandbox.policy.elevated_execution == False:
      raise Framework.exceptions.FrameworkException("Accessing the media server's HTTP interface is not permitted.")
    
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0
        
    all_headers = self._add_headers(headers)
    return self.ElementFromString(self._http_request(
      url = url,
      values = values,
      headers = all_headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      timeout = timeout,
      immediate = True,
      sleep = sleep,
      opener = self._opener,
      follow_redirects=follow_redirects,
      method=method,
    ).content, max_size=max_size)
  
  
class ParseKit(BaseKit):

  _included_policies = [
    Framework.policies.CodePolicy
  ]
  
  _root_object = False
  
  def _init(self):
    self._publish(JSONKit)
    self._publish(PlistKit)
    self._publish(RSSKit)
    self._publish(YAMLKit)
    self._publish(XMLKit)
    self._publish(HTMLKit)
    
