#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, Plugin, os

####################################################################################################

__resourcePath = None
__sharedResourcePath = None
__publicResources = {}
__publicSharedResources = {}
__mimeTypes = None

####################################################################################################

def __load(path, binary=True):
  if os.path.exists(path):
    if not binary:
      f = open(path, "r")
    else:
      f = open(path, "rb")
    resource = f.read()
    f.close()
    return resource
  return None

####################################################################################################
  
def __exposeResource(name, contentType):
  global __publicResources
  if os.path.exists("%s/%s" % (__resourcePath, name)) and not __publicResources.has_key(name):
    __publicResources[name] = contentType
    PMS.Log("(Framework) Resource named '%s' of type '%s' was made public." % (name, contentType))
    return True
  else: return False

####################################################################################################

def __exposeSharedResource(name, contentType):
  global __publicSharedResources
  if os.path.exists("%s/%s" % (__sharedResourcePath, name)) and not __publicSharedResources.has_key(name):
    __publicSharedResources[name] = contentType
    PMS.Log("(Framework) Shared resource named '%s' of type '%s' was made public." % (name, contentType))
    return True
  else: return False

####################################################################################################

def __realSharedItemName(itemName):
  global __sharedResourcePath
  for ext in ["png", "jpg"]:
    itemNameWithExt = itemName + "." + ext
    if os.path.exists(os.path.join(__sharedResourcePath, itemNameWithExt)):
      return itemNameWithExt
  return None
  
####################################################################################################
####################################################################################################

def Load(itemName, binary=True):
  data = __load("%s/%s" % (__resourcePath, itemName), binary)
  if data is not None:
    PMS.Log("(Framework) Loaded resource named '%s'" % itemName)
    return data

####################################################################################################

def LoadShared(itemName, binary=True):
  data = __load("%s/%s" % (__sharedResourcePath, itemName), binary)
  if data is not None:
    PMS.Log("(Framework) Loaded shared resource named '%s'" % itemName)
    return data

####################################################################################################

def ExternalPath(itemName):
  if len(Plugin.Prefixes()) == 0: return None
  global __publicResources
  global __mimeTypes
  if itemName is None: return
  ext = itemName[itemName.rfind("."):]
  if __mimeTypes.has_key(ext):
    __exposeResource(itemName, __mimeTypes[ext])
  else:
    __exposeResource(itemName, "application/octet-stream")
  if __publicResources.has_key(itemName):
    return "%s/:/resources/%s" % (Plugin.Prefixes()[0], itemName)
  else:
    return None

####################################################################################################

def SharedExternalPath(itemName):
  if len(Plugin.Prefixes()) == 0: return None
  global __publicSharedResources
  global __mimeTypes
  if itemName is None: return
  
  if itemName.find(".") < 0:
    itemName = __realSharedItemName(itemName)
  
  if itemName is None: return
  
  ext = itemName[itemName.rfind("."):]
  if __mimeTypes.has_key(ext):
    __exposeSharedResource(itemName, __mimeTypes[ext])
  else:
    __exposeSharedResource(itemName, "application/octet-stream")
  if __publicSharedResources.has_key(itemName):
    return "%s/:/sharedresources/%s" % (Plugin.Prefixes()[0], itemName)
  else:
    return None
  
####################################################################################################

def MimeTypeForExtension(ext):
  global __mimeTypes
  if __mimeTypes.has_key(ext):
    return __mimeTypes[ext]
  else:
    return "application/octet-stream"

####################################################################################################

def AddMimeType(ext, mimeType):
  global __mimeTypes
  __mimeTypes[ext] = mimeType

####################################################################################################

