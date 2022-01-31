#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import os, sys

osnames = {
  "Darwin"    : "MacOSX",
  "Linux"     : "Linux",
  "Windows"   : "Windows"
}

cpunames = {
  "i386"      : "i386",
  "i686"      : "i386",
  "x86_64"    : "i386",
  "3548b0-smp": "MIPS",
  "mips"      : "MIPS",
  "mips64"    : "mips64",
  "Win32"     : "Win32"
}

class PlatformMetaclass(type):
  def __getattr__(self, name):
    if name == "HasSilverlight":
      return os.path.exists("/Library/Internet Plug-ins/Silverlight.plugin")
    
    elif name == "OS":
      if sys.platform == "win32":
        uname_os = "Windows"
      else:
        uname_os = os.uname()[0]
      if uname_os in osnames:
        return osnames[uname_os]
    
    elif name == "CPU":
      if sys.platform == "win32":
        #TODO: Support x64 CPUs on Windows
        uname_cpu = "Win32"
      else:
        uname_cpu = os.uname()[4]
        
      # Special case for Linux/x86_64.
      if sys.platform != "win32" and os.uname()[4] == 'x86_64' and os.uname()[0] == 'Linux':
        return 'x86_64'
        
      if uname_cpu in cpunames:
        return cpunames[uname_cpu]
      raise Exception("CPU type %s is unsupported" % uname_cpu)
        
    raise AttributeError("No attribute named "+name)

class Platform(object):
  __metaclass__ = PlatformMetaclass
