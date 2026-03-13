#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import yaml
import HTTP

####################################################################################################

def ObjectFromString(string):
  return yaml.load(string)

####################################################################################################

def ObjectFromURL(url, values=None, headers={}, cacheTime=None, autoUpdate=False, encoding=None, errors=None):
  return ObjectFromString(HTTP.Request(url, values=vaules, headers=headers, cacheTime=cacheTime, autoUpdate=autoUpdate, encoding=encoding, errors=errors))
  
####################################################################################################
