#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import __error, Request, Plugin, Datetime, RSS, Client, Data, Database, Dict, Hash, Helper, HTTP, JSON, Locale, Network, Prefs, Resource, String, Thread, XML, YAML, Plist
from Shortcuts import *
from Objects import *
from Decorators import *
from Constants import *
from Platform import Platform

from Routes import ConnectRoute as route
from Routes import GenerateRoute as Route

import sys as __sys, traceback as __traceback
Error = __error.StandardErrors

FrameworkVersion = "1.1b5"
FrameworkCompatibilityVersion = "1.0.0"

__logSaveScheduled = False
__logSaveBuffer = None

def __saveLog():
  global __logSaveBuffer
  global __logSaveScheduled
  Thread.Lock("Framework.Log", False)
  f = open(Plugin.__logFilePath, 'a')
  f.write("%s\n" % __logSaveBuffer)
  f.close()
  __logSaveBuffer = None
  __logSaveScheduled = False
  Thread.Unlock("Framework.Log", addToLog=False)
  
####################################################################################################

def Log(msg, debugOnly=True, encoding=None):
  if not debugOnly or Plugin.Debug:
    global __logSaveScheduled
    global __logSaveBuffer
    
    # Try to decode the message if the encoding is specified
    try:
      if encoding is not None:
        msg = str(msg.decode(encoding).encode("utf8"))
    except:
      pass
    
    msg = "%s: %-40s:   %s" % (str(Datetime.Now().time()), Plugin.Identifier, msg)
    
    # Don't write to stderr on Windows, it causes issues.
    if __sys.platform != "win32":
      __sys.stderr.write("%s\n" % msg)
      
    Thread.Lock("Framework.Log", addToLog=False)
    if __logSaveBuffer == None:
      __logSaveBuffer = msg + "\n"
    else:
      __logSaveBuffer += msg + "\n"
    if not __logSaveScheduled:
      Thread.CreateTimer(5, __saveLog)
      __logSaveScheduled = True
    Thread.Unlock("Framework.Log", addToLog=False)
    
####################################################################################################

def LogChildren(dir, debugOnly=True, encoding=None):
  Log(dir.__items__, debugOnly=debugOnly, encoding=encoding)

####################################################################################################
      
def LogSelfAndChildren(dir, debugOnly=True, encoding=None):
  s = '[%s, [' % (repr(dir))
  for item in dir.__items__:
    s += '\n\t%s,' % (repr(item))
  s = s[:-1] + '] ]'
  Log(s, debugOnly=debugOnly, encoding=encoding)

####################################################################################################

def LogSelfAndDescendants(dir, debugOnly=True, encoding=None):
  Log(_getDescendants(dir, ''), debugOnly=True, encoding=encoding)

####################################################################################################
    
def _getDescendants(dir, tabs):
    s = '[\n' + tabs + repr(dir) + ', ['
    tabs += '\t'

    for item in dir.__items__:
      if '_Function__obj' in item.__dict__:
        s += ' [\n' + tabs + repr(item) + ', '

        handler, args = _getHandlerArgs(dir, item)
        try:
          if item.tagName == 'Directory':
            s += _getDescendants(handler(**args), tabs + '\t')
          else:
            s += '[\n%s\t%s ]' % (tabs, repr(handler(**args)))
        except Exception:
          s += '\n%sException("""' % (tabs + '\t')
          for traceLine in __traceback.format_exception(__sys.exc_type, __sys.exc_value, __sys.exc_traceback):
            for internalLine in traceLine.split('\n')[:-1]:
              s += '\n' + tabs + '\t\t' + internalLine 
          s += '""") ] ],'
        else:          
          s += ' ],'
      else:
        s += '\n%s\t%s,' % (tabs, repr(item))
    # get rid of trailing comma
    s = s[:-1] + ' ] ]'
    
    return s

####################################################################################################
    
def _getHandlerArgs(dir, item):
  handler = item._Function__obj.key
  
  sender = ItemInfoRecord()
  sender.title1 = dir.title1
  sender.title2 = dir.title2
  sender.art = dir.art
  try:
    sender.itemTitle = item.name
  except:
    sender.itemTitle = item.title
  args = item._Function__kwargs
  args['sender'] = sender
  return (handler, args)

####################################################################################################