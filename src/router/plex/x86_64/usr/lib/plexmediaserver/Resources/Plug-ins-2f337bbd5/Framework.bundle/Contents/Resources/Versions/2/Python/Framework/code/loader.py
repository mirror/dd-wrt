#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os
import sys
import types
import RestrictedPython
import xpython

class RestrictedModule(types.ModuleType):
  """
    Compiles a module with the sandbox's policy restrictions, storing enough information to allow relative
    imports from this module.
  """
  def __init__(self, name, filename, sandbox, rel_path=None):
    types.ModuleType.__init__(self, name)
    code = sandbox._core.loader.load(filename, sandbox.policy.elevated_execution)
    self.__dict__.update(sandbox.environment)
    self.__dict__['_current_code_path'] = os.path.dirname(filename)
    module_name = os.path.splitext(os.path.basename(filename))[0]
    sys.modules[module_name] = self
    sandbox.modules[module_name] = self
    if rel_path:
      module_name = os.path.splitext(os.path.basename(rel_path))[0] + '.' + module_name
    self.__dict__['__name__'] = module_name
    self.__path__ = os.path.dirname(filename)
    exec(code) in self.__dict__

    del sys.modules[module_name]

class Loader(object):
  def load(self, filename, elevated=False, use_xpython=False):
    name = os.path.basename(filename)
    
    f = open(filename, 'r')
    source = f.read()
    f.close()

    # Convert Extended Python into regular Python
    # See xpython.py in Platforms/Shared/Libraries for details
    if use_xpython:
      source = xpython.convert(source)

    code = self.compile(str(source), str(uni(filename)), elevated)
    return code
    
    
  def compile(self, source, name, elevated=False):
    return RestrictedPython.compile_restricted(source, name, 'exec', elevated=elevated)
    
