import Framework
import types


class SpecialMethodType(object):
  before = 'before'
  after = 'after'

class BaseHandler(Framework.CoreObject):

  _internal_names = ('routes', 'before_all_functions', 'after_all_functions')

  @staticmethod
  def route(path, methods=['GET'], **kwargs):
    def decorator(f):
      if not hasattr(f, 'f_routes'):
        f.f_routes = []
      f.f_routes.append((path, methods, kwargs))
      return f
    return decorator


  @staticmethod
  def before_all(f):
    f.f_special = SpecialMethodType.before
    return f


  @staticmethod
  def after_all(f):
    f.f_special = SpecialMethodType.after
    return f


  @property
  def routes(self):
    routes = []
    for name in dir(self):
      if name not in BaseHandler._internal_names:
        obj = getattr(self, name)
        if isinstance(obj, types.MethodType) and hasattr(obj, 'f_routes'):
          routes.append(obj)
    return routes


  def _special_methods(self, method_type):
    return [obj for obj in [getattr(self, name) for name in dir(self) if name not in BaseHandler._internal_names] if isinstance(obj, types.MethodType) and hasattr(obj, 'f_special') and obj.f_special == method_type]


  @property
  def before_all_functions(self):
    return self._special_methods(SpecialMethodType.before)
    

  @property
  def after_all_functions(self):
    return self._special_methods(SpecialMethodType.after)


  @classmethod
  def _route_group(cls):
    return None



class PrefixRequestHandler(BaseHandler):
  pass



class InternalRequestHandler(BaseHandler):
  @classmethod
  def _route_group(cls):
    return 'internal'



class ManagementRequestHandler(BaseHandler):
  @classmethod
  def _route_group(cls):
    return 'management'



def generate_prefix_handler(route, func):
  class PrefixHandler(PrefixRequestHandler):

    def __init__(self, core, name, thumb, art, titleBar, share):
      PrefixRequestHandler.__init__(self, core)
      self.func = func
      self.name = name
      self.thumb = thumb
      self.art = art
      self.titleBar = titleBar
      self.share = share
      self.prefix = route
      
      
    @BaseHandler.route(route)
    def call(self, *args, **kwargs):
      # Check the minimum server version requirement.
      if self._core.minimum_server_version != None and not self._core.server_version_at_least(*self._core.minimum_server_version):
        version_string = '.'.join([str(x) for x in self._core.minimum_server_version])
        self._core.log.error("Current server version %s doesn't satisfy minimum requirement %s.", self._core.get_server_attribute('serverVersion'), version_string)
        return self._core.sandbox.environment['ObjectContainer'](
          header = self._core.localization.localize("Update required"),
          message = self._core.localization.localize("This channel requires Plex Media Server version %s or greater.") % version_string,
        )

      # Call the function.
      result = self.func(*args, **kwargs)

      # If the response is a non-System ObjectContainer...
      if isinstance(result, Framework.api.objectkit.ObjectContainer) and self._core.config.daemonized == False and self._core.identifier != 'com.plexapp.system':

        # Check whether the client platform is unsupported.
        platform = self._core.sandbox.context.platform
        if platform is not None and platform in self._core.client_platform_exclusions:
          self._core.log.info("Channel has flagged '%s' as an unsupported platform.")
          result.header = "This app isn't supported"
          result.message = "You can still access %s, but you may encounter unexpected problems." % self.name
      
      return result
        

  return PrefixHandler
