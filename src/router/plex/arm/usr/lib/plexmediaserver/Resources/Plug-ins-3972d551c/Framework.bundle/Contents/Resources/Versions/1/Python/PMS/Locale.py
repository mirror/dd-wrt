#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import os
import PMS, Plugin, JSON, HTTP

CurrentLocale = None

__defaultLocale = None
langDict = {}
countryDict = {}
defaultLangDict = {}
defaultCountryDict = {}

####################################################################################################

def SetDefaultLocale(loc="en-us"):
  global defaultLangDict
  global defaultCountryDict
  global __defaultLocale
  global CurrentLocale
  if loc == __defaultLocale: return
  
  __defaultLocale = loc
  
  pos = __defaultLocale.find("-")
  
  if pos > -1:
    lang = __defaultLocale[:pos]
    langPath = os.path.join(Plugin.__bundlePath, "Contents/Strings/%s.json" % lang)
    if os.path.exists(langPath):
      f = open(langPath, "r")
      defaultLangDict = JSON.ObjectFromString(f.read())
      f.close()
      PMS.Log("(Framework) Loaded %s strings" % lang)
    else:
      PMS.Log("(Framework) Couldn't find %s strings" % lang)
    
    locPath = os.path.join(Plugin.__bundlePath, "Contents/Strings/%s.json" % __defaultLocale)
    if os.path.exists(locPath):
      f = open(locPath, "r")
      defaultCountryDict = JSON.ObjectFromString(f.read())
      f.close()
      PMS.Log("(Framework) Loaded %s strings" % __defaultLocale)
    else:
      PMS.Log("(Framework) Couldn't find %s strings" % __defaultLocale)
      
  else:
    langPath = os.path.join(Plugin.__bundlePath, "Contents/Strings/%s.json" % __defaultLocale)
    if os.path.exists(langPath):
      f = open(langPath, "r")
      defaultLangDict = JSON.ObjectFromString(f.read())
      f.close()
      PMS.Log("(Framework) Loaded %s strings" % __defaultLocale)
    else:
      PMS.Log("(Framework) Couldn't find %s strings" % __defaultLocale)
      
  if CurrentLocale == None: CurrentLocale = loc

####################################################################################################

def __loadLocale(loc):
  global CurrentLocale
  if loc == CurrentLocale:
    return None
  
  global langDict
  global countryDict
  pos = loc.find("-")
  
  if pos > -1:
    
    lang = loc[:pos]
    langPath = os.path.join(Plugin.__bundlePath, "Contents/Strings/%s.json" % lang)
    if os.path.exists(langPath):
      f = open(langPath, "r")
      langDict = JSON.ObjectFromString(f.read())
      f.close()
      PMS.Log("(Framework) Loaded %s strings" % lang)
    else:
      PMS.Log("(Framework) Couldn't find %s strings" % lang)
    
    locPath = os.path.join(Plugin.__bundlePath, "Contents/Strings/%s.json" % loc)
    if os.path.exists(locPath):
      f = open(locPath, "r")
      countryDict = JSON.ObjectFromString(f.read())
      f.close()
      PMS.Log("(Framework) Loaded %s strings" % loc)
    else:
      PMS.Log("(Framework) Couldn't find %s strings" % loc)
      
  else:
    langPath = os.path.join(Plugin.__bundlePath, "Contents/Strings/%s.json" % loc)
    if os.path.exists(langPath):
      f = open(langPath, "r")
      langDict = JSON.ObjectFromString(f.read())
      f.close()
      PMS.Log("(Framework) Loaded %s strings" % loc)
    else:
      PMS.Log("(Framework) Couldn't find %s strings" % loc)
      
  CurrentLocale = loc

####################################################################################################

# Subclass of str - dynamically localizes when converted to str
class LocalString(str):
  def __init__(self, key):
    if isinstance(key, LocalString):
        self.key = key.key
    else:
        self.key = key
    str.__init__(self)
    
  # Static method to localize a key on demand
  @staticmethod
  def Localize(key):
    global langDict
    global countryDict
    global defaultLangDict
    global defaultCountryDict
    
    if countryDict.has_key(key):
      return countryDict[key]
    elif langDict.has_key(key):
      return langDict[key]
    elif defaultCountryDict.has_key(key):
      return defaultCountryDict[key]
    elif PMS.Locale.defaultLangDict.has_key(key):
      return PMS.Locale.defaultLangDict[key]
    else:
      return key
    
  # Conversion to regular string or unicode
  def __str__(self):
    return LocalString.Localize(self.key)
  def __unicode__(self):
    return unicode(self.__str__())
        
  # Representation  
  def __repr__(self):
    return "PMS.Locale.LocalString(" + repr(self.key) + ")"
    
  # Support for the + operator while retaining dynamic localization
  def __add__(self,other):
    return LocalStringPair(self, other)
  def __radd__(self,other):
    return LocalStringPair(other, self)
 
# Maintains two objects for concatentation at runtime. Allows string expressions to be generated & evaluated dynamically
class LocalStringPair(object):
  
  # Store both objects
  def __init__(self, string1, string2):
    self.string1 = string1
    self.string2 = string2
    
  # Conversion to regular string or unicode
  def __str__(self):
    return self.string1.__str__() + self.string2.__str__()
  def __unicode__(self):
    return unicode(self.__str__())
    
  # Representation
  def __repr__(self):
    return "PMS.Locale.LocalStringPair(" + repr(self.string1) + ", " + repr(self.string2) + ")"
    
  def __add__(self, other):
    return LocalStringPair(self, other)

  def __radd__(self, other):
    return LocalStringPair(other, self)
        
# Similar to LocalStringPair, but uses the second object for string formatting
class LocalStringFormatter(LocalStringPair):
  def __str__(self):
    return self.string1.__str__() % self.string2
  def __repr__(self):
    return "PMS.Locale.LocalStringFormatter(" + repr(self.string1) + ", " + repr(self.string2) + ")"
    
# Replacement for % operator (shortcut: F) which supports dynamic localization
def LocalStringWithFormat(key, *args):
  return LocalStringFormatter(LocalString(key), args)
    
####################################################################################################

def Geolocation():
  return HTTP.Request("http://james.plexapp.com/apps/geolocate.php", cacheTime=86400)

####################################################################################################