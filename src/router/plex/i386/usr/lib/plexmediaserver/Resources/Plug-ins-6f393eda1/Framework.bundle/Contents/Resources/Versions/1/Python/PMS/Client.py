#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS

__major = 0
__minor = 0
__release = 0

def __setVersion(versionString):
  global __major
  global __minor
  global __release
  try:
    if versionString.find("-") > 0:
      versionString = versionString.split("-")[0]
    version = versionString.split(".")
    __major = int(version[0])
    __minor = int(version[1])
    __release = int(version[2])
  except:
    __major = 0
    __minor = 0
    __release = 0

def MajorVersion():
  return __major
  
def MinorVersion():
  return __minor
  
def ReleaseVersion():
  return __release
  
def VersionAtLeast(major=0, minor=0, release=0):
  return ((__major > major) or ((__major == major) and (__minor > minor)) or ((__major == major) and (__minor == minor) and (__release >= release)))