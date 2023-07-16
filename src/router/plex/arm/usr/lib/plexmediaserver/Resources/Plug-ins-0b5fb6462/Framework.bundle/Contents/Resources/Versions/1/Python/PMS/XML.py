#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from lxml import etree, html, objectify
from lxml.html.soupparser import fromstring
import HTTP, Plugin

####################################################################################################

def Element(name, text=None, **kwargs):
  el = etree.Element(name)
  if text is not None:
    el.text = text
  for key in kwargs:
    el.set(key, kwargs[key])
  return el

####################################################################################################
  
def ElementFromURL(url, isHTML=False, values=None, headers={}, cacheTime=None, autoUpdate=False, encoding=None, errors=None):
  return ElementFromString(HTTP.Request(url, values, headers, cacheTime, autoUpdate, encoding, errors), isHTML)

####################################################################################################

def ElementFromString(string, isHTML=False):
  if string is None: return None
  #xmlstring = string.encode("utf8")
  if isHTML:
    try:
      root = html.fromstring(string)
      return root
    except:
      return fromstring(string)
  else:
    return etree.fromstring(string)
  
####################################################################################################

def StringFromElement(el, encoding="utf8", method=None):
  if type(el).__name__ == "HtmlElement":
    if method == None: method = "html"
    return html.tostring(el, method=method, encoding=encoding)
    
  if method == None: method = "xml"
  return etree.tostring(el, pretty_print=Plugin.Debug, encoding=encoding)

####################################################################################################

def ObjectFromString(string):
  return objectify.fromstring(string)

####################################################################################################

def ObjectFromURL(url, values=None, headers={}, cacheTime=None, autoUpdate=False, encoding=None, errors=None):
  return ObjectFromString(HTTP.Request(url, values, headers, cacheTime, autoUpdate, encoding, errors))

####################################################################################################

def StringFromObject(obj, encoding="utf8"):
  return etree.tostring(obj, pretty_print=Plugin.Debug, encoding=encoding)

####################################################################################################
