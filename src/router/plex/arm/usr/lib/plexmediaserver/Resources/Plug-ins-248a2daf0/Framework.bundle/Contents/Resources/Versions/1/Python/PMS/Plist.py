import plistlib
import HTTP

def ObjectFromString(string):
  return plistlib.readPlistFromString(string)
  
def StringFromObject(obj):
  return plistlib.writePlistToString(obj)
  
def ObjectFromURL(url, values=None, headers={}, cacheTime=None, autoUpdate=False, encoding=None, errors=None):
  return ObjectFromString(HTTP.Request(url, values=values, headers=headers, cacheTime=cacheTime, autoUpdate=autoUpdate, encoding=encoding, errors=errors))
