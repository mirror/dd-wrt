#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  WIP: Functions for handling threads.
"""

import threading

####################################################################################################

class __pmsthread(threading.Thread):
  #
  # Simple class to allow functions to be run inside a separate thread
  #
  def run(self):
    if self.a is None: self.f()
    else : self.f(self.a)

####################################################################################################

def Create(function, args=None):
  """
    Creates a new Thread object with the given function and arguments.
    
    @param function: The function to run in a new thread.
    @type function: function
    @param args: The arguments to be passed to the function.
    @type args: custom
    @return: Thread
  """
  t = __pmsthread()
  t.a = args
  t.f = function
  return t

####################################################################################################

def Run(function, args=None):
  """
    Creates a new Thread object with the given function and arguments and starts it.
    
    @param function: The function to run in a new thread.
    @type function: function
    @param args: The arguments to be passed to the function.
    @type args: custom
    @return: Thread
  """
  t = Create(function, args)
  t.start()
  return t

####################################################################################################
  
def Lock():
  """
    Locks a thread to allow synchronization.
    
    @return: RLock
  """
  return threading.RLock()

####################################################################################################
