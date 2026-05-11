#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  WIP: Functions for storing and retrieving preferences.
"""

# TODO: Add support for read/write-only prefs

import Plugin, XML, Log

__publicPrefs = {}

####################################################################################################    

def Get(key):
  """
    Gets the preference value for the given key.
    
    @param key: The key of the preference
    @type key: string
    @return: key
  """
  return XML.TextFromElement(Plugin.__prefs.find(key))

####################################################################################################

def Set(key, value):
  """
    Sets the preference for the given key to the given value.
    
    @param key: The key of the preference
    @type key: string
    @param value: The value to set
    @type value: string
  """
  el = Plugin.__prefs.find(key)
  if el is None:
    el = XML.Element(key)
    el.text = value
    Plugin.__prefs.append(el)
  else:
    el.text = value
  XML.ElementToFile(Plugin.__prefs, Plugin.__prefsPath)
  Log.Add("(Framework) Set preference '%s' to '%s'", (key, value))
  
####################################################################################################

def Expose(key, description):
  """
    Make the given preference available for modification via the plugin's request handlers.
    
    @param key: The key of the preference
    @type key: string
    @param description: The description of the preference
    @type description: string
  """
  global __publicPrefs
  if not __publicPrefs.has_key(key):
    __publicPrefs[key] = description
    Log.Add("(Framework) Exposed preference '%s'" % key)

####################################################################################################

def __getPublicPrefs():
  #
  # Return an XML description of all exposed preferences
  #
  global __publicPrefs
  pp = XML.Element("PluginPreferences")
  for key in __publicPrefs:
    e = XML.ElementWithText(key, Get(key))
    e.set("description", __publicPrefs[key])
    pp.append(e)
  return XML.ElementToString(pp)

####################################################################################################
    
def __handleRequest(pathNouns, count):
  #
  # Handle preference requests
  #
  if count == 3:
    if pathNouns[2] == "list":
      return __getPublicPrefs()
  elif count == 4:
    if pathNouns[2] == "get":
      if __publicPrefs.has_key(pathNouns[3]):
        return "<" + pathNouns[3] + ">" + __publicPrefs[pathNouns[3]] + "</" + pathNouns[3] + ">"
  elif count == 5:
    if pathNouns[2] == "set":
      if __publicPrefs.has_key(pathNouns[3]):
        Set(pathNouns[3], pathNouns[4])
        return Plugin.__XML_RESPONSE_OK
        
####################################################################################################
