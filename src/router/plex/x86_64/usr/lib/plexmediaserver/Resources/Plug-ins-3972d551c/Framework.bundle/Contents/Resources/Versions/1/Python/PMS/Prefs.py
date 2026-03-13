#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, Plugin, JSON, Locale, Thread, os, __objectManager
from Objects import *
from Shortcuts import *
from string import lower

__prefs = []
__prefsPath = ""
__dialogTitle = "Preferences"

####################################################################################################    

def __load():
  global __prefs
  global __prefsPath
  
  path = "%s/Contents/DefaultPrefs.json" % Plugin.__bundlePath
  if os.path.exists(path):
    f = open(path, "r")
    string = f.read()
    f.close()
    __prefs = JSON.ObjectFromString(string)
    PMS.Log("(Framework) Loaded the default preferences file")
  Plugin.__callNamed("CreatePrefs", addToLog=False)
  if os.path.exists(__prefsPath):
    try:
      f = open(__prefsPath, "r")
      userPrefs = XML.ElementFromString(f.read())
      f.close()
      for userPref in userPrefs:
        for pref in __prefs:
          if pref["id"] == userPref.tag:
            pref["value"] = userPref.text
      PMS.Log("(Framework) Loaded the user preferences file")
    except:
      PMS.Log("(Framework) Exception when reading preferences")

####################################################################################################    

def __container():
  global __prefs
  global __dialogTitle
  container = MediaContainer(title=__dialogTitle)
  for pref in __prefs:
    prefObj = XMLObject(tagName="Setting")
    for key in pref:
      prefObj.__dict__[key] = pref[key]
      
    # If dealing with an enum, deal with the keys differently
    if pref.has_key("values"):
      if isinstance(pref["values"], basestring):
        prefValues = pref["values"].split("|")
      else:
        prefValues = list(pref["values"])
      
      # Extract the default index
      if pref.has_key("default"):
        if pref["default"] in prefValues:
          prefObj.default = prefValues.index(pref["default"])

      # Set the default index
      if pref.has_key("value"):
        if pref["value"] in prefValues:
          prefObj.value = prefValues.index(pref["value"])
          
      # Localize values
      for i in range(len(prefValues)):
        prefValues[i] = str(L(prefValues[i]))
      prefObj.values = String.Join(prefValues, "|")

    if not pref.has_key("value") and pref.has_key("default"):
      prefObj.value = prefObj.default
    if pref.has_key("label"):
      prefObj.label = L(prefObj.label)
    container.Append(prefObj)
  return container

####################################################################################################

def __setAll(args):
  global __prefs
  for pref in __prefs:
    for arg in args:
      if pref["id"] == arg:
        if lower(pref["type"]) == "enum" and pref.has_key("values"):
          if isinstance(pref["values"], basestring):
            prefValues = pref["values"].split("|")
          else:
            prefValues = list(pref["values"])
          try:
            if int(args[arg]) < len(prefValues):
              pref["value"] = prefValues[int(args[arg])]
          except:
            pass
        else:
          pref["value"] = args[arg]
  __save()
  
####################################################################################################

def __save():
  global __prefs
  global __prefsPath
  Thread.Lock("Framework.Prefs", addToLog=False)
  try:
    userPrefs = XML.Element("PluginPreferences")
    for pref in __prefs:
      if pref.has_key("value"):
        userPrefs.append(XML.Element(pref["id"], pref["value"]))
    f = open(__prefsPath, "w")
    f.write(XML.StringFromElement(userPrefs))
    f.close()
    PMS.Log("(Framework) Saved the user preferences file")
  finally:
    Thread.Unlock("Framework.Prefs", addToLog=False)

####################################################################################################
  
def Get(id):
  global __prefs
  for pref in __prefs:
    if pref.has_key("id"):
      if pref["id"] == id:
        ret = None
        if pref.has_key("value"):
          ret = pref["value"]
        elif pref.has_key("default"):
          ret = pref["default"]
        if lower(pref["type"]) == "bool":
          if ret == "true":
            return True
          else:
            return False
        return ret
  return None

####################################################################################################

def Set(id, value):
  global __prefs
  for pref in __prefs:
    if pref["id"] == id:
      if lower(pref["type"]) == "bool":
        if value == True:
          pref["value"] = "true"
        else:
          pref["value"] = "false"
      else:
        pref["value"] = value
      __save()
      
####################################################################################################

def Add(id, type, default, label, **kwargs):
  global __prefs
  for pref in __prefs:
    if pref.has_key("id"):
      if pref["id"] == id:
        return

  # Convert boolean prefs to strings
  if lower(type) == "bool":
    if default == True:
      default = "true"
    else:
      default = "false"
      
  pref = {"id":id, "type":type, "default":default, "label":label}
  for kw in kwargs:
    pref[kw] = kwargs[kw]
  pref["value"] = default
  __prefs.append(pref)
  PMS.Log("(Framework) Added new preference '%s'" % id)
  if not __objectManager.FunctionIsParent("CreatePrefs"):
    PMS.Log("(Framework) WARNING: Prefs.Add() should only be called within the CreatePrefs() method.")

####################################################################################################

def Reset():
  global __prefs
  global __prefsPath
  __prefs = []
  if os.path.exists(__prefsPath):
    os.remove(__prefsPath)
  __load()

####################################################################################################

def SetDialogTitle(dialogTitle):
  global __dialogTitle
  __dialogTitle = dialogTitle

####################################################################################################

