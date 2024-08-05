#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from __future__ import with_statement

import Framework
import urlparse
import urllib
import cgi
import routes
import threading
import sys
import os
import Queue
import base64
import types
import socket
import Cookie
import mimetools
import traceback
import datetime
#import ctypes
from lxml import etree

try:
  from cStringIO import StringIO
except:
  from StringIO import StringIO

from base import BaseComponent
from Framework.components.notifications import paste
import time

"""
def _async_raise(tid, excobj):
  res = ctypes.pythonapi.PyThreadState_SetAsyncExc(ctypes.c_long(tid), ctypes.py_object(excobj))
  if res == 0:
    raise ValueError("nonexistent thread id")
  elif res > 1:
    # if it returns a number greater than one, you're in trouble, 
    # and you should call it again with exc=NULL to revert the effect
    ctypes.pythonapi.PyThreadState_SetAsyncExc(ctypes.c_long(tid), None)
    raise SystemError("PyThreadState_SetAsyncExc failed")
 
class Thread(threading.Thread):
  def raise_exc(self, excobj):
    assert self.isAlive(), "thread must be started"
    for tid, tobj in threading._active.items():
        if tobj is self:
            _async_raise(tid, excobj)
            return
    
    # the thread was alive when we entered the loop, but was not found 
    # in the dict, hence it must have been already terminated. should we raise
    # an exception here? silently ignore?
  
  def terminate(self):
    # must raise the SystemExit type, instead of a SystemExit() instance
    # due to a bug in PyThreadState_SetAsyncExc
    self.raise_exc(SystemExit)
"""

class HostedResource(object):
  def __init__(self, core, rtype, group, identifier, ext):
    self._core = core
    self._rtype = rtype
    self._group = group
    self._identifier = identifier
    self._ext = ext
  
  def __str__(self):
    filename = self._core.runtime.filename_for_hosted_resource(self._rtype, self._identifier, self._ext)
    file_hash = str(self._core.runtime.hash_for_hosted_resource(self._rtype, self._group, self._identifier, self._ext))
    url = "http://resources-cdn.plexapp.com/%s/%s/%s?h=%s" % (self._rtype, self._group, filename, file_hash)
    return url


class FunctionDecorator(object):
  def __init__(self, f):
    self._f = f
  
  def __call__(self, *args, **kwargs):
    return self._f(*args, **kwargs)

class IndirectFunction(FunctionDecorator): pass
class DeferredFunction(FunctionDecorator): pass
    
class callback_string(unicode):
  def __init__(self, u):
    self.post_url = None
    self.post_headers = {}
    
class indirect_callback_string(callback_string):
  pass


view_modes = {
  "List": 65586, "InfoList": 65592, "MediaPreview": 458803, "Showcase": 458810, "Coverflow": 65591, 
  "PanelStream": 131124, "WallStream": 131125, "Songs": 65593, "Seasons": 65593, "Albums": 131123, 
  "Episodes": 65590,"ImageStream":458809,"Pictures":131123
}


class ViewGroupRecord(object):

  def __init__(self, viewMode=None, mediaType=None, type=None, menu=None, cols=None, rows=None, thumb=None, summary=None):
    if viewMode:
      if viewMode not in view_modes:
        raise Framework.exceptions.FrameworkException("%s is not a valid view mode." % viewMode)
      self.viewMode = view_modes[viewMode]
    else:
      self.viewMode = None
    self.mediaType = mediaType
    self.viewType = type
    self.viewMenu = menu
    self.viewCols = cols
    self.viewRows = rows
    self.viewThumb = thumb
    self.viewSummary = summary


class BaseTaskQueue(object):
  """
    BaseTaskQueue implements a simple queue that adds tasks to a task pool. This class is
    pointless when instantiated directly as it simply drops tasks into the pool as soon as
    they're received. One of the subclasses should be used instead. 
  """
  
  def __init__(self, pool):
    self._pool = pool
    self._queue = Queue.Queue()
    self._pool._runtime.create_thread(self._thread)
  
  def _thread(self):
    dead = False
    while not dead:
      dead = not self._process()
    self._pool._runtime._core.log.debug("Finished dispatching queued tasks to the pool - ending the thread.")
      
  def _process(self):
    obj = self._queue.get()
    self._pool._add(obj)
    self._queue.task_done()
    return obj != None
    
  def _add(self, task):
    self._queue.put(task)
    
  def add_task(self, f, args=[], kwargs={}, important=False):
    obj = TaskObject(f, args, kwargs, important)
    self._add(obj)
    return obj
    
  def end(self):
    self._add(None)
    
  @property
  def size(self):
    return self._queue.qsize()
    
    
class BlockingTaskQueue(BaseTaskQueue):
  """
    A task queue with an event assigned to it. The queue blocks new tasks from being added to the
    pool until the event is set.
  """
  
  def __init__(self, pool, event):
    self._event = event
    BaseTaskQueue.__init__(self, pool)
    
  def _process(self): 
    self._event.wait()
    return BaseTaskQueue._process(self)
    
    
class LimitingTaskQueue(BaseTaskQueue):
  """
    A task queue that only allows a maximum number of tasks to enter the pool at any one time. The
    queue will block when the number of tasks in the pool is equal to the defined limit and allow
    subsequent tasks to be processed only when existing ones have finished.
  """
  
  def __init__(self, pool, limit):
    self._semaphore = pool._runtime.semaphore(limit=limit)
    BaseTaskQueue.__init__(self, pool)

  def _process(self):
    obj = self._queue.get()
    self._semaphore.acquire()
    self._pool.add_task(self._run, kwargs=dict(obj=obj))
    self._queue.task_done()
    return obj != None
    
  def _run(self, obj):
    ret = None
    if isinstance(obj, TaskObject):
      try:
        obj._exec()
        ret = obj.result
      except:
        self._pool._runtime._core.log_exception('Exception in task thread')
    self._semaphore.release()
    return ret


