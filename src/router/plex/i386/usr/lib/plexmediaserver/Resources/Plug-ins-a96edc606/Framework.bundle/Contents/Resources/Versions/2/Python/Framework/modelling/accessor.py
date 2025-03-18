#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os
import templates, model, classes


class ModelAccessPoint(object):
  def __init__(self, accessor, identifier, read_only=False):
    self._accessor = accessor
    self._identifier = identifier
    self._read_only = read_only
    self._classes = dict()
    for name in accessor._templates:
      self._classes[name] = classes.generate_class(self, accessor._namespace_name, accessor._templates[name], accessor._storage_path)

  def __getattribute__(self, name):
    if name[0] == '_': return object.__getattribute__(self, name)
    classes = self._classes
    if name in classes:
      return classes[name]
    
    return object.__getattribute__(self, name)
    
  def __getitem__(self, identifier):
    return self._accessor.get_access_point(self._core.runtime._expand_identifier(identifier), read_only=True)
    
  @property
  def _core(self):
    return self._accessor._core


class ModelAccessor(object):

  def __init__(self, core, namespace_name, template_file, storage_path):
    self._templates = dict()
    self._access_points = dict()
    self._read_only_access_points = dict()
    self._namespace_name = namespace_name
    self._storage_path = storage_path
    self._core = core
    
    template_sandbox = Framework.code.Sandbox(self._core, os.path.dirname(template_file), Framework.policies.ModelPolicy)
    template_code = self._core.loader.load(template_file)
    
    template_sandbox.execute(template_code)
    
    for name in template_sandbox.environment:
      obj = template_sandbox.environment[name]
      if hasattr(obj, '__mro__') and (templates.ModelTemplate in obj.__mro__ or templates.RecordTemplate in obj.__mro__) and hasattr(obj, '__bases__') and templates.AbstractTemplate not in obj.__bases__:
        self._templates[name] = obj
        
    del template_sandbox
    del template_code
    
  def get_access_point(self, identifier, read_only=False):
    if not read_only:
      access_points = self._access_points
      ro_str = ''
    else:
      access_points = self._read_only_access_points
      ro_str = 'read-only '
      
    if identifier not in access_points:
      self._core.log.debug("Creating a new %smodel access point for provider %s in namespace '%s'", ro_str, identifier, self._namespace_name, )
      access_points[identifier] = ModelAccessPoint(self, identifier, read_only)
    return access_points[identifier]
    
    