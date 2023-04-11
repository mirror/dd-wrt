import Framework

from base import BaseHandler, ManagementRequestHandler, PrefixRequestHandler


class PluginRequestHandler(ManagementRequestHandler):

  @BaseHandler.route('/prefixes')
  def prefixes(self):
    d = Framework.objects.MediaContainer(self._core)

    for handler in [handler for handler in self._core.runtime._handlers if isinstance(handler, PrefixRequestHandler)]:
      d.Append(Framework.objects.XMLObject(
        self._core,
        tagName="Prefix",
        key=handler.prefix,
        name=handler.name,
        thumb=handler.thumb if handler.thumb else self._core.runtime.external_resource_path(self._core.icon_resource_name),
        art=handler.art if handler.art else self._core.runtime.external_resource_path(self._core.art_resource_name),
        titleBar=handler.titleBar if handler.titleBar else self._core.runtime.external_resource_path(self._core.title_bar_resource_name),
        hasPrefs=self._core.prefs_available(self._core.identifier),
        identifier=self._core.identifier
      )
    )
    
    #TODO: This too.
    for arg in self._core.runtime._interface_args:
      setattr(d, arg, self._core.runtime._interface_args[arg])
    
    return d


  @BaseHandler.route('/prefs')
  def get_prefs(self):
    return self._core.runtime.find_handler(Framework.handlers.RuntimeRequestHandler).get_prefs()


  @BaseHandler.route('/prefs/set')
  def set_prefs(self, **kwargs):
    return self._core.runtime.find_handler(Framework.handlers.RuntimeRequestHandler).set_prefs(**kwargs)


  @BaseHandler.route('/events/systemBundleRestarted')
  def system_bundle_restarted(self):
    self._core.log.debug('System bundle restarted - reloading services')
    self._core.runtime.create_thread(self._core.services.load_all_services)
    return ''
