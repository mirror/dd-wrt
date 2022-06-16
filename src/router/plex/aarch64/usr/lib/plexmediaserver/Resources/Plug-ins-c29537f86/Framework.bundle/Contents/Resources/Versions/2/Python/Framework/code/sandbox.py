#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import binascii
import os
import sys
import types
from RestrictedPython.Guards import safe_builtins

from preferences import PreferenceManager
from context import ExecutionContext
from loader import RestrictedModule


def _apply(f, *args, **kwargs):
  return apply(f, args, kwargs)

def _inplacevar(op, arg1, arg2):
  if op == '+=':
    return arg1 + arg2
  raise Framework.exceptions.FrameworkException("Operator '%s' is not supported" % op)


class PrintHandler:
  """
    A simple class for handling print statements in restricted code.
  """
  def write(self, text):
    if sys.platform != "win32":
      sys.stdout.write(text)

  
def loader(path, sandbox):
  class Loader(object):
    def load_module(self, name):
      lastname = name.split('.')[-1]
      if lastname in sandbox.modules:
        return sandbox.modules[lastname]
      if name not in sandbox.modules:
        sandbox.modules[name] = None
        module = RestrictedModule(name, path, sandbox)
        sandbox.modules[name] = module
        if '.' in name:
          parent_name, child_name = name.rsplit('.', 1)
          setattr(sandbox.modules[parent_name], child_name, module)
      m = sandbox.modules[name]
      return m
  return Loader()
   

