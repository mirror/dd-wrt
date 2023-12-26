#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import os.path, Queue, time, traceback
import PMS, Plugin, Thread, __objectManager

__db = None
__databasePath = None

__sqlQueue = Queue.Queue()
__dbThread = None
__alive = False
__shouldCreate = False
__changed = False

####################################################################################################

class __cmd:
  def __init__(self, cmd, args = []):
    self.cmd = cmd
    self.args = args

####################################################################################################

def __loadDB():
  global __dbThread
  __dbThread = Thread.Create(__loadDBInThread)
  
  # Give the database thread time to start accepting queries
  while not __alive:
    time.sleep(0.1)
  
  # If the database needs to be created, load the defaults file & call the create function
  if __shouldCreate:
    try:
      path = "%s/Contents/DefaultDatabase.sql" % Plugin.__bundlePath
      if os.path.exists(path):
        f = open(path, "r")
        string = f.read()
        f.close()
        Exec(string)
        PMS.Log("(Framework) Loaded the default database file")
        Commit()
      Plugin.__callNamed("CreateDatabase")
      PMS.Log("(Framework) Created database tables")
    except:
      PMS.Log("(Framework) Error creating database tables", False)
    finally:
      Commit()

####################################################################################################

def __loadDBInThread():
  import sqlite3
  global __db
  global __databasePath
  global __alive
  global __sqlQueue
  global __shouldCreate
  global __changed
  
  # Connect to the database, marking __shouldCreate if necessary
  __alive = False
  __shouldCreate = not os.path.exists(__databasePath)
  __db = sqlite3.connect(__databasePath)
  __alive = True
  __changed = False
  
  # Keep waiting for queued commands
  while True:
    s = __sqlQueue.get()
    ret = None
    
    try:

      if s.cmd == "Close":
        # When closing the database, end the thread too
        __db.close()
        __db = None
        break

      elif s.cmd == "Commit":
        __commit()
        
      elif s.cmd == "Rollback":
        __rollback()
        
      elif s.cmd == "SQL":
        ret = __db.execute(s.args[0], s.args[1])
        __changed = not s.args[0].lower().startswith("select")
    
    # Catch exceptions
    except:
      __except()
    if s.resultqueue:
      s.resultqueue.put(ret)

####################################################################################################

def __except():
  # If in debug mode, print the traceback, otherwise report an internal error
  if Plugin.Debug:
    PMS.Log("(Framework) An exception happened in the database thread:\n%s" % traceback.format_exc())
  else:
    PMS.Log("(Framework) An internal error occurred in the database thread", False)

####################################################################################################
    
def __exec(s):
  global __db
  if __db is not None:
    # Creates a result queue for the command object & adds the object to the SQL queue
    s.resultqueue = Queue.Queue()
    __sqlQueue.put(s)
    return s.resultqueue.get()

####################################################################################################

def __commit():
  global __changed
  __db.commit()
  PMS.Log("(Framework) Committed the database")
  __changed = False

####################################################################################################
    
def __rollback():
  global __changed
  __db.rollback()
  PMS.Log("(Framework) Rolled back the database")
  __changed = False

####################################################################################################
  
def Exec(sql, values=[]):
  global __db
  global __alive
  
  # Load the database if it's not there already
  if __db is None:
    __alive = False
    __loadDB()
  
  return __exec(__cmd("SQL", [sql, values]))

####################################################################################################

def Commit():
  __exec(__cmd("Commit"))

####################################################################################################
  
def Rollback():
  __exec(__cmd("Rollback"))
  
####################################################################################################
  
def Reset():
  global __db
  global __databasePath
  global __alive
  global __changed
  __exec(__cmd("Close"))
  if os.path.exists(__databasePath):
    os.remove(__databasePath)
  __alive = False
  __changed = False
  PMS.Log("(Framework) Reset the database")
  
####################################################################################################