class TaskObject(object):
  """
    Task objects define a function to execute, args/kwargs to pass to the function, the function
    result, the function's completion status and a priority flag.
  """

  def __init__(self, f, args, kwargs, important=False):
    self._complete = threading.Event()
    self._f = f
    self._args = args
    self._kwargs = kwargs
    self._result = None
    self._important = important
    
  def wait(self):
    """
      Blocks the calling thread until the task is completed.
    """
    return self._complete.wait()
    
  @property
  def result(self):
    """
      Blocks the calling thread until the task is completed, then returns the result
    """
    self._complete.wait()
    return self._result
    
  def _exec(self):
    """
      Executes the function, setting the result and handling any exceptions, then sets the
      completed event to unblock any waiting threads.
    """
    try:
      self._result = self._f(*self._args, **self._kwargs)
    finally:
      self._complete.set()


class TaskThread(object):
  """
    Task threads handle the execution of tasks belonging to a task pool.
  """
  
  def __init__(self, pool, priority=False):
    """
      Store a reference to the task pool and whether this is a priority thread or not.
      Priority threads only execute tasks in the task pool's priority queue. Standard
      queues give priority to tasks in the priority queue, then fall back to the standard
      queue if no important tasks have been added
    """
    self._priority = priority
    self._pool = pool
    self._running = True
    self._pool._runtime.create_thread(self._start)
  
  def stop(self):
    self._running = False
  
  def _start(self):
    """
      Keep fetching tasks from the pool and processing them
    """
    while self._running:
      obj = None
      important = False
      
      # If this is a priority thread, only execute tasks in the priority queue. Otherwise, check
      # for important tasks first, then wait for standard ones.
      if self._priority:
        obj = self._pool._priority_queue.get()
        important = True
      else:
        if not self._pool._priority_queue.empty():
          obj = self._pool._priority_queue.get()
          important = True
        else:
          obj = self._pool._queue.get()
      
      # If executing an important task, put None in the standard queue to wake up non-priority threads
      # for processing of additional important tasks (if any)     
      if important:
        self._pool._queue.put(None)

      # If the fetched object is a TaskObject, execute it
      if obj != None and isinstance(obj, TaskObject):
        try:
          obj._exec()
        except:
          self._pool._runtime._core.log_exception('Exception in task thread')
        
      # Notify the relevant queue that processing has completed
      if important:
        self._pool._priority_queue.task_done()
      else:
        self._pool._queue.task_done()
        

class TaskPool(object):
  """
    Task pools manage a number of worker threads and the distribution of task objects
    between them.
  """
  
  def __init__(self, runtime, threadcount=8, prioritycount=0):
    # Store a reference to the runtime, and store the thread count and priority count.
    self._runtime = runtime
    self._threadcount = threadcount
    self._prioritycount = prioritycount
    
    # Create the queues
    self._queue = Queue.Queue()
    self._priority_queue = Queue.Queue()
    
    # Create the required number of threads (setting the priority flag accordingly)
    self._threads = []
    for x in range(prioritycount):
      self._threads.append(TaskThread(self, True))
    for x in range(threadcount - prioritycount):
      self._threads.append(TaskThread(self, False))
      
  def _add(self, obj):
    """
      Adds a given task object to the relevant queue.
    """
    if obj._important:
      self._priority_queue.put(obj)
    else:
      self._queue.put(obj)
      
  def add_task(self, f, args=[], kwargs={}, important=False, globalize=True):
    """
      Creates a task object with the given attributes and adds it to the pool.
    """
    #FIXME: Make sure globalization works
    #if globalize:
    #  f = self._runtime._globalize(f)
    obj = TaskObject(f, args, kwargs, important)
    self._add(obj)
    return obj
    
  def wait_for_tasks(self, tasks=[]):
    """
      Blocks the calling thread until all listed tasks have completed.
    """
    for obj in tasks:
      if isinstance(obj, TaskObject):
        obj.wait()
  
  def shutdown(self):
    # Stop all the threads.
    for t in self._threads:
      t.stop()
      
    # Wake them all up by posting to the queues.
    for t in self._threads:
      self._queue.put(None)
      self._priority_queue.put(None)
  
  def create_blocking_queue(self, event):
    """
      Creates a BlockingTaskQueue for this pool.
    """
    return BlockingTaskQueue(self, event)
    
  def create_limiting_queue(self, limit):
    """
      Creates a LimitingTaskQueue for this pool.
    """
    return LimitingTaskQueue(self, limit)
    
