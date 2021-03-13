#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import os
import Plugin, JSON, HTTP, Log

CurrentLocale = ""

__defaultLocale = "en-us"
__langDict = {}
__countryDict = {}
__defaultLangDict = {}
__defaultCountryDict = {}

####################################################################################################

def __loadDefaults():
  global __defaultLangDict
  global __defaultCountryDict
  global __defaultLocale
  pos = __defaultLocale.find("-")
  
  if pos > -1:
    lang = __defaultLocale[:pos]
    langPath = os.path.join(Plugin.BundlePath, "Contents/Strings/%s.json" % lang)
    if os.path.exists(langPath):
      __defaultLangDict = JSON.DictFromFile(langPath)
      Log.Add("(Framework) Loaded %s strings" % lang)
    else:
      Log.Add("(Framework) Couldn't find %s strings" % lang)
    
    locPath = os.path.join(Plugin.BundlePath, "Contents/Strings/%s.json" % __defaultLocale)
    if os.path.exists(locPath):
      __defaultCountryDict = JSON.DictFromFile(locPath)
      Log.Add("(Framework) Loaded %s strings" % __defaultLocale)
    else:
      Log.Add("(Framework) Couldn't find %s strings" % __defaultLocale)
      
  else:
    langPath = os.path.join(Plugin.BundlePath, "Contents/Strings/%s.json" % __defaultLocale)
    if os.path.exists(langPath):
      __defaultLangDict = JSON.DictFromFile(langPath)
      Log.Add("(Framework) Loaded %s strings" % __defaultLocale)
    else:
      Log.Add("(Framework) Couldn't find %s strings" % __defaultLocale)

####################################################################################################

def __loadLocale(loc):
  global CurrentLocale
  global __langDict
  global __countryDict
  pos = loc.find("-")
  
  if pos > -1:
    
    lang = loc[:pos]
    langPath = os.path.join(Plugin.BundlePath, "Contents/Strings/%s.json" % lang)
    if os.path.exists(langPath):
      __langDict = JSON.DictFromFile(langPath)
      Log.Add("(Framework) Loaded %s strings" % lang)
    else:
      Log.Add("(Framework) Couldn't find %s strings" % lang)
    
    locPath = os.path.join(Plugin.BundlePath, "Contents/Strings/%s.json" % loc)
    if os.path.exists(locPath):
      __countryDict = JSON.DictFromFile(locPath)
      Log.Add("(Framework) Loaded %s strings" % loc)
    else:
      Log.Add("(Framework) Couldn't find %s strings" % loc)
      
  else:
    langPath = os.path.join(Plugin.BundlePath, "Contents/Strings/%s.json" % loc)
    if os.path.exists(langPath):
      __langDict = JSON.DictFromFile(langPath)
      Log.Add("(Framework) Loaded %s strings" % loc)
    else:
      Log.Add("(Framework) Couldn't find %s strings" % loc)
      
  CurrentLocale = loc

####################################################################################################

def LocalString(key):
  global __langDict
  global __countryDict
  global __defaultLangDict
  global __defaultCountryDict
  
  if __countryDict.has_key(key):
    return __countryDict[key]
  elif __langDict.has_key(key):
    return __langDict[key]
  elif __defaultCountryDict.has_key(key):
    return __defaultCountryDict[key]
  elif __defaultLangDict.has_key(key):
    return __defaultLangDict[key]
  else:
    return key

####################################################################################################

def Geolocation():
  return HTTP.Get("http://api.hostip.info/country.php")

####################################################################################################