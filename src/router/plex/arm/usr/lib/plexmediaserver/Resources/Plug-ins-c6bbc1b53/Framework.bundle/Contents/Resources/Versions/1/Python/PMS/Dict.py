#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, Plugin, Data, JSON, os, Thread, copy

####################################################################################################

__saveScheduled = False
__dict = {}

####################################################################################################

def __loadDefaults():
  global __dict
  __dict = {}
  path = "%s/Contents/DefaultDict.json" % Plugin.__bundlePath
  if os.path.exists(path):
    f = open(path, "r")
    string = f.read()
    f.close()
    __dict = JSON.ObjectFromString(string)
    PMS.Log("(Framework) Loaded the default dictionary file")
  Plugin.__callNamed("CreateDict", addToLog=False)
  Set("Framework.LastCompatibilityVersion", PMS.FrameworkCompatibilityVersion, addToLog=False)

####################################################################################################

def __load():
  global __dict
  path = "%s/Dict" % Data.__dataPath
  if os.path.exists(path):
    try:
      __dict = Data.__unpickle(path)
      PMS.Log("(Framework) Loaded the dictionary file")
    except:
      PMS.Log("(Framework) The dictionary file is corrupt & couldn't be loaded")
      __loadDefaults()
  else:
    __loadDefaults()

####################################################################################################

def __save(addToLog=True):
  Thread.Lock("Framework.Dict", addToLog=False)
  dictToSave = copy.deepcopy(__dict)
  global __saveScheduled
  __saveScheduled = False
  try:
    Data.__pickle("%s/Dict" % Data.__dataPath, dictToSave)
    if addToLog:
      PMS.Log("(Framework) Saved the dictionary file")
  finally:
    Thread.Unlock("Framework.Dict", addToLog=False)

####################################################################################################
####################################################################################################

def Get(key):
  global __dict
  if __dict.has_key(key):
    return __dict[key]

####################################################################################################

def Set(key, value, addToLog=True):
  global __dict
  global __saveScheduled
  Thread.Lock("Framework.Dict", addToLog=False)
  __dict[key] = value
  if not __saveScheduled:
    Thread.CreateTimer(5, __save, addToLog=addToLog)
    __saveScheduled = True
  Thread.Unlock("Framework.Dict", addToLog=False)
  #__save(addToLog)
  
####################################################################################################

def HasKey(key):
  global __dict
  return __dict.has_key(key)
  
####################################################################################################

def Reset():
  path = "%s/Dict" % Data.__dataPath
  if os.path.exists(path):
    os.unlink(path)
  __loadDefaults()
  
####################################################################################################
