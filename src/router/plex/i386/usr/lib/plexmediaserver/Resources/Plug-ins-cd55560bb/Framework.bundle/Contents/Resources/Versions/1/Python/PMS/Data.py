#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, os, pickle

####################################################################################################

__dataPath = None
__dataItemPath = None

####################################################################################################

def __path(itemName):
  return "%s/%s" % (__dataItemPath, itemName)

####################################################################################################
  
def __write(path, data):
  f = open(path, "w")
  f.write(data)
  f.close()

####################################################################################################

def __unpickle(path):
  f = open(path, "r")
  obj = pickle.load(f)
  f.close()
  return obj

####################################################################################################

def __pickle(path, obj):
  f = open(path, "w")
  pickle.dump(obj, f, 2)
  f.close()

####################################################################################################
####################################################################################################

def Exists(itemName):
  return os.path.exists(__path(itemName))

####################################################################################################

def Remove(itemName):
  if Exists(itemName):
    return os.unlink(__path(itemName))
  else:
    return False

####################################################################################################

def Load(itemName):
  if Exists(itemName):
    f = open(__path(itemName), "r")
    data = f.read()
    f.close()
    PMS.Log("(Framework) Loaded the data item named '%s'" % itemName)
    return data
  else:
    PMS.Log("(Framework) Couldn't load the data item named '%s' - the item does not exist" % itemName)

####################################################################################################

def LoadObject(itemName):
  if Exists(itemName):
    try:
      obj = __unpickle(__path(itemName))
      PMS.Log("(Framework) Loaded the data item named '%s' as an object" % itemName)
      return obj
    except:
      PMS.Log("(Framework) Couldn't load the data item named '%s' as an object - the item couldn't be read" % itemName)
      return None
  else:
    PMS.Log("(Framework) Couldn't load the data item named '%s' as an object - the item does not exist" % itemName)

####################################################################################################

def Save(itemName, data):
  __write(__path(itemName), data)
  PMS.Log("(Framework) Saved the data item named '%s'" % itemName)

####################################################################################################

def SaveObject(itemName, obj):
  __pickle(__path(itemName), obj)
  PMS.Log("(Framework) Saved the data item named '%s' as an object" % itemName)

####################################################################################################
