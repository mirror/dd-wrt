#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import weakref
import cerealizer


class Object(object):

  pass



class CoreObject(object):

  def __init__(self, core):
    self._core = weakref.proxy(core)
    self._init()

  
  def _init(self):
    pass



class AttributeLocker(type):

  def __setattr__(cls, name, value):
    if cls._locked:
      raise Framework.exceptions.FrameworkException("The '%s' attribute of '%s' can't be modified." % (name, cls.__name__))
    super(AttributeLocker, cls).__setattr__(name, value)



class ConstantGroup(object):

  __metaclass__ = AttributeLocker
  _locked = False

  _excluded_policies = []
  _included_policies = []
  

  @classmethod
  def lock(cls):
    cls._locked = True



class SerializableMetaclass(type):

  # Intercept creation of new subclasses and register them with cerealizer.
  def __new__(cls, name, bases, dct):
    cls = super(SerializableMetaclass, cls).__new__(cls, name, bases, dct)
    cerealizer.register(cls)
    return cls
  


class Serializable(object):

  __metaclass__ = SerializableMetaclass
