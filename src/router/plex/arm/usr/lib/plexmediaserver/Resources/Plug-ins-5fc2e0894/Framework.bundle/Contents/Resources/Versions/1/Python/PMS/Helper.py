#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, os

__helperPath = None

####################################################################################################

def Run(helper, *args):
  helperPath = os.path.join(__helperPath, helper)
  if os.path.exists(helperPath):
    os.chmod(helperPath, 0755)
    execString = "\"%s\"" % helperPath
    for arg in args:
      execString += " \"%s\"" % arg
    return os.popen(execString).read().strip()
  else:
    PMS.Log("(Framework) Helper named '%s' does not exist" % helper)
    return None