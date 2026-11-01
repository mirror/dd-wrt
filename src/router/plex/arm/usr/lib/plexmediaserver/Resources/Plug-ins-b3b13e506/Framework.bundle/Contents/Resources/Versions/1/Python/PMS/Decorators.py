#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, Plugin, Thread, inspect, traceback, Locale, Dict, Objects, Datetime
from Shortcuts import *

####################################################################################################

def handler(prefix, name, thumb="icon-default.png", art="art-default.png", titleBar="titlebar-default.png"):
  def pms_fwk_handler_wrapper(f):
    Plugin.AddPrefixHandler(prefix, f, L(name), thumb, art, titleBar)
    return f
  return pms_fwk_handler_wrapper
  
####################################################################################################

class thread(object):
  def __init__(self, f):
    self.f = f
  def __call__(self):
    Thread.Create(self.f)

####################################################################################################
    
class spawn(thread):
  def __init__(self, f):
    thread.__init__(self, f)
    self.__call__()
  def __call__(self):
    thread.__call__(self)

####################################################################################################

class parallel(object):
  tasksets = {}
  maxThreads = 8
  
  def __init__(self, f):
    # Set up instance vars & create a task set list
    self.tasksetname = f.__name__
    self.threadcount = 0
    parallel.tasksets[self.tasksetname] = []
    
    # Call the task setup function
    f()
    
  def __call__(self):
    PMS.Log("(Framework) Starting a parallel task set named '%s'" % self.tasksetname)
    
    # Try & fetch the task set
    try:
      taskset = parallel.tasksets[self.tasksetname]
    except:
      PMS.Log( "(Framework) ERROR: Unable to start the task set named '%s'" % self.tasksetname)
      return
    
    if len(taskset) == 0:
      PMS.Log("(Framework) ERROR: The task set named '%s' contains no tasks" % self.tasksetname)
      return
    
    # Start tasks, making sure we don't go over the maximum limit
    lockname = "Framework.Parallel." + self.tasksetname
    taskindex = 0
    while taskindex < len(taskset):
      Thread.Lock(lockname, addToLog=False)
      if self.threadcount < parallel.maxThreads:
        Thread.Create(self.__runtask__, taskset[taskindex])
        taskindex += 1
      Thread.Unlock(lockname, addToLog=False)
      Thread.Sleep(0.05)
    
    # Wait for all threads to terminate
    while True:
      Thread.Lock(lockname, addToLog=False)
      if self.threadcount > 0:
        Thread.Unlock(lockname, addToLog=False)
        Thread.Sleep(0.05)
      else:
        Thread.Unlock(lockname, addToLog=False)
        break

    # Remove the task set
    PMS.Log("(Framework) Parallel task set named '%s' has finished." % self.tasksetname)

    
  def __runtask__(self, t):
    lockname = "Framework.Parallel." + self.tasksetname
    taskset = parallel.tasksets[self.tasksetname]
    
    Thread.Lock(lockname, addToLog=False)
    self.threadcount += 1
    Thread.Unlock(lockname, addToLog=False)
    
    try: t()
    finally:
      Thread.Lock(lockname, addToLog=False)
      self.threadcount -= 1
      Thread.Unlock(lockname, addToLog=False)
    
####################################################################################################

class parallelize(parallel):
  def __init__(self, f):
    parallel.__init__(self, f)
    self.__call__()
  def __call__(self):
    parallel.__call__(self)
    del parallel.tasksets[self.tasksetname]
    
####################################################################################################

class task(object):
  def __init__(self, f, *args, **kwargs):
    self.taskname = f.__name__
    self.__function = f
    self.__args = args
    self.__kwargs = kwargs
    
    try:
      for item in inspect.stack():
        if parallel.tasksets.has_key(item[3]):
          self.tasksetname = item[3]
          parallel.tasksets[self.tasksetname].append(self)
          return
      raise
    except:
      PMS.Log("(Framework) ERROR: Unable to find a task set for the task named '%s'" % self.taskname)
      return
    
  def __call__(self):
    self.__function(*self.__args, **self.__kwargs)
    
####################################################################################################

def modify_dict(key):
  def pms_fwk_modifydict_wrapper(f):
    lockname = "Framework.ModDict:" + key
    try:
      Thread.Lock(lockname, addToLog=False)
      dictitem = Dict.Get(key)
      f(dictitem)
      Dict.Set(key, dictitem, addToLog=False)
    finally:
      Thread.Unlock(lockname, addToLog=False)
  return pms_fwk_modifydict_wrapper
  
####################################################################################################

def lock(f):
  lockname = "Framework.Lock:" + f.__name__
  try:
    Thread.Lock(lockname, addToLog=False)
    f()
  finally:
    Thread.Unlock(lockname, addToLog=False)
  return f

####################################################################################################

class progressive_load(object):
  loaders = {}
  
  def __init__(self, baseContainer):
    self.container = baseContainer
    self.batches = []
    self.finished_loading = False

  def __call__(self, f):
    self.name = f.__name__
    if progressive_load.loaders.has_key(self.name):
      self = progressive_load.loaders[self.name]
    else:
      PMS.Log("(Framework) Started a new progressive loader named '%s'" % self.name)
      self.__function = f
      self.last_fetch_time = Datetime.Now()
      progressive_load.loaders[self.name] = self
      Thread.Create(progressive_load.__runLoader, self)
    i = 0
    while i < 10 and len(self.container) == 0:
      Thread.Sleep(0.1)
    return self.getContainer()
    
  def getContainer(self):
    self.last_fetch_time = Datetime.Now()
    if self.finished_loading:
      self.container.autoRefresh = 0
      del progressive_load.loaders[self.name]
    else:
      self.container.autoRefresh = 1
    return self.container
    
  def __runLoader(self):
    self.__function(self.container)
    for batch in self.batches:
      if self.last_fetch_time < Datetime.Now() - Datetime.Delta(seconds=5):
        del progressive_load.loaders[self.name]
        return
      try:
        batch()
      except:
        PMS.Log(("(Framework) An exception happened in a progressive loader batch named '%s'\n" % batch.batchname) + traceback.format_exc())
    self.finished_loading = True
  
####################################################################################################

class batch(object):
  def __init__(self, f, *args, **kwargs):
    self.batchname = f.__name__
    self.__function = f
    self.__functionArgs = args
    self.__functionKwargs = kwargs
    try:
      for item in inspect.stack():
        if progressive_load.loaders.has_key(item[3]):
          self.loadername = item[3]
          progressive_load.loaders[self.loadername].batches.append(self)
          return
      raise
    except:
      PMS.Log("(Framework) ERROR: Unable to find a progressive loader for the batch named '%s'" % self.batchname)
      return

  def __call__(self):
    self.__function(*self.__functionArgs, **self.__functionKwargs)
  
####################################################################################################