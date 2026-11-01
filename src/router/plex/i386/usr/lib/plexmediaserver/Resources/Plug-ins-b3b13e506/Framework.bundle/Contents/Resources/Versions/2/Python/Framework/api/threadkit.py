#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import time, Queue, types

from base import BaseKit

class ThreadKit(BaseKit):
  
  _included_policies = [
    Framework.policies.BundlePolicy,
  ]

  _excluded_policies = [
    Framework.policies.CloudPolicy,
  ]

  def _init(self):
    self._parallelizer_tasks = None
    self._publish(self._thread_decorator, name='thread')
    self._publish(self._spawn_decorator, name='spawn')
    self._publish(self._lock_decorator, name='lock')
    self._publish(self._locks_decorator, name='locks')
    self._publish(self._parallelize_decorator, name='parallelize')
    self._publish(self._parallel_decorator, name='parallel')
    self._publish(self._task_decorator, name='task')
    
    self.Queue = Queue.Queue
  
  def _key(self, key):
    #print "TKKey:%s" % key
    if key is not None:
      return 'ThreadKit:'+str(self._core.runtime._key(key))
  
  def Create(self, f, globalize=True, *args, **kwargs):
    """
      Creates a new thread which executes the given function, using the arguments and keyword
      arguments provided.
      
      :arg f: The function to execute.
      :type f: function
      
      .. note::
      
        By default, threads are globalized (removed from the current request context and
        executed in a context of their own - see :ref:`contexts`). If the developer chooses not
        to globalize a thread, it is their responsibility to ensure the main request thread
        stays alive while the new thread is running to avoid the request context being
        released too early.
    """
    return self._core.runtime.create_thread(f, True, self._sandbox, globalize, *args, **kwargs)
    
  def CreateTimer(self, interval, f, globalize=True, *args, **kwargs):
    """
      Similar to the *Create* method above, but executes the function in a background thread after
      the given time interval rather than executing it immediately.
      
      :arg interval: The number of seconds to wait before executing the function
      :type interval: float
      
      :arg f: The function to execute.
      :type f: function
    """
    return self._core.runtime.create_timer(interval, f, True, self._sandbox, globalize, *args, **kwargs)
    
  def Sleep(self, interval):
    """
      Pauses the current thread for a given time interval.
      
      :arg interval: The number of seconds to sleep for.
      :type interval: float
    """
    time.sleep(interval)
    
  def Lock(self, key=None):
    """
      Returns the lock object with the given key. If no named lock object exists, a new one
      is created and returned. If no key is provided, a new, anonymous lock object is returned.
      
      :arg key: The key of the named lock to return.
      :type key: str or None
      
      :rtype: `Lock <http://docs.python.org/library/threading.html#lock-objects>`_
    """
    return self._core.runtime.lock(self._key(key))
    
  def AcquireLock(self, key):
    """
      Acquires the named lock for the given key.
      
      :arg key: The key of the named lock to acquire.
      :type key: str
      
      .. note::
         
         It is the developer's responsibility to ensure the lock is released correctly.
    """
    return self._core.runtime.acquire_lock(self._key(key))
    
  def ReleaseLock(self, key):
    """
      Releases the named lock for the given key.
      
      :arg key: The key of the named lock to acquire.
      :type key: str
    """
    return self._core.runtime.release_lock(self._key(key))
    
  def Block(self, key):
    """
      Clears a named event, causing any threads that subsequently wait for the event will block.
      
      :arg key: The key of the named event.
      :type key: str
    """
    return self._core.runtime.block_event(self._key(key))
    
  def Unblock(self, key):
    """
      Sets a named event, unblocking any threads currently waiting on it.
      
      :arg key: The key of the named event.
      :type key: str
    """
    return self._core.runtime.unblock_event(self._key(key))
    
  def Wait(self, key, timeout=None):
    """
      If the named event is unset, this method causes the current thread to block until the event
      is set, or the timeout interval elapses (if provided).
      
      :arg key: The key of the named event.
      :type key: str

      :arg timeout: The number of seconds to wait before aborting.
      :type timeout: float
      
      :returns: True if the named event was set while waiting, or False if the timeout occurred first.
      :rtype: bool
    """
    return self._core.runtime.wait_for_event(self._key(key), timeout)
    
  def Event(self, key=None):
    """
      Returns the event object with the given key. If no named event object exists, a new one
      is created and returned. If no key is provided, a new, anonymous event object is returned.
      
      :arg key: The key of the named event to return.
      :type key: str or None
      
      :rtype: `Event <http://docs.python.org/library/threading.html#event-objects>`_
      
    """
    return self._core.runtime.event(self._key(key))
    
  def Semaphore(self, key=None, limit=1):
    """
      Returns the semaphore object with the given key. If no named semaphore object exists, a new one
      is created and returned. If no key is provided, a new, anonymous semaphore object is returned.
      
      :arg key: The key of the named semaphore to return.
      :type key: str or None
      
      :rtype: `Semaphore <http://docs.python.org/library/threading.html#semaphore-objects>`_
    """
    return self._core.runtime.semaphore(self._key(key), limit)
    
  def _thread_decorator(self, f):
    def _thread_function(*args, **kwargs):
      self.Create(f, *args, **kwargs)
    return _thread_function
    
  def _spawn_decorator(self, f):
    self.Create(f)
    
  def _lock_decorator(self, key, *keys):
    def _lock_decorator_inner(f, *args, **kwargs):
      #print "About to acquire: %s" % key
      _key = self._key(key)
      self._core.runtime.acquire_lock(_key)
      l = list(self._key(k) for k in keys)
      #print "About to lock %s" % str(l)
      for k in l:
        self._core.runtime.acquire_lock(k)
      try:
        return f(*args, **kwargs)
      finally:
        l.reverse()
        for k in l:
          self._core.runtime.release_lock(k)
        self._core.runtime.release_lock(_key)
    return _lock_decorator_inner
  
  def _locks_decorator(self, key, *keys):
    def _locks_decorator_inner(f, key=key):
      def _locks_function(key=key, f=f, *args, **kwargs):
        return self._lock_decorator(key, *keys)(f, *args, **kwargs)
      return _locks_function
    if isinstance(key, types.FunctionType):
      return _locks_decorator_inner(key)
    return _locks_decorator_inner
    
  def _block_decorator(self, f, *args, **kwargs):
    blockname = "ThreadKit:" + f.__name__
    self.Block(blockname)
    ret = f(*args, **kwargs)
    self.Unblock(blockname)
    return ret
    
  def _blocks_decorator(self, f):
    def _blocks_function(*args, **kwargs):
      return self._block_decorator(f, *args, **kwargs)
    return _blocks_function
    
    
  """
    The @parallelize and @task decorators are designed to be used in conjunction with each other - 
    @parallelize decorates a function that defines one or more @task functions. A ThreadKit-specific
    lock is maintained so only one parallelizer can be created at any time. When the parallelizer
    has run, the tasks are queued for dispatch to the runtime's task pool, and the calling thread is
    blocked until all tasks have completed.
  """
  def _parallelize_decorator(self, f, *args, **kwargs):
    self.AcquireLock("_ThreadKit:Parallelizer")
    self._parallelizer_tasks = []
    pool = self._core.runtime.create_taskpool(self._core.config.threadkit_parallel_limit)
    name = f.__name__
    tasks = []
    try:
      f(*args, **kwargs)
      self._core.log.debug("Starting a parallel task set named %s with %d tasks", name, len(self._parallelizer_tasks))
      for f in self._parallelizer_tasks:
        obj = pool.add_task(f, globalize=False)
        tasks.append(obj)
    finally:
      self._parallelizer_tasks = None
      self.ReleaseLock("_ThreadKit:Parallelizer")
      pool.wait_for_tasks(tasks)
      pool.shutdown()
      del pool
      self._core.log.debug("Parallel task set %s ended", name)
    
  def _task_decorator(self, f):
    self.AcquireLock("_ThreadKit:ParallelizerTask")
    if hasattr(self, '_parallelizer_tasks') and isinstance(self._parallelizer_tasks, list):
      self._parallelizer_tasks.append(f)
    else:
      self._core.log.debug("Unable to create a task for %s - not inside a parallelizer!", f.__name__)
    self.ReleaseLock("_ThreadKit:ParallelizerTask")
    
  def _parallel_decorator(self, f):
    def _parallel_function(*args, **kwargs):
      return self._parallelize_decorator(f, *args, **kwargs)
    return _parallel_function