class RouteManager(Framework.CoreObject):
  def __init__(self, core, runtime, method, group):
    Framework.CoreObject.__init__(self, core)
    self._method = method
    self._route_lock = runtime.lock()
    self._route_controllers = {}
    self._route_mapper = routes.Mapper()
    self._route_generator = routes.URLGenerator(self._route_mapper, {})
    self._route_group = group
    self._arg_types = {}
    
  def connect_route(self, path, f, allow_sync=False, **kwargs):
    if isinstance(f, FunctionDecorator):
      f = f._f
    self._route_lock.acquire()
    try:
      # Get the function's identity
      identity = str(f)
      if identity not in self._route_controllers:
        self._route_controllers[identity] = f

      # Flag whether the route can by synced.
      f.__dict__['f_syncable'] = allow_sync
      
      # Store any arg types provided
      arg_types = {}
      for name, value in kwargs.items():
        if isinstance(value, type):
          arg_types[name] = value
          del kwargs[name]

      if len(arg_types) > 0:
        self._arg_types[identity] = arg_types
        
      self._route_mapper.connect(None, path, controller=identity, action = '__NONE__', **kwargs)
      if (self._route_group == None and Framework.constants.flags.log_route_connections in self._core.flags) or Framework.constants.flags.log_all_route_connections in self._core.flags:
        self._core.log.debug("Connecting route %s (group: %s) to %s (%s)", path, str(self._route_group), f.__name__, identity)

    finally:
      self._route_lock.release()


  def generate_route(self, f, **kwargs):
    if 'action' not in kwargs:
      kwargs['action'] = '__NONE__'
    identity = str(f)

    # Convert any arg types required
    arg_types = self._arg_types.get(identity, {})
    for arg_name, arg_type in arg_types.items():
      if arg_name in kwargs:
        arg_value = kwargs[arg_name]

        if arg_value != None:
          # Raise an exception if a type was specified but the provided value was incorrect
          if not isinstance(arg_value, arg_type):
            raise Framework.exceptions.FrameworkException("The argument '%s' of function %s expects a value of type '%s', but received a value of type '%s'." % (arg_name, f.__name__, arg_type.__name__, type(arg_value).__name__))

          if arg_type in (list, tuple, dict):
            kwargs[arg_name] = self._core.data.json.to_string(arg_value)
          elif arg_type == bool:
            kwargs[arg_name] = "1" if arg_value else "0"

    # Convert None to the special __NONE__ string
    for arg_name, arg_value in kwargs.items():
      if arg_value == None:
        kwargs[arg_name] = '__NONE__'

    # Set the correct prefix depending on the route group.
    if self._route_group == 'management':
      prefix = '/:'
    elif self._route_group == 'internal':
      prefix = '/:/plugins/%s' % self._core.identifier
    else:
      prefix = ''

    return prefix + self._route_generator(controller=identity, **kwargs)


  def match_route(self, path, kwargs={}):
    self._route_lock.acquire()
    try:
      d = self._route_mapper.match(path)
      if not d:
        raise Framework.exceptions.FrameworkException("No route found matching '%s'" % path)
      f = self._route_controllers[d['controller']]
      d.update(kwargs)
      del d['controller']
      if d.get('action') == '__NONE__': del d['action']

      # Convert any arg types required
      arg_types = self._arg_types.get(str(f), {})
      for arg_name in d:
        # Get the arg value
        arg_value = d[arg_name]

        # Convert the special __NONE__ string.
        if arg_value == '__NONE__':
          d[arg_name] = None
        
        # Otherwise, if the arg has a type specified, convert it if necessary.
        elif arg_name in arg_types:
          arg_type = arg_types[arg_name]
          if arg_type in (list, tuple, dict):
            arg_value = self._core.data.json.from_string(arg_value)
          elif arg_type == bool:
            arg_value = True if arg_value == "1" else False
          d[arg_name] = arg_type(arg_value)

      self._core.sandbox.format_kwargs_for_function(f, d)
      return f, d  
      
    except:
      self._core.log_exception('Exception matching route for path "%s"', path)
      return None, None
    finally:
      self._route_lock.release()
      
class CachedHTTPRequestHandler(object):
  def __init__(self, core, request_text):
    self._core = core
    pos = request_text.find('\r\n\r\n')
    self._core.log.debug("Found \\r\\n\\r\\n at pos %d" % pos)
    header_data = request_text[:pos]
    self._core.log.debug("Constructing response with header data: "+header_data)
    self.body = request_text[pos+4:]
    self.headers = mimetools.Message(StringIO(header_data))
    
  def getheaders(self, name):
    # Only handle Set-Cookie headers; cookielib seems incapable of correctly parsing this, so use
    # SimpleCookie to split multiple cookies up into individual headers
    self._core.log.debug("Trying to get headers named '%s'", name)
    if name == 'Set-Cookie':
      headers = []
      for header_string in self.headers.getallmatchingheaders('Set-Cookie'):
        self._core.log.debug("Parsing header '%s'", header_string)
        header_string = header_string[header_string.find(':')+1:].strip()
        cookie = Cookie.SimpleCookie(header_string)

        for name in cookie:
          # Sometimes a comma gets tacked onto the end of the domain - remove it if found
          morsel = cookie[name]
          domain = morsel['domain']
          if domain and domain[-1] == ',':
            morsel['domain'] = domain[:-1] 
          
          # Convert the single cookie to a header string and add it to the list
          headers.append(unicode(morsel)[12:])
      self._core.log.debug("Extracted cookie headers: %s" % str(headers))
      return headers
    else:
      return []
    
  def info(self):
    return self
    

