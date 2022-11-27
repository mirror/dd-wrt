#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, time
import threading as _threading

__locks = {}

####################################################################################################

def __startTimedThread(function, *args, **kwargs):
  if function.__name__[0] != "_":
    PMS.Log("(Framework) Started a new thread named '%s' after a timed interval" % function.__name__)
  function(*args, **kwargs)

####################################################################################################

def Create(function, *args, **kwargs):
  if function.__name__[0] != "_":
    PMS.Log("(Framework) Started a new thread named '%s'" % function.__name__)
    
  th = _threading.Thread(None, function, None, args, kwargs)
  th.start()
  return th

####################################################################################################

def CreateTimer(interval, function, *args, **kwargs):
  a = list(args)
  a.insert(0, function)
  timer = _threading.Timer(interval, __startTimedThread, tuple(a), kwargs)
  timer.start()
  if function.__name__[0] != "_":
    PMS.Log("(Framework) Scheduled a timed thread named '%s'" % function.__name__)
  return timer

####################################################################################################

def Lock(key, addToLog=True):
  global __locks
  try:
    if not __locks.has_key(key):
      __locks[key] = _threading.Lock()
    if addToLog:
      PMS.Log("(Framework) Acquiring the thread lock '%s'" % key)
    __locks[key].acquire()
    return True
  except:
    PMS.Log("(Framework) Unable to acquire the thread lock '%s'" % key)

####################################################################################################

def Unlock(key, addToLog=True):
  global __locks
  if __locks.has_key(key):
    if addToLog:
      PMS.Log("(Framework) Releasing the thread lock '%s'" % key)
    __locks[key].release()
    return True
  else:
    PMS.Log("(Framework) Unable to find a thread lock named '%s'" % key)
    return False

####################################################################################################

def Sleep(secs):
  time.sleep(secs)
  
####################################################################################################
