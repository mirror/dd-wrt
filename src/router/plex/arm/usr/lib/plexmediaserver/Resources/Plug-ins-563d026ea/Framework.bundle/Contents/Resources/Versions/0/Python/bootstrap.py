#!/usr/bin/python
#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import importer
#importer.install()

import os, sys
from platform import Platform

if sys.platform == "win32":
  # This is needed to ensure binary data transfer over stdio between PMS and plugins
  import msvcrt
  msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)
  msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)

  # This is needed to prevent explosions when there's a system Python installed whose libraries conflict with our own
  for x in [x for x in sys.path if sys.prefix.lower() not in x.lower()]: sys.path.remove(x)

PMSLibPath = os.path.join(sys.path[0], "../../../Platforms/%s/%s/Libraries" % (Platform.OS, Platform.CPU))
PMSSharedLibPath = os.path.join(sys.path[0], "../../../Platforms/Shared/Libraries")

if sys.platform != "darwin":
  # The binary lib path goes at the end, because binary extensions should be picked up from the PMS Exts directory first (on non-Mac platforms)
  # In the future, the binary lib path will be deprecated
  sys.path.append(PMSLibPath)
else:
  sys.path.insert(0, PMSLibPath)
sys.path.insert(0, PMSSharedLibPath)

# Import the Plugin module
import PMS.Plugin, PMS.JSON

# Load the default MIME types
PMS.Plugin.MimeTypes = PMS.JSON.DictFromFile(os.path.join(os.path.split(sys.argv[0])[0], "PMS/MimeTypes.json"))

# Run the plugin
if len(sys.argv) >= 2: PMS.Plugin.__run(sys.argv[-1])
