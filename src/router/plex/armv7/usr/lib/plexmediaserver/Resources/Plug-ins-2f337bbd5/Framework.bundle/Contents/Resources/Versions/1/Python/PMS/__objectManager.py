#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, inspect

def ObjectHasBase(obj, *args):
  for cls in args:
    if cls in inspect.getmro(obj.__class__):
      return True
  return False
  
def FunctionIsParent(*args):
  try:
    return inspect.stack()[2][3] in args
  except:
    return False