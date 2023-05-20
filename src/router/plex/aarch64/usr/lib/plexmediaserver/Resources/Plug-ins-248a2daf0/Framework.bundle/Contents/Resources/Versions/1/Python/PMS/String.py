#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import base64, urllib, XML, string

####################################################################################################

def Encode(string):
  return base64.b64encode(string).replace('/','@').replace('+','*').replace('=','_')

####################################################################################################

def Decode(string):
  return base64.b64decode(string.replace('@','/').replace('*','+').replace('_','='))
    
####################################################################################################

def Quote(url, usePlus=False):
  if usePlus:
    return urllib.quote_plus(url)
  else:
    return urllib.quote(url)

####################################################################################################
    
def URLEncode(string):
  encoded = urllib.urlencode({'v':string})
  return encoded[2:]

####################################################################################################
    
def Unquote(url, usePlus=False):
  if usePlus:
    return urllib.unquote_plus(url)
  else:
    return urllib.unquote(url)
    
####################################################################################################

def StripTags(html, encoding="utf8"):
  h = XML.ElementFromString(html, isHTML=True)
  return XML.StringFromElement(h, encoding=encoding, method="text")

####################################################################################################

def Join(words, sep=None):
  return string.join(words, sep)

####################################################################################################