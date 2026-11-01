#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import simplejson
import demjson
import HTTP
import PMS

####################################################################################################

def ObjectFromString(string, encoding=None, errors=None):
  if string is None: return None
  if not (encoding is None and errors is None):
    if encoding is None: encoding = "utf8"
    if errors is None: errors = "strict"
    string = string.encode(encoding, errors)
    
  try:
    obj = simplejson.loads(string)
  except:
    obj = demjson.decode(string)
  
  return obj
  
####################################################################################################
  
def ObjectFromURL(url, values=None, headers={}, cacheTime=None, autoUpdate=False, encoding=None, errors=None):
  return ObjectFromString(HTTP.Request(url, values=values, headers=headers, cacheTime=cacheTime, autoUpdate=autoUpdate, encoding=encoding, errors=errors), encoding=encoding, errors=errors)

####################################################################################################

def StringFromObject(obj):
  try:
    jsonstr = simplejson.dumps(obj)
  except:
    jsonstr = demjson.encode(obj)
  return jsonstr
  
####################################################################################################
