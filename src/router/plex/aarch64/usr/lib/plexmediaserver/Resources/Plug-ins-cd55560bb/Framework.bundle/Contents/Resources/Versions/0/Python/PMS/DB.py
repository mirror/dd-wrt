#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  WIP: Functions for accessing the database
"""

# TODO: Add support for scheduled updates

import os.path
import Plugin, Log

__db = None

####################################################################################################

def __loadDB():
  import sqlite3
  global __db  
  shouldCreateTables = not os.path.exists(Plugin.__databasePath)
  __db = sqlite3.connect(Plugin.__databasePath)
  if shouldCreateTables:
    try:
      Plugin.__call(Plugin.__pluginModule.CreateTables)
      Commit()
      Log.Add("(Framework) Created database tables")
    except AttributeError:
      Log.Add("(Framework) Error creating database tables", False)
      pass

####################################################################################################

def Commit():
  """
    Commit any pending changes to the database file.
  """
  global __db
  if __db is not None:
    __db.commit()

####################################################################################################

def Rollback():
  """
    Roll back any pending changes to the database file.
  """
  global __db
  if __db is not None:
    __db.rollback()

####################################################################################################
  
def Exec(sql, values=[]):
  global __db
  if __db is None:
    __loadDB()
  return __db.execute(sql, values)

####################################################################################################
  
def ExecMany(sql, many_values=[]):
  global __db
  if __db is None:
    __loadDB()
  return __db.executemany(sql, many_values)

####################################################################################################
  
def Reset():
  """
    Empty the current database file.
  """
  global __db
  if db is not None:
    __db.close()
    __db = None
  if os.path.exists(Plugin.__databasePath):
    os.remove(Plugin.__databasePath)
  
####################################################################################################
