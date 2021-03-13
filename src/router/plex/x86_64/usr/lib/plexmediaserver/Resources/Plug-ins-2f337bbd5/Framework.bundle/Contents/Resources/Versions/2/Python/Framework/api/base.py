#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import weakref


class BaseKit(object):
  _root_object = True

  _excluded_policies = []
  _included_policies = []

  _children = []

  
  def __init__(self, sandbox):
    self._sandbox = weakref.proxy(sandbox)
    self._init()


  @property
  def _core(self):
    """ Get the framework core instance from the provided policy instance """
    return self._sandbox._core

    
  @property
  def _context(self):
    return self._sandbox.context

    
  @property
  def _flags(self):
    """ Gets the set of flags from the current policy instance's flag set """
    return self._sandbox.flags


  def _init(self):
    pass

  
  def _publish(self, *args, **kwargs):
    self._sandbox.publish_api(*args, **kwargs)


  @classmethod
  def _exclude_from(cls, *policies):
    def decorator(f):
      def func(self, *args, **kwargs):
        if len(policies) > 0 and True in [self._sandbox.conforms_to_policy(policy) for policy in policies]:
          raise Framework.exceptions.APIException("Accessing %s's '%s' attribute is not permitted by the current policy." % (self.__class__.__name__, f.__name__))
        return f(self, *args, **kwargs)

      func._f_pol = f._f_pol if hasattr(f, '_f_pol') else {}
      func._f_pol.setdefault('exclude', []).extend(policies)
      return func

    return decorator


  @classmethod
  def _include_in(cls, *policies):
    def decorator(f):
      def func(self, *args, **kwargs):
        if len(policies) > 0 and False in [self._sandbox.conforms_to_policy(policy) for policy in policies]:
          raise Framework.exceptions.APIException("Accessing %s's '%s' attribute is not permitted by the current policy." % (self.__class__.__name__, f.__name__))
        return f(self, *args, **kwargs)

      func._f_pol = f._f_pol if hasattr(f, '_f_pol') else {}
      func._f_pol.setdefault('include', []).extend(policies)
      return func

    return decorator
      


