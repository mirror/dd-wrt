#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  Functions for interacting with web services.
"""
import urllib, urllib2, cookielib, os.path, datetime, socket
import Plugin, Log

__cookieFile = "/CookieJar"
__cookieJar = cookielib.LWPCookieJar()
__headers =  {'User-Agent' : 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'}

def SetTimeout(timeout):
  socket.setdefaulttimeout(timeout)
  Log.Add("(Framework) Set the default socket timeout to %.1f seconds" % timeout)

####################################################################################################

def __loadCookieJar():
  #
  # Load the CookieJar file from the plugin's data directory
  #
  path = Plugin.DataPath + __cookieFile
  if os.path.isfile(path):
    __cookieJar.load(path)

  opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(__cookieJar))
  urllib2.install_opener(opener)

####################################################################################################

def Download(url, localFile, headers={}):
  """
    Downloads a file from the given URL and saves it to the given local path.
    
    @param url: The URL to request data from
    @type url: string
    @param localFile: The local path to save data to
    @type localFile: string
    @param headers: Extra headers to send when making the request (optional)
    @type headers: dictionary
    @return: string
  """
  try:
    data = Get(url, headers)
    f = open(localFile, 'w')
    f.write(data)
    f.close()
    return True
  except:
    return False
  
####################################################################################################

def Get(url, headers={}):
  """
    Returns the result of a HTTP GET request using the given URL
    
    @param url: The URL to request data from
    @type url: string
    @param headers: Extra headers to send when making the request (optional)
    @type headers: dictionary
    @return: string
  """
  return Post(url, None, headers)

####################################################################################################  

def GetCached(url, cacheTime=1800, headers={}, forceUpdate=False):
  if not Plugin.Dict.has_key("_PMS.HTTP.Cache"):
    Plugin.Dict["_PMS.HTTP.Cache"] = {}
    
  cache = Plugin.Dict["_PMS.HTTP.Cache"]
  now = datetime.datetime.now()
  if cache.has_key(url):
    cachedAt = cache[url]["CheckTime"]
    expiresAt = cachedAt + datetime.timedelta(seconds=cacheTime)
    if datetime.datetime.now() < expiresAt and forceUpdate == False:
      Log.Add("(Framework) Loaded '%s' from the cache" % url)
      return cache[url]["Content"]

  content = Get(url, headers)
  if content is not None:
    cache[url] = {}
    cache[url]["Content"] = content
    cache[url]["CheckTime"] = now
    Plugin.Dict["_PMS.HTTP.Cache"] = cache
    Plugin.Dict["_PMS.HTTP.CacheTime"] = datetime.datetime.now()
    Log.Add("(Framework) Saved '%s' to the cache" % url)
    return content
  else:
    Log.Add("(Framework) Couldn't cache '%s'" % url)
    return None
  

####################################################################################################

def Post(url, values, headers={}):
  """
    Returns the result of a HTTP POST request using the given URL and values
    
    @param url: The URL to request data from
    @type url: string
    @param values: The values to post as part of the request
    @type values: dictionary
    @param headers: Extra headers to send when making the request (optional)
    @type headers: dictionary
    @return: string
  """
  response = Open(url, values)
  if response is not None: return response.read()
  else: return None
    
####################################################################################################

def Open(url, values=None, headers={}):
  """
    Returns the response of a HTTP request using the given URL and values
    
    @param url: The URL to request data from
    @type url: string
    @param values: The values to post as part of the request
    @type values: dictionary
    @param headers: Extra headers to send when making the request (optional)
    @type headers: dictionary
    @return: string
  """
  # TODO: Add gzip support
  try:
    data = None
    if values is not None: data = urllib.urlencode(values)
    h = __headers.copy()
    for header in headers:
      h[header] = headers[header]
    request = urllib2.Request(url, data, __headers)
    response = urllib2.urlopen(request)
    if __cookieJar is not None:
      __cookieJar.save(Plugin.DataPath + __cookieFile)
    return response
  except urllib2.HTTPError:
    Log.Add("(Framework) HTTPError when requesting '%s'" % url)
    return None
  except urllib2.URLError:
    Log.Add("(Framework) URLError when requesting '%s'" % url)
    return None

####################################################################################################

def BuildURL(prefix, nouns):
  if prefix[-1] != "/": prefix += "/"
  url = prefix
  if len(nouns) > 0:
    url += nouns[0]
    if len(nouns) > 1:
      for i in range(len(nouns)-1):
        url += "/%s" % nouns[i+1]
  if url[-1] == "/": url = url[:-1]
  return url

####################################################################################################

def Quote(url, usePlus=False):
  if usePlus:
    return urllib.quote_plus(url)
  else:
    return urllib.quote(url)

####################################################################################################
    
def Unquote(url, usePlus=False):
  if usePlus:
    return urllib.unquote_plus(url)
  else:
    return urllib.unquote(url)
    
####################################################################################################
