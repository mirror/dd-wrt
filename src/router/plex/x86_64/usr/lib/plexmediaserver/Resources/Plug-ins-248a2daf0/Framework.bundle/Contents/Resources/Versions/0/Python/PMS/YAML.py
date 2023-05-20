#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  WIP: Functions for handling YAML content
"""

import yaml
import HTTP

####################################################################################################

def DictFromString(str):
  """
    Return a dictionary by parsing the given YAML string.
    
    @param str: The JSON string to parse
    @type str: string
    @return: dictionary
  """
  return yaml.load(str)

####################################################################################################

def DictFromURL(url):
  """
    Return a dictionary by parsing the YAML response from a given URL.
    
    @param url: The URL to request data from
    @type url: string
    @return: dictionary
  """
  return DictFromString(HTTP.Get(str))
  
####################################################################################################
