#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  Logging functions.
"""

import Plugin, sys
from datetime import datetime

####################################################################################################

def Add(msg, debugOnly=True):
  """
    Add a message to the plugin's log file.
    
    @param msg: The message to log
    @type msg: string
    @param debugOnly: Specifies whether the message should only be logged when debugging is enabled.
    @type debugOnly: Boolean
    @return None
  """
  if not debugOnly or Plugin.Debug:
    logmsg = "%s: %-32s:   %s" % (str(datetime.now().time()), Plugin.Identifier, str(msg))
    
    # Don't write to stderr on Windows, it causes issues.
    if sys.platform != "win32":
      sys.stderr.write("%s\n" % logmsg)
      
    f = open(Plugin.__logFilePath, 'a')
    f.write("%s\n" % logmsg)
    f.close()

####################################################################################################