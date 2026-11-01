#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import urllib, urllib2, cookielib, os, StringIO, gzip, socket
import PMS, Data, Datetime, Thread

####################################################################################################

__cacheTime   = 0
__cookieJar   = cookielib.MozillaCookieJar()
__passMgr     = urllib2.HTTPPasswordMgrWithDefaultRealm()
__authHandler = urllib2.HTTPBasicAuthHandler(__passMgr)
__cache       = {}
__headers     = {"User-Agent" : "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_6; en-us) AppleWebKit/530.1+ (KHTML, like Gecko) Version/3.2.1 Safari/525.27.1", "Accept-encoding" : "gzip"}
__saveScheduled = False
__autoUpdateCacheTime = 60

def SetTimeout(timeout):
  socket.setdefaulttimeout(timeout)
  PMS.Log("(Framework) Set the default socket timeout to %.1f seconds" % timeout)

####################################################################################################

def __loadCookieJar():
  path = "%s/HTTPCookies" % Data.__dataPath
  if os.path.isfile(path):
    __cookieJar.load(path)
    PMS.Log("(Framework) Loaded HTTP cookies")
  else:
    PMS.Log("(Framework) No cookie jar found")

  # Build & install an opener with the cookie jar & auth handler
  opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(__cookieJar), __authHandler)
  urllib2.install_opener(opener)

####################################################################################################  
  
def __loadCache():
  global __cache
  path = "%s/HTTPCache" % Data.__dataPath
  if os.path.exists(path):
    try:
      __cache = Data.__unpickle(path)
      PMS.Log("(Framework) Loaded HTTP cache")
    except:
      __cache = {}
      PMS.Log("(Framework) An error occurred when loading the HTTP cache")

def __save():
  global __saveScheduled
  Thread.Lock("Framework.HTTPCache", addToLog=False)
  try:
    # Save the cache
    Data.__pickle("%s/HTTPCache" % Data.__dataPath, __cache)
    
    # Save the cookie jar
    if __cookieJar is not None:
      __cookieJar.save("%s/HTTPCookies" % Data.__dataPath)

  finally:
    __saveScheduled = False
    Thread.Unlock("Framework.HTTPCache", addToLog=False)
    PMS.Log("(Framework) Saved shared HTTP data")

####################################################################################################    
####################################################################################################

def SetHeader(key, value):
  global __headers
  __headers[key] = value

####################################################################################################
  
def SetCacheTime(cacheTime):
  global __cacheTime
  __cacheTime = cacheTime

####################################################################################################

def SetPassword(url, username, password, realm=None):
  global __passMgr
  # Strip http:// from the beginning of the url
  if url[0:6] == "http://":
    url = url[7:]
  __passMgr.add_password(realm, url, username, password)

####################################################################################################

def GetCookiesForURL(url):
  request = urllib2.Request(url)
  __cookieJar.add_cookie_header(request)
  if request.unredirected_hdrs.has_key('Cookie'):
    return request.unredirected_hdrs['Cookie']

  return None

####################################################################################################

def PreCache(url, values=None, headers={}, cacheTime=None, autoUpdate=False, encoding=None, errors=None):
  Thread.Create(Request, url=url, values=values, headers=headers, cacheTime=cacheTime, autoUpdate=autoUpdate, encoding=encoding, errors=errors)

####################################################################################################

def Request(url, values=None, headers={}, cacheTime=None, autoUpdate=False, encoding=None, errors=None, addToLog=True):
  global __cache
  global __saveScheduled
  now = Datetime.Now()
  
  # If no cache time is given, use the default
  if cacheTime is None:
    cacheTime = __cacheTime
  
  # Attempt to return a cached copy, fetching again if an exception occurs
  try:
    # Make sure we don't cache POST requests
    if values == None and cacheTime > 0:
      if __cache.has_key(url):
        cachedAt = __cache[url]["CheckTime"]
        expiresAt = cachedAt + Datetime.Delta(seconds=cacheTime)
        if Datetime.Now() < expiresAt:
          if addToLog: PMS.Log("(Framework) Loaded %s from the cache (expires at %s)" % (url, expiresAt))
          return __cache[url]["Content"]
  except:
    if addToLog: PMS.Log("(Framework) Couldn't load %s from the cache, attempting to fetch again.")
    
  # Try to fetch the page from the server
  try:
    # Encode the values
    data = None
    if values is not None: data = urllib.urlencode(values)
    h = __headers.copy()
    for header in headers:
      h[header] = headers[header]
    request = urllib2.Request(url, data, h)
    f = urllib2.urlopen(request)
    response = f.read()
    
    # If the response is gzipped, unzip it
    if f.headers.get('Content-Encoding') == "gzip":
      if addToLog: PMS.Log("(Framework) Received gzipped response from %s" % url)
      stream = StringIO.StringIO(response)
      gzipper = gzip.GzipFile(fileobj=stream)
      response = gzipper.read()
    else:
      if addToLog: PMS.Log("(Framework) Received response from %s" % url)
      
    # Try to decode the response if manually specified
    try:
      if not (encoding is None and errors is None):
        if encoding is None: encoding = "utf8"
        if errors is None: errors = "strict"
        response = str(response.decode(encoding, errors).encode("utf8", errors))
    except:
      if addToLog: PMS.Log("(Framework) Unable to decode response from '%s' with codec %s" % (url, encoding))

  # Handle common errors
  except urllib2.HTTPError:
    PMS.Log("(Framework) HTTPError when requesting '%s'" % url)
    return None
  except urllib2.URLError:
    PMS.Log("(Framework) URLError when requesting '%s'" % url)
    return None

  Thread.Lock("Framework.HTTPCache", addToLog=False)
  try:
    # Cache the data if required
    if response is not None and cacheTime > 0:
      item = {}
      item["Content"] = response
      item["CheckTime"] = Datetime.Now()
      if autoUpdate:
        item["UpdateTime"] = Datetime.Now() + Datetime.Delta(seconds=cacheTime)
        item["CacheTime"] = cacheTime
        item["Headers"] = headers
        item["Encoding"] = encoding
        item["Errors"] = errors
      __cache[url] = item
      if addToLog: PMS.Log("(Framework) Cached response from %s" % url)
      
    
    if not __saveScheduled:
      Thread.CreateTimer(5, __save)
      __saveScheduled = True
  finally:
    Thread.Unlock("Framework.HTTPCache", addToLog=False)
      
  # Return the data
  return response

####################################################################################################

def __autoUpdateCachedPages():
  global __cache
  for url in __cache:
    item = __cache[url]
    if item.has_key("UpdateTime"):
      # Check whether the page would expire before the next cache update is triggered
      if item["UpdateTime"] < (Datetime.Now() + Datetime.Delta(seconds=__autoUpdateCacheTime)):
        if item.has_key("Headers"):
          headers = item["Headers"]
        else:
          headers = {}
        PMS.Log("(Framework) Automatically updating the cached copy of %s" % url)
        Request(url, headers=headers, cacheTime=item["CacheTime"], autoUpdate=True, encoding=item["Encoding"], errors=item["Errors"], addToLog=False)

####################################################################################################