class Runtime(BaseComponent):
  """
    The Runtime component manages the state of the current plug-in, including request handling
    and thread management. Most of the functionality provided here is exposed via the RuntimeKit,
    and ThreadKit APIs.
  """

  def _init(self):
    self.view_groups = {}
    
    self._thread_locks = {}
    self._thread_events = {}
    self._thread_semaphores = {}
    self._interface_args = {}
    
    self._route_groups = {}
    self._handlers = []
    self._resource_hashes = None
    self._resource_hashes_update_time = None
    self._resource_hashes_lock = threading.Lock()

    self._taskpool = TaskPool(
      self,
      threadcount = self._core.config.taskpool_maximum_threads,
      prioritycount = self._core.config.taskpool_priority_threads
    )

  def _get_route_manager(self, group, method):
    if group not in self._route_groups:
      self._route_groups[group] = {}
    if method not in self._route_groups[group]:
      self._route_groups[group][method] = RouteManager(self._core, self, method, group)
    return self._route_groups[group][method]
    
  def install_handler(self, handler_class, allow_sync=False, **kwargs):
    try:
      handler = handler_class(self._core, **kwargs)
      self._handlers.append(handler)
      for route_function in handler.routes:
        for path, methods, kwargs in route_function.f_routes:
          self.connect_route(path, route_function, methods, handler._route_group(), allow_sync = allow_sync, **kwargs)
    except:
      self._core.log_exception("Exception installing handler '%s'", handler_class.__name__)


  def find_handler(self, handler_class):
    for handler in self._handlers:
      if isinstance(handler, handler_class):
        return handler

    raise Framework.exceptions.FrameworkException("Unable to find handler of type '%s'" % handler_class.__name__)
      

  def connect_route(self, path, f, method=['GET'], route_group=None, allow_sync=False, **kwargs):
    if isinstance(method, basestring):
      method = [method]
    # Register the route for each method provided
    for m in method:
      self._get_route_manager(route_group, m).connect_route(path, f, allow_sync=allow_sync, **kwargs)
    
  def generate_route(self, f, method='GET', route_group=None, **kwargs):
    # If no route group is provided, see if the function belongs to a handler class with a group defined.
    if route_group == None and hasattr(f, 'im_class'):
      cls = f.im_class
      if hasattr(cls, '_route_group'):
        route_group = f.im_class._route_group()
        
    return self._get_route_manager(route_group, method).generate_route(f, **kwargs)
    
  def match_route(self, path, method='GET', route_group=None, kwargs={}):
    return self._get_route_manager(route_group, method).match_route(path, kwargs)
   

  @property
  def os(self):
    """
      Returns the current OS name (e.g. MacOSX or Linux)
    """
    if sys.platform == "win32":
      os_name = "Windows"
    else:
      os_name = os.uname()[0]
    if os_name in self._core.config.os_map:
      return self._core.config.os_map[os_name]
  
  @property
  def cpu(self):
    """
      Returns the current CPU name (e.g. x86 or MIPS)
    """
    if sys.platform == "win32":
      #TODO: Support x64 CPUs on Windows
      cpu_name = "i386"
    else:
      cpu_name = os.uname()[4]

    return self._core.config.cpu_map.get(cpu_name, cpu_name)
      
  @property
  def platform(self):
    return '%s-%s' % (self.os, self.cpu)
    
  def add_prefix_handler(self, prefix, handler, name, thumb, art, titleBar, share=False, allow_sync=False):
    self._core.log.debug("Adding a prefix handler for '%s' ('%s')", name, prefix)
    
    if thumb == None:
      for thumb_name in ('icon-default.png', 'icon-default.jpg'):
        if self._core.storage.resource_exists(thumb_name):
          thumb = self.external_resource_path(thumb_name)
      if self._core.identifier != 'com.plexapp.system' and thumb == None:
        thumb = self.hosted_resource_url('image', 'source')
    else:
      thumb = self.external_resource_path(thumb)
            
    if art == None:
      for art_name in ('art-default.jpg', 'art-default.jpg'):
        if self._core.storage.resource_exists(art_name):
          art = self.external_resource_path(art_name)
      if self._core.identifier != 'com.plexapp.system' and art == None:
        art = self.hosted_resource_url('image', 'art')
    else:
      art = self.external_resource_path(art)
            
    if titleBar == None:
      for title_bar_name in ('titlebar-default.png', 'titlebar-default.jpg'):
        if self._core.storage.resource_exists(title_bar_name):
          titleBar = self.external_resource_path(title_bar_name)
    else:
      titleBar = self.external_resource_path(titleBar)
    
    self.install_handler(
      Framework.handlers.generate_prefix_handler(prefix, handler),
      name = name,
      thumb = thumb,
      art = art,
      titleBar = titleBar,
      share = share,
      allow_sync = allow_sync,
    )
    
  def add_view_group(self, name, viewMode=None, mediaType=None, type=None, menu=None, cols=None, rows=None, thumb=None, summary=None):
    self.view_groups[name] = ViewGroupRecord(viewMode, mediaType, type, menu, cols, rows, thumb, summary)
  
  def create_query_string(self, kwargs):
    return 'function_args=' + Framework.utils.pack(kwargs)
    
  def parse_query_string(self, querystring):
    kwargs = {}
    
    # Check for an empty querystring
    if len(querystring) == 0:
      return kwargs
      
    # Check for empty args
    for arg in querystring.split('&'):
      if len(arg) > 1 and arg[-1] == '=':
        kwargs[arg[:-1]] = None
    
    # Parse populated args
    qs_args = cgi.parse_qs(querystring)
    if 'function_args' in qs_args:
      try:
        packed_args = qs_args['function_args'][0]
        kwargs = Framework.utils.unpack(packed_args)
      except:
        self._core.log_exception("Exception decoding function arguments")
    for arg in qs_args:
      if arg != 'function_args':
        kwargs[arg] = qs_args[arg][0]
    return kwargs
  
  def add_private_request_handler(self, f):
    self._core.log.critical("Private handlers are no longer supported; couldn't register %s", str(f))
    
  def handle_request(self, request): #path, headers, method='GET', body=None):
    """
      Handles a given path & set of headers, directing a request to the relevant plug-in or
      framework function and returning the result.
    """
    try:
      self._core.log.debug("Handling request %s %s", request.method, request.uri)
      
      result = None
      status = None
      headers = {}
      controller = None
      kwargs = {}
      original_type = None
      
      # Parse the path
      req = urlparse.urlparse(request.uri)
      path = req.path
      kwargs = self.parse_query_string(req.query)

      # Replace '+' with '%20', because iOS likes to send these back sometimes.
      path = path.replace('+', '%20')
      
      # If we received an auth token as a querystring arg, move it to the header dictionary.
      if Framework.constants.arguments.token in kwargs:
        request.headers[Framework.constants.header.token] = kwargs[Framework.constants.arguments.token]
        del kwargs[Framework.constants.arguments.token]

      # Check for X-Plex-Token too, since this won't get converted automatically on the node
      # TODO: Maybe convert all X-Plex arguments here?
      if Framework.constants.header.token in kwargs:
        request.headers[Framework.constants.header.token] = kwargs[Framework.constants.header.token]
        del kwargs[Framework.constants.header.token]

      # Set the request property of the current execution context and get the transaction ID.
      context = self._core.sandbox.context
      context.request = request
      
      # If a postURL argument has been provided, copy the request body into the
      # context's cached response dictionary.
      #
      # Also rewrite the method as a GET, since the POST behaviour is being made
      # use of by the framework, not the plug-in code itself.
      #
      if 'postURL' in kwargs:
        postURL = kwargs['postURL']
        postBody = request.body
        self._core.log.debug("Received data (%d bytes) for URL '%s' via POST", len(postBody), postURL)
        context.cached_http_responses[postURL] = CachedHTTPRequestHandler(self._core, postBody)
        context.add_cached_response_cookies()
        del kwargs['postURL']
        request.method = 'GET'
      
      if path[-1] == '/': path = path[:-1]
      pathNouns = [urllib.unquote(p) for p in path.split('/')]
      pathNouns.pop(0)

      # Return simple OK for requests for root
      if len(pathNouns) == 0:
        result = "OK\n"
        f = None
          
      # Hand over to a request handler.
      else:
        # Compute the route group and path
        full_path = '/'+'/'.join(pathNouns)

        # Paths starting with /:/ are special...
        if len(pathNouns) and pathNouns[0] == ':':
          # If the path is under /:/plugins, it has been routed from the HTTP interface to this
          # plug-in by its' identifier. This is an internal request.
          if pathNouns[1] == 'plugins':
            route_group = Framework.handlers.InternalRequestHandler._route_group()
            nouns = pathNouns[3:]

          # Anything else under /:/ could only have come from the media server. This is a
          # management request.
          else:
            route_group = Framework.handlers.ManagementRequestHandler._route_group()
            nouns = pathNouns[1:]

        # Normal paths use the default group.
        else:
          route_group = None
          nouns = pathNouns
        
        route_path = '/'+'/'.join(nouns)

        # Check for a prefix to assign to the context.
        for prefix in [handler.prefix for handler in self._handlers if isinstance(handler, Framework.handlers.PrefixRequestHandler)]:
          if full_path.startswith(prefix):
            context.prefix = prefix

            # Check for internal routes called via the prefix. If found, strip the prefix & change the group.
            if route_path.startswith(prefix + '/:/'):
              route_group = Framework.handlers.InternalRequestHandler._route_group()
              route_path = route_path[len(prefix)+2:]

            break

        # Try to find a matching route
        f,d = self.match_route(route_path, request.method, route_group, kwargs)
        if f:
          self._core.log.debug("Found route matching "+full_path)

          def convert_arg_to_flag(arg, flag):
            if arg in d:
              if d[arg] == '1':
                context.flags.append(flag)
              del d[arg]
              
          # Backwards compatibility: Parse media info sent via the URL.
          if 'mediaInfo' in d:
            try:
              context.media_info = self._core.data.json.from_string(urllib.unquote(d['mediaInfo']))
            except:
              pass
            del d['mediaInfo']

          # Flag the context as indirect if specified by a querystring arg.
          convert_arg_to_flag('indirect', Framework.code.context.flags.indirect)
          
          # Flag the context as syncable if specified by a querystring arg or the route definition.
          if hasattr(f, 'f_syncable') and f.f_syncable:
            context.flags.append(Framework.code.context.flags.syncable)
          convert_arg_to_flag('syncable', Framework.code.context.flags.syncable)
          
          # Keep references to the function and kwargs for later.
          controller = f
          kwargs = d

          # Get the result from the function.
          result = f(**d)

        else:
          self._core.log.error("Could not find route matching "+full_path)

      # Page results if requested.
      if Framework.constants.header.container_start in request.headers and Framework.constants.header.container_size in request.headers and isinstance(result, Framework.objects.MediaContainer):
        try:
          start = int(request.headers[Framework.constants.header.container_start])
          end = start + int(request.headers[Framework.constants.header.container_size])
          total_size = len(result)
          result = result[start:end]
          result.SetHeader(Framework.constants.header.container_start, str(start))
          result.SetHeader(Framework.constants.header.container_total_size, str(total_size))
          result.totalSize = total_size

        except:
          self._core.log_exception('Exception when calculating paged results')

      original_type = type(result)
      status, headers, body = self.construct_response(result, f)
    
    except KeyboardInterrupt:
      raise # re-raise KeyboardInterrupts - let the interface catch those
    
    except Framework.exceptions.UnauthorizedException:
      self._core.log.debug("Unauthorized")
      status = 401
      headers = {}
      body = ''

    except Framework.exceptions.BadRequestException:
      self._core.log.debug("Bad Request")
      status = 400
      headers = {}
      body = ''

    except Exception, e:
      headers = {}
        
      # If the exception was a timeout error, return 504, otherwise return 500 with more info
      if isinstance(e, Framework.exceptions.URLError) and len(e.args) > 0 and isinstance(e.args[0], socket.timeout):
        status = 504
        body = ''

      else:
        self._core.log_exception("Exception")

        # Report the error unless it's non-critical
        if not isinstance(e, Framework.exceptions.PlexNonCriticalError):
          self._report_error(request, controller, kwargs, e)

        # Convert regular exceptions to the Plex error format
        if not isinstance(e, Framework.exceptions.PlexError):
          e = Framework.exceptions.PlexError(2000, '%s: %s' % (str(type(e).__name__), e.message), self._core.traceback())
          
        status = 500

        el = self._core.data.xml.element('Response')
        el.set('code', str(e.code))
        el.set('status', e.status)
        if e.traceback:
          el.append(self._core.data.xml.element('Traceback', e.traceback))
        
        body = self._core.data.xml.to_string(el)
        headers['Content-Type'] = 'application/xml'
    
    finally:
      # If the status code was set manually, override the code that was set automatically
      if context.response_status != None:
        status = context.response_status
      
      # Get response headers from the context
      final_headers = context.get_final_headers()  
      
      # Don't return context headers when the status is 401 (Unauthorized)
      if status != 401:
        final_headers.update(headers)
        headers = final_headers
    
      # Log the response code
      self._core.log.debug("Response: [%d] %s%d bytes", status, (original_type.__name__ + ", ") if original_type else '', len(body))
    
    return (status, headers, body)
      
      
  def construct_response(self, result, caller=None):
    """
      Converts a function call result into a server response. Handles conversion of
      Framework Objects, XML elements, strings or 'None'.
    """
    resultStr = None
    
    if result == None:
      resultStr = ''
      resultHeaders = {}
      resultStatus = 404
      
    elif isinstance(result, Framework.modelling.objects.Object):
      try:
        # Backwards compatibility: Apply media info stored in the context to the returned object.
        context = self._core.sandbox.context
        if Framework.code.context.flags.indirect in context.flags and context.media_info:
          for obj in result.objects:
            for item in obj.items:
              [setattr(item, key, value) for key, value in context.media_info.items()]
              

        el = result._to_xml()

        # Set the "allowSync" attribute if the context has the syncable flag set.
        if Framework.code.context.flags.syncable in context.flags:
          el.set('allowSync', '1')

        resultStr = self._core.data.xml.to_string(el)
        resultStatus = result._get_response_status()
        resultHeaders = result._get_response_headers()
        
        # If the result is a MediaContainer with the noCache attribute set, make sure we set the Cache-Control header properly
        if isinstance(result, Framework.api.objectkit.ObjectContainer) and result.no_cache == True:
          resultHeaders['Cache-Control'] = 'no-cache'

        if resultStr is not None:
          resultStr = str(resultStr)
        else:
          resultStr = ""
      except:
        self._core.log_exception("Exception when constructing response")
        resultStr = None
    
    # If the result is an old-school Framework Object, return its content, status and headers
    elif isinstance(result, Framework.objects.Object):
      try:
        resultStr = result.Content()
        resultStatus = result.Status()
        resultHeaders = result.Headers()

        # If the result is a MediaContainer with the noCache attribute set, make sure we set the Cache-Control header properly
        if (isinstance(result, Framework.objects.MediaContainer) or isinstance(result, Framework.objects.MessageContainer)) and hasattr(result, 'noCache') and getattr(result, 'noCache') == True:
          resultHeaders['Cache-Control'] = 'no-cache'

        if resultStr is not None:
          resultStr = str(resultStr)
        else:
          resultStr = ""
      except:
        self._core.log_exception("Exception when constructing response")
        resultStr = None

    # If the result is a string, convert it to unicode and populate the headers/status with
    # valid values
    elif isinstance(result, basestring):
      resultStr = str(result)
      resultStatus = 200
      resultHeaders = {'Content-type': 'text/plain'}

    # If the result is a list or dict, convert it to JSON and populate the header/status with
    # valid values
    elif isinstance(result, (list, dict)):
      resultStr = self._core.data.json.to_string(result)
      resultStatus = 200
      resultHeaders = {'Content-type': 'application/json'}
      
    # If the result is an lxml.etree element, convert it to a unicode string and populate the
    # headers/status with valid values
    elif isinstance(result, etree._Element):
      resultStr = str(self._core.data.xml.to_string(result))
      resultStatus = 200
      resultHeaders = {'Content-type': 'application/xml'}
      
    # If the result is a boolean value, return a 200 for True or a 404 for False
    elif isinstance(result, bool):
      if result == True:
        resultStatus = 200
      else:
        resultStatus = 404
      resultStr = ''
      resultHeaders = {}
      
    # Otherwise, return a 500 error
    if resultStr == None:
      resultStr = ""
      resultStatus = 500
      resultHeaders = {}
      self._core.log.debug("Unable to handle response type: %s", str(type(result)))
      
    # Add response headers from the current context
    resultHeaders.update(self._core.sandbox.context.response_headers)

    return resultStatus, resultHeaders, resultStr
    
    #TODO: Check if plugin is broken

  @property
  def current_prefix(self):
    if hasattr(self._core, 'sandbox') and self._core.sandbox.context.prefix:
      return self._core.sandbox.context.prefix
    else:
      for handler in self._handlers:
        if isinstance(handler, Framework.handlers.PrefixRequestHandler):
          return handler.prefix

    return self._core.channel_prefix

 
  def generate_callback_path(self, func, ext=None, post_url=None, post_headers={}, **kwargs):
    #TODO: Make this a common handler that works with service functions too.
    
    # If we were passed an IndirectFunction, get the actual function from the object
    if isinstance(func, IndirectFunction):
      func = func._f
      indirect = True
    else:
      indirect = False
    try:
      s = self.generate_route(func, **kwargs)
    except:
      # Raise if auto-generated routes are disabled.
      if self._core.policy.enable_auto_generated_routes == False:
        raise

      # Log a warning.
      self._core.log.warn("Generating a callback path for a function with no route: %s", str(func))

      # Build the path.
      query_string = self._core.runtime.create_query_string(kwargs)
      query_string = '?' + query_string if (query_string and len(query_string) > 0) else ''
      ext = '.' + ext if (ext and ext[0] != '.') else ''
      s = "%s/:/function/%s%s%s" % (self.current_prefix, func.__name__, ext, query_string)
      
    # If this is an indirect callback, use an indirect_callback_string object, otherwise use a callback_string object.
    s = indirect_callback_string(s + "&indirect=1") if indirect else callback_string(s)

    # Set additional properties defining HTTP POST callback behaviour
    s.post_url = post_url
    s.post_headers = post_headers
      
    return s
        

  def external_resource_path(self, itemname, sandbox=None):
    """
      Generates an external path for accessing a bundled resource file.
    """
    if not itemname: return
    if self._core.storage.resource_exists(itemname, sandbox):
      mtime = self._core.storage.resource_mtime(itemname, sandbox)
      path = "/:/plugins/%s/resources/%s?t=%d" % (self._core.identifier, itemname, mtime)
      if sandbox and sandbox.identifier != self._core.identifier:
        path += '&identifier=%s' % sandbox.identifier
      return path


  # Threading functions
        
  def create_thread(self, f, log=True, sandbox=None, globalize=True, *args, **kwargs):
    """
      Spawns a new thread with the given function, args & kwargs
    """
    th = threading.Thread(
      None,
      self._start_thread,
      name=f.__name__,
      args=(),
      kwargs=dict(
        f = f,
        args = args,
        kwargs = kwargs,
        sandbox = sandbox,
        context_values = sandbox.context.export_values() if sandbox and globalize else None,
      )
    )
    th.start()
    if log and (self._core.config.log_internal_component_usage or f.__name__[0] != '_'):
      self._core.log.debug("Created a thread named '%s'" % f.__name__)
    return th
    
  def _start_thread(self, f, args, kwargs, sandbox=None, context_values=None):
    # Import values into the new context.
    if sandbox and context_values:
      sandbox.context.import_values(context_values)

    try:
      f(*args, **kwargs)
    except:
      self._core.log_exception("Exception in thread named '%s'", f.__name__)
      
  def create_timer(self, interval, f, log=True, sandbox=None, globalize=True, *args, **kwargs):
    """
      Schedules a thread with the given function, args & kwargs to fire after the given
      interval.
    """
    timer = threading.Timer(
      interval,
      self._start_timed_thread,
      args=(),
      kwargs=dict(
        f = f,
        log = log,
        args = args,
        kwargs = kwargs,
        sandbox = sandbox,
        context_values = sandbox.context.export_values() if sandbox and globalize else None
      )
    )
    timer.start()
    if log and (self._core.config.log_internal_component_usage or f.__name__[0] != '_'):
      self._core.log.debug("Scheduled a timed thread named '%s'", f.__name__)
    return timer
    
  def _start_timed_thread(self, f, log, args, kwargs, sandbox=None, context_values=None):
    # Internal code for running the given function and handling exceptions
    if log and (self._core.config.log_internal_component_usage or f.__name__[0] != '_'):
      self._core.log.debug("Starting timed thread named '%s'", f.__name__)

    self._start_thread(f, args, kwargs, sandbox, context_values)  
      
  def _key(self, key):
    if key == None:
      raise Framework.exceptions.FrameworkException('Cannot lock the None object')
    elif isinstance(key, basestring):
      return key
    else:
      return id(key)
    
  def acquire_lock(self, key, addToLog=True):
    """
      Acquires a named lock.
    """
    try:
      key = self._key(key)
      if key not in self._thread_locks:
        self._thread_locks[key] = threading.Lock()
      if addToLog:
        if self._core.config.log_internal_component_usage and '_Storage:' not in str(key):
          self._core.log.debug("Acquiring the thread lock '%s'" % str(key))
      self._thread_locks[key].acquire()
      return True
    except:
      self._core.log_exception("Unable to acquire the thread lock '%s'" % str(key))
      raise
      
  def release_lock(self, key, addToLog=True):
    """
      Releases a named lock.
    """
    key = self._key(key)
    if key in self._thread_locks:
      if addToLog:
        if self._core.config.log_internal_component_usage and '_Storage:' not in str(key):
          self._core.log.debug("Releasing the thread lock '%s'" % str(key))
      self._thread_locks[key].release()
      return True
    else:
      self._core.log_exception("Unable to find a thread lock named '%s'" % str(key))
      return False
      
  def lock(self, key = None):
    """
      Returns a lock object. If a key is provided, the named lock is returned, otherwise a new
      lock object is generated.
    """
    if key:
      key = self._key(key)
    if key in self._thread_locks:
      lock = self._thread_locks[key]
    else:
      lock = threading.Lock()
      if key != None:
        self._thread_locks[key] = lock
    return lock
      
  def event(self, key = None):
    """
      Returns an event object. If a key is provided, the named event is returned, otherwise a new
      event object is generated.
    """
    if key:
      key = self._key(key)
    if key in self._thread_events:
      event = self._thread_events[key]
    else:
      event = threading.Event()
      if key != None:
        self._thread_events[key] = event
    return event
      
  def block_event(self, key):
    """
      Clears the named event, causing any threads waiting for it to block.
    """
    self.event(key).clear()
    
  def unblock_event(self, key):
    """
      Sets the named event, causing any threads currently waiting for it to be unblocked.
    """
    self.event(key).set()
    
  def wait_for_event(self, key, timeout=None):
    """
      Blocks the calling thread until the named event is set.
    """
    self.event(key).wait(timeout)
    return self.event(key).isSet()
    
  def semaphore(self, key = None, limit=1):
    """
      Returns an semaphore object. If a key is provided, the named semaphore is returned, otherwise
      a new semaphore object is generated.
    """
    if key:
      key = self._key(key)
    if key in self._thread_semaphores:
      sem = self._thread_semaphores[key]
    else:
      sem = threading.Semaphore(limit)
      if key != None:
        self._thread_semaphores[key] = sem
    return sem
    
  @property
  def taskpool(self):
    """
      Returns the runtime's task pool
    """
    return self._taskpool
    
  def create_taskpool(self, threadcount, prioritycount=0):
    """
      Creates a new task pool (not used by the framework - meant for access via CoreKit)
    """
    return TaskPool(self, threadcount)
    
  def _expand_identifier(self, identifier):
    if identifier[0:2] == '..':
      return 'com.plexapp.'+identifier[2:]
    elif identifier[0] == '.':
      return self._core.identifier[:self._core.identifier.rfind('.')]+identifier
    else:
      return identifier


  def get_resource_hashes(self):
    with self._resource_hashes_lock:
      if self._resource_hashes_update_time == None or self._resource_hashes_update_time < datetime.datetime.now():
        try:
          if self._core.identifier == 'com.plexapp.system' or self._core.config.daemonized:
            json = self._core.networking.http_request("http://resources-cdn.plexapp.com/hashes.json", timeout=5).content
          else:
            json = self._core.networking.http_request("http://127.0.0.1:32400/:/plugins/com.plexapp.system/resourceHashes", timeout=10).content

          self._resource_hashes = self._core.data.json.from_string(json)
          self._resource_hashes_update_time = datetime.datetime.now() + datetime.timedelta(hours=24)

        except:
          self._core.log_exception("Exception getting hosted resource hashes")

          # If fetching resources failed, don't try again for 1 hour.
          self._resource_hashes_update_time = datetime.datetime.now() + datetime.timedelta(hours=1)

      return self._resource_hashes


  def filename_for_hosted_resource(self, resource_type, identifier=None, ext=None):
    if identifier == None:
      identifier = self._core.identifier
      
    if ext == None:
      if resource_type == "image":
        ext = "jpg"
      if resource_type == "html":
        ext = "html"
    
    filename = identifier
    if ext != None:
      filename += "." + ext
      
    return filename

    
  def hash_for_hosted_resource(self, resource_type, group, identifier=None, ext=None):
    hashes = self.get_resource_hashes()
    if hashes is not None:
      filename = self.filename_for_hosted_resource(resource_type, identifier, ext)
      return hashes.get(resource_type, {}).get(group, {}).get(filename)


  def hosted_resource_url(self, resource_type, group, identifier=None, ext=None):
    return HostedResource(self._core, resource_type, group, identifier, ext)
    

  def _report_error(self, request, controller, kwargs, e):
    try:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      log_lines = list(self._core.sandbox.context.log)
      
      def _thread(request, controller, kwargs, e):
        # Grab the traceback frames.
        frames = traceback.extract_tb(exc_traceback)

        if self._core.config.cf_token:
          self._core.log.debug("Sending report to Campfire")

          def format_dict(name, dct, include_types=False):
            s = '\n%s: {\n' % name
            for k,v in dct.items():
              s += '  %-28s: %s%s\n' % (k, str(v), (('(%s)' % type(v).__name__) if include_types else ''))
            s += '}\n'
            return s

          body =  'Error                         : %s\n' % exc_type.__name__
          body += 'Message                       : %s\n\n' % str(exc_value)

          body += 'Framework Version             : %s\n' % self._core.version
          body += 'Identifier                    : %s\n' % self._core.identifier
          body += 'Request                       : %s %s\n' % (request.method, request.uri)
          body += 'Handler                       : function "%s" from "%s"\n' % (controller.__name__, controller.__module__)
          if len(kwargs) > 0:
            body += format_dict('Arguments', kwargs)
          if len(request.headers) > 0:
            body += format_dict('Headers', request.headers)

          body += '\nLog:\n  %s' % '\n'.join(log_lines).replace('\n', '\n  ')

          lines = [
            self._core.notifications.error_alert_message(),
            paste(body),
          ]
          self._core.notifications.post_to_campfire(lines, rooms=['err'])

        if self._core.config.ab_api_key:
          self._core.log.debug("Sending report to Airbrake.")
            
          # Build the XML document
          el = self._core.data.xml.element

          notice = el('notice', version='2.2')
          notice.append(el('api-key', self._core.config.ab_api_key))

          environment = el('server-environment')
          #environment.append(el('app-version', self._core.version)) # Currently makes Airbrake explode. Fix before enabling for PMS.
          environment.append(el('environment-name', 'production'))
          notice.append(environment)

          notifier = el('notifier')
          notifier.append(el('name', 'PlexFrameworkNotifier'))
          notifier.append(el('version', '1.0'))
          notifier.append(el('url', 'http://dev.plexapp.com'))
          notice.append(notifier)

          req = el('request')
          req.append(el('url', '%s://%s%s' % (request.protocol, request.host, request.uri)))
          req.append(el('component', self._core.identifier))
          req.append(el('action', controller.__name__ if controller else 'Unknown'))
          notice.append(req)

          params = el('params')
          [params.append(el('var', str(value), key=key)) for key, value in kwargs.items()]
          if len(params) > 0:
            req.append(params)

          session = el('session')
          [session.append(el('var', str(value), key=key)) for key, value in request.headers.items()]
          if len(session) > 0:
            req.append(session)

          error = el('error')
          error.append(el('class', exc_type.__name__))
          error.append(el('message', str(exc_value)))
          notice.append(error)

          backtrace = el('backtrace')
          [backtrace.append(el('line', file=f_file, number=str(f_number))) for f_file, f_number, f_func, f_line in frames]
          error.append(backtrace)

          # POST the data to Airbrake.
          data = self._core.data.xml.to_string(notice)
          req = self._core.networking.http_request(
            url = 'http://airbrakeapp.com/notifier_api/v2/notices',
            headers = {
              'Content-type': 'text/xml',
            },
            data = data,
            immediate = True,
          )
          
          try:
            xml = self._core.data.xml.from_string(req.content)
            self._core.log.info("Report received by Airbrake, ID is %s", xml.xpath('id')[0].text)
          except:
            self._core.log.debug("Unable to get report ID from Airbrake.")

      self.create_thread(_thread, request=request, controller=controller, kwargs=kwargs, e=e)

    except:
      self._core.log_exception("Exception when sending error reports.")

