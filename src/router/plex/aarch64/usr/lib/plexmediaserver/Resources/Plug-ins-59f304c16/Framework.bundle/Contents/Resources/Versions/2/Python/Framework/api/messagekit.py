#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BaseKit

##################################################################################################

"""
  External function API:
  
  These classes provide the syntax for calling external functions. The first class used is the
  ExternalObjectBroker class. A global instance of this class called External is made available
  by MessageKit. Accessing items of this class generates FunctionBroker objects, which provide
  ExternalFunction objects when accessing their attributes. These objects can then be called,
  which generates a call to the external function in the messaging component and returns its
  response.
  
  Usage:
    
    result = External['com.plexapp.plugins.example'].TestFunction('abc', x=1, y=2)
    
    or
    
    my_plugin = External['com.plexapp.plugins.example']
    result = my_plugin.TestFunction('abc', x=1, y=2)
    
"""

class ExternalFunction(Framework.CoreObject):
  def __init__(self, core, identifier, name):
    Framework.CoreObject.__init__(self, core)
    self._identifier = identifier
    self._name = name
    
  def __call__(self, *args, **kwargs):
    return self._core.messaging.call_external_function(self._identifier, "MessageKit:"+self._name, args, kwargs)

class FunctionBroker(Framework.CoreObject):
  def __init__(self, core, identifier):
    Framework.CoreObject.__init__(self, core)
    self._identifier = identifier
    
  def __getattr__(self, name):
    return ExternalFunction(self._core, self._identifier, name)

class ExternalPluginBroker(Framework.CoreObject):
  def __getitem__(self, identifier):
    return FunctionBroker(self._core, identifier)
    
  def Wait(self, identifier, timeout=None):
    if timeout == 0:
      timeout = None
    if ':' in identifier:
      raise Framework.exceptions.FrameworkException("The ':' character cannot be used in plug-in identifiers.")
    return self._core.messaging.wait_for_presence(identifier, timeout)

##################################################################################################

"""
  Notification generation API:
  
  These classes provide the syntax for generating notifications. A global NotificationBroker object
  named Notify is created by MessageKit. Accessing attributes of this object generates Notification
  objects, which generates a notification in the messaging component when called.
  
  Usage:
    
    Notify.MyNotification('abc', x=1, y=2)

"""

class Notification(Framework.CoreObject):
  def __init__(self, core, name):
    Framework.CoreObject.__init__(self, core)
    self._name = name
    
  def __call__(self, *args, **kwargs):
    self._core.messaging.send_notification(self._name, args, kwargs)

class NotificationBroker(Framework.CoreObject):
  def __getattr__(self, name):
    return Notification(self._core, name)

##################################################################################################

class Trigger(Framework.CoreObject):
  def __init__(self, broker, name):
    Framework.CoreObject.__init__(self, broker._core)
    self._broker = broker
    self._name = name
    
  def __call__(self, *args, **kwargs):
    def decorator(f):
      return self._broker._f(f, self._name, args, kwargs)
    return decorator
    
class TriggerBroker(Framework.CoreObject):
  def __init__(self, core, f):
    Framework.CoreObject.__init__(self, core)
    self._f = f
  
  def __getattr__(self, name):
    return Trigger(self, name)
  
class TriggerManager(Framework.CoreObject):
  def _init(self):
    self.onCall       = TriggerBroker(self._core, self._onCall)
    self.onComplete   = TriggerBroker(self._core, self._onComplete)
    self.onResponse   = TriggerBroker(self._core, self._onResponse)
    self.onSuccess    = TriggerBroker(self._core, self._onSuccess)
    self.onFailure    = TriggerBroker(self._core, self._onFailure)
    
  def _onCall(self, f, name, args, kwargs):
    def func(*f_args, **f_kwargs):
      self._core.messaging.send_notification(name, args, kwargs)
      result = f(*f_args, **f_kwargs)
      return result
    return func
    
  def _onComplete(self, f, name, args, kwargs):
    def func(*f_args, **f_kwargs):
      result = f(*f_args, **f_kwargs)
      self._core.messaging.send_notification(name, args, kwargs)
      return result
    return func

  def _onResponse(self, f, name, args, kwargs):
    def func(*f_args, **f_kwargs):
      result = f(*f_args, **f_kwargs)
      if result != None:
        self._core.messaging.send_notification(name, args, kwargs)
      return result
    return func

  def _onSuccess(self, f, name, args, kwargs):
    def func(*f_args, **f_kwargs):
      result = f(*f_args, **f_kwargs)
      if result == True:
        self._core.messaging.send_notification(name, args, kwargs)
      return result
    return func

  def _onFailure(self, f, name, args, kwargs):
    def func(*f_args, **f_kwargs):
      result = f(*f_args, **f_kwargs)
      if result in [None, False]:
        self._core.messaging.send_notification(name, args, kwargs)
      return result
    return func
    
##################################################################################################
    
"""
  Event subscription API:
  
  These classes provide the syntax for receiving framework notification events (the general event
  system is not exposed by MessageKit). Two decorators are made available - @source and @event.
  These are instances of SourceBroker and EventBroker respectively. When used as a pair, they
  register a function as a responder for a framework event of the given name from the given source.
  
  Usage:
  
    @source('com.plexapp.plugins.example')
    @event.MyNotification
    def MyNotificationFired(s, x, y):
      print s
      print x + y
      
"""

class Event(object):
  def __init__(self, name):
    self._name = name

  def __call__(self, f):
    self._f = f
    return self
    
class EventBroker(object):
  def __getattr__(self, name):
    return Event(name)
    
class Source(Framework.CoreObject):
  def __init__(self, core, identifier):
    Framework.CoreObject.__init__(self, core)
    self._identifier = identifier
    
  def __call__(self, event):
    self._core.messaging.register_for_notification(event._f, self._identifier, event._name)
    return event._f
    
class SourceBroker(Framework.CoreObject):
  def __call__(self, identifier):
    return Source(self._core, identifier)
    
    
##################################################################################################

class MessageKit(BaseKit):
  """
    The MessageKit class exposes global instances of ExternalPluginBroker, EventBroker,
    SourceBroker and NotificationBroker. It also provides an @expose decorator that registers
    a function as remotely callable with the dispatch node in the messaging component.
  """  
  _root_object = False

  _included_policies = [
    Framework.policies.BundlePolicy,
  ]

  _excluded_policies = [
    Framework.policies.ModernPolicy,
  ]

  
  def _init(self):
    self._publish(self._expose_decorator, name='expose')
    self._publish(ExternalPluginBroker(self._core), name='External')
    self._publish(EventBroker(), name='event')
    self._publish(SourceBroker(self._core), name='source')
    self._publish(NotificationBroker(self._core), name='Notify')
    self._publish(TriggerManager(self._core), name='trigger')
    
    
  def _expose_decorator(self, f):
    self._core.messaging.expose_function(f, "MessageKit:"+f.__name__)
    return f
  