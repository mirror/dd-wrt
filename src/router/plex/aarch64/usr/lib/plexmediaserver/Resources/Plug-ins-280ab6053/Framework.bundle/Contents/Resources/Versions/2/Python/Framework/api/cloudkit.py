#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import cerealizer

from base import BaseKit


def check_context(func):
  def f(self, *args, **kwargs):
    if self._context.proxy_user_data == False:
      raise Framework.exceptions.ContextException("%s data is not available in this context.", self._class__.__name__)
    return func(self, *args, **kwargs)
  return f



def check_type(obj, root=None, path=None):
  if root == None:
    root = obj
    path = str(obj)

  if isinstance(obj, (bool, basestring, int, float)):
    return True

  elif isinstance(obj, CloudObject):
    for key, value in obj._values.items():
      if check_type(value, root, '%s.%s' % (path, key)) != True:
        return path
    return True

  elif isinstance(obj, list):
    for child in obj:
      if check_type(child, root, '%s[%d]' % (path, obj.index(child))) != True:
        return path
    return True

  elif isinstance(obj, dict):
    for key, value in obj.items():
      if check_type(value, root, '%s.%s' % (path, key)) != True:
        return path
    return True

  return False



class CloudObject(Framework.Serializable):

  def __new__(cls, *args, **kwargs):
    obj = object.__new__(cls, *args, **kwargs)
    obj._values = {}
    return obj


  def __getattr__(self, name):
    if name[0] != '_' and name in self._values:
      return self._values[name]

    return object.__getattribute__(self, name)


  def __setattr__(self, name, value):
    if name[0] != '_':
      self._values[name] = value
    else:
      object.__setattr__(self, name, value)



class UserKit(BaseKit):
  pass



class SessionKit(BaseKit):

  @check_context
  def get(self, key, default=None):
    return self._context.session_data.get(key, default)


  @check_context
  def __getitem__(self, key):
    return self._context.session_data[key]


  @check_context
  def __setitem__(self, key, value):
    ret = check_type(value)
    if ret != True:
      raise TypeError("Objects of type '%s' %scan't be added to session data." % (value.__class__.__name__, ('' if not isinstance(ret, basestring) else ('(%s) ' % ret))))

    self._context.session_data[key] = value


  @check_context
  def __delitem__(self, key):
    del self._context.session_data[key]


  @check_context
  def __len__(self):
    return len(self._context.session_data)


  @check_context
  def __contains__(self, key):
    return


  @check_context
  def clear(self):
    self._core.sandbox.context.create_session_data()


  @check_context
  def setdefault(self, key, default):
    return self._core.sandbox.context.session_data.setdefault(key, default)



class CloudKit(BaseKit):

  _root_object = False
  _included_policies = [
    Framework.policies.CloudPolicy,
    Framework.policies.ElevatedPolicy,
  ]

  _children = [
    UserKit,
    SessionKit,
    CloudObject
  ]
  