class Sandbox(Framework.CoreObject):
  """
    Sandbox manages the loading, compilation and execution of restricted code within a plug-in.
    Multiple sandboxes can be created and used by the framework. Each sandbox is bound to a Framework
    core object and a policy class. The policy dictates the API exposed to the sandboxed code and
    which security measures should be used.
  """
  def __init__(self, core, code_path, policy, flags=[], identifier = None):
    Framework.CoreObject.__init__(self, core)

    # Store the code path and identifier.
    self.code_path = code_path
    self.identifier = identifier if identifier else self._core.identifier

    # Initialise attributes.
    self.whitelist = []
    self.modules = {}
    self.custom_paths = []
    self.custom_headers = {}
    self.resource_paths = [self._core.storage.join_path(self._core.framework_path, 'Resources')]

    # Create a new context for this sandbox.
    self.context = ExecutionContext(self)

    # Create a preference manager.
    self.preferences = PreferenceManager(self)
    
    # Store a reference to the code policy.
    self.policy = policy
    
    # Get the flags from the config module and update them with provided flags
    config_flags = list(core.config.flags)
    for flag in flags:
      if flag[0] == '-':
        flag = flag[1:]
        if flag in config_flags:
          config_flags.remove(flag)
      else:
        if flag not in config_flags:
          config_flags.append(flag)
    self.flags = config_flags
    
    # Set up the default builtins
    standard_builtins = dict(safe_builtins)
    standard_builtins['__import__'] = self.__import__
    
    # Configure the environment
    self.environment = dict(
      _print_             = PrintHandler,
      _getattr_           = __builtins__['getattr'],
      __builtins__        = standard_builtins,
      __name__            = '__code__',
      _write_             = lambda x: x,
      _getiter_           = lambda x: x.__iter__(),
      _getitem_           = lambda x, y: x.__getitem__(y),
      _apply_             = _apply,
      _inplacevar_        = _inplacevar,
      object              = object,
      set                 = set,
      str                 = str,
      unicode             = unicode,
      min                 = min,
      max                 = max,
      xrange              = xrange,
      list                = list,
      dict                = dict,
      staticmethod        = staticmethod,
      classmethod         = classmethod,
      property            = property,
      sorted              = sorted,
      reversed            = reversed,
      reduce              = reduce,
      filter              = filter,
      map                 = map,
      enumerate           = enumerate,
      FrameworkException  = Framework.exceptions.FrameworkException,
      Object              = Framework.Object,
      hexlify             = binascii.hexlify,
      unhexlify           = binascii.unhexlify,
    )

    # Try to add bytearray, but don't try too hard, because this will fail on Python 2.5.
    try:
      base_environment['bytearray'] = bytearray
    except:
      pass

    # Include anything defined by the policy.
    self.environment.update(self.policy.environment)
    
    # Publish the API.
    self.publish_api(Framework.api.DevKit)


  def _check_policy_type(self, excluded_policies=[], included_policies=[]):
    
    policy_in_list = lambda policies: len([policy for policy in policies if self.conforms_to_policy(policy)]) != 0
    
    if len(excluded_policies) > 0 and policy_in_list(excluded_policies) is True:
      return False

    if len(included_policies) > 0 and policy_in_list(included_policies) is False:
      return False

    return True


  def conforms_to_policy(self, *policies):
    return False not in [policy in self.policy.__mro__ for policy in policies]

  def publish_api(self, g, name=None, excluded_policies=[], included_policies=[]):
    
    is_const_group, is_kit_class = (Framework.ConstantGroup in g.__mro__, Framework.api.BaseKit in g.__mro__) if isinstance(g, type) else (False, False)
    
    # If a name wasn't provided, synthesise one.
    if name == None:
      name = g.__name__
      if is_kit_class:
        name = name[:-3]
      elif is_const_group:
        name = name[:-1]

    # Check for API exclusions.
    if name in self._core.api_exclusions:
      return

    # If we were given a method, grab the instance method rather than the class method.
    if isinstance(g, types.MethodType):
      for key, value in self.environment.items():
        if isinstance(value, g.im_class):
          g = getattr(value, g.__name__)
    
    # Copy the restriction lists.
    exclusions = list(excluded_policies)
    inclusions = list(included_policies)

    # Grab policy restrictions from classes and methods.
    if is_kit_class or is_const_group:
      exclusions.extend(g._excluded_policies)
      inclusions.extend(g._included_policies)

    elif isinstance(g, (types.FunctionType, types.MethodType)) and hasattr(g, '_f_pol'):
      exclusions.extend(g._f_pol.get('exclude', []))
      inclusions.extend(g._f_pol.get('include', []))
    
    # Check whether this object is required under the current policy.
    if self._check_policy_type(exclusions, inclusions):
      
      # If this is a kit object, instantiate it and publish children.
      if is_kit_class:
        children = g._children
        g = g(self)

        for cls in children:
          self.publish_api(cls)

      # Check for old-style object classes & create object factories.
      elif isinstance(g, type) and Framework.objects.Object in g.__mro__:
        g = Framework.objects.ObjectFactory(self._core, g)

      # Add root objects to the environment.
      if not is_kit_class or g._root_object == True:
        self.environment[name] = g
        
    
  def format_kwargs_for_function(self, f, kwargs):
    # Check for FunctionDecorator instances
    if isinstance(f, Framework.components.runtime.FunctionDecorator):
      f = f._f
      
    # Get the defaults from the function & grab the names of the variables
    defaults = f.func_defaults
    if not defaults: return
    varnames = f.func_code.co_varnames
    
    while len(varnames) > 0 and varnames[-1][0] == '_':
      varnames = varnames[:-1]
    varnames = varnames[-len(defaults):]
    
    # Iterate through the variables
    for i in range(len(varnames)):
      
      # Get the name
      n = varnames[i]
      
      # If the name is in the provided kwargs...
      if n in kwargs:
        
        # Get the type of the default
        t = type(defaults[i])
        
        # Convert the provided kwarg to the correct type if necessary
        v = kwargs[n]
        if t == bool:
          kwargs[n] = (v == True or str(v).lower() == 'true' or str(v) == '1')
        elif t == int:
          kwargs[n] = int(v)
        elif t == float:
          kwargs[n] == float(v)

      
  def execute(self, code):
    exec(code) in self.environment
    

  def function_is_deferred(self, function_name):
    return isinstance(self.environment.get(function_name), Framework.components.runtime.DeferredFunction)
    

  def call_named_function(self, function_name, allow_deferred=False, raise_exceptions=False, args=None, kwargs=None, mod_name=None, optional_kwargs=None):
    """
      Try to call a function in the sandboxed environment with the given args & kwargs.
    """
    try:
      # Set defaults
      if args == None: args = []
      if kwargs == None: kwargs = {}
      if optional_kwargs == None: optional_kwargs = {}

      # Try to find the function
      f = None
      if function_name in self.environment:
        f = self.environment[function_name]

      elif mod_name and mod_name in self.environment:
        module_dict = self.environment[mod_name].__dict__
        if function_name in module_dict:
          f = module_dict[function_name]

      if f:
        # If this is a deferred function, check whether we should allow it
        if isinstance(f, Framework.components.runtime.DeferredFunction) and not allow_deferred:
          return None

        # If the function accpets optional args, copy them to the kwargs dict.
        for arg_name in optional_kwargs:
          if Framework.utils.function_accepts_arg(f, arg_name):
            kwargs[arg_name] = optional_kwargs[arg_name]
          
        self.format_kwargs_for_function(f, kwargs)
        result = f(*args, **kwargs)
        return result

      else:
        self._core.log.critical("Function named '%s' couldn't be found in the current environment", function_name)

    except Framework.exceptions.UnauthorizedException:
      raise
    except:
      self._core.log_exception("Exception when calling function '%s'", function_name)
      if raise_exceptions:
        raise
      

  def __import__(self, _name, _globals={}, _locals={}, _fromlist=[], _level=-1):
    """
      Calls the standard Python import mechanism. We use the meta importer
      to apply restrictions to plug-in code (in Contents/Code)
    """

    path = _locals.get('__path__') if _locals else None
    lastname = _name.rsplit('.', 1)[-1]
    possible_paths = []
    
    def add_possible_path(possible_path):
      possible_paths.append(os.path.join(possible_path, lastname + '.' + self.policy.ext))
      possible_paths.append(os.path.join(possible_path, lastname, '__init__' + '.' + self.policy.ext))
    
    root_path = path if isinstance(path, basestring) else self.code_path
    
    if root_path:
      add_possible_path(root_path)
      
    for custom_path in self.custom_paths:
      add_possible_path(custom_path)
    
    for pyp in possible_paths:
      if os.path.exists(pyp):
        mod = loader(pyp, self)
        return mod.load_module(_name)

    m = None
    if _name in sys.modules:
      m = sys.modules[_name]
    else:
      try:
        m = __import__(_name, _globals, _locals, _fromlist, _level)
      except Exception, e:
        try:
          m = __import__('_'+_name, _globals, _locals, _fromlist, _level)
        except:
          raise e
    
    # If we're blocking imports, only allow RestrictedModules to be returned
    if self.policy.block_imports and not isinstance(m, RestrictedModule) and _name not in self.whitelist:
      raise ImportError("Importing '%s' is disallowed under the current code policy." % _name)

    return m
