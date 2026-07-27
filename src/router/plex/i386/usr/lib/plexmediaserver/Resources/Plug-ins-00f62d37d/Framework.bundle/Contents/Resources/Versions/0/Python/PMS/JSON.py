#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  Functions for handling JSON content.
"""

import demjson as json
import HTTP

####################################################################################################

def DictFromString(str, encoding="utf8"):
  """
    Return a dictionary by parsing the given JSON string with the given encoding.
    
    @param str: The JSON string to parse
    @type str: string
    @return: dictionary
  """
  if str is None: return None
  return json.decode(str.decode(encoding))
  
####################################################################################################
  
def DictFromURL(url, encoding="utf8"):
  """
    Return a dictionary by parsing the JSON response with the given encoding from a given URL.
    
    @param url: The URL to request data from
    @type url: string
    @return: dictionary
  """
  return DictFromString(HTTP.Get(url), encoding)
  
####################################################################################################

def DictFromFile(fileName, encoding="utf8"):
  """
    Return a dictionary by parsing the JSON content of a given file.
    
    @param fileName: The file to load
    @type fileName: string
    @return: dictionary
  """
  f = open(fileName, "r")
  d = f.read()
  f.close()
  return DictFromString(d, encoding)

####################################################################################################

def StringFromDict(dictionary):
  """
    Return a JSON-formatted string representing the given dictionary.
    
    @param dictionary: The dictionary to use
    @type dictionary: dictionary
    @return: string
  """
  return json.encode(dictionary)
  
####################################################################################################
