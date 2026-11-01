#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService
import re


UPDATE_FEED_KEY = 'PlexPluginUpdateFeed'

class BundleInfo(object):
  def __init__(self, system, plugin_path, name):
    self.system = system
    self.path = plugin_path
    self.name = name
    self.bundled = Core.bundled_plugins_path in plugin_path if Core.bundled_plugins_path is not None else False
    self.load_plist()
    self.update_version()
    
  def load_plist(self):
    plist = Plist.ObjectFromString(Core.storage.load(Core.storage.join_path(self.path, "Contents", "Info.plist")))
    self.service_dict = Core.services.get_services_from_bundle(self.path, plist)
    self.identifier = plist['CFBundleIdentifier']
    self.plugin_class = plist.get('PlexPluginClass', 'Channel')
    
    # Read the update feed from the plist, if present
    if UPDATE_FEED_KEY in plist:
      url = plist[UPDATE_FEED_KEY]
      if url[-1] != '/':
        url += '/'
      self.update_feed = url
    else:
      self.update_feed = None
    
    self.ignore = 'PlexPluginDevMode' in plist and plist['PlexPluginDevMode'] == '1'
    if Core.storage.link_exists(self.path):
      Log("Plug-in bundle with identifier '%s' is a symbolic link, and will be ignored.", self.identifier)
      self.ignore = True
      
    # If we found an agent bundle, check that the agent info exists.
    if self.plugin_class == 'Agent':
      self.system.agentservice.ensure_agent_info_exists(self.identifier)
    
  
  @property
  def has_services(self):
    for key in ('Services', 'ServiceSets', 'OldServices'):
      for service_type in self.service_dict[self.identifier][key]:
        if len(self.service_dict[self.identifier][key][service_type]) > 0:
          return True
    return False

  def update_version(self):
    version_path = Core.storage.join_path(self.path, 'Contents', 'VERSION')
    if Core.storage.file_exists(version_path):
      self.git_sha = Core.storage.load(version_path)[:7]
    else:
      self.git_sha = None


class BundleService(SystemService):
  def __init__(self, system):
    SystemService.__init__(self, system)
    Log.Debug("Starting the bundle service")
    
    self.plugins_path = Core.storage.join_path(Core.app_support_path, 'Plug-ins')
    self.bundle_dict = dict()
    self.bundled_identifiers = []
    self.update_lock = Thread.Lock()
    self.service_started = Thread.Event()
    self.first_scan = True
    
    Core.messaging.expose_function(self.all_services, '_BundleService:AllServices')
    
    Thread.Create(self.start)
    
  def start(self):
    self.update_bundles()
    self.service_started.set()
    Log.Debug("Started the bundle service")
    
  @property
  def bundles(self):
    self.service_started.wait()
    return self.bundle_dict
    
  def all_services(self):
    service_dict = {}
    bundles = self.bundles
    for identifier in bundles:
      bundle = bundles[identifier]
      if bundle.has_services:
        service_dict.update(bundle.service_dict)
    return service_dict
    
  def update_bundles(self, remove_unavailable_agents=True):
    try:
      self.update_lock.acquire()
      if Core.bundled_plugins_path is not None:
        bundled_plugin_paths = {d : Core.storage.join_path(Core.bundled_plugins_path, d) for d in Core.storage.list_dir(Core.bundled_plugins_path) if d.endswith('.bundle')}
      else:
        bundled_plugin_paths = {}
      Core.storage.ensure_dirs(self.plugins_path)
      plugin_paths = {d : Core.storage.join_path(self.plugins_path, d) for d in Core.storage.list_dir(self.plugins_path) if d.endswith('.bundle')}
      identifiers = self.bundle_dict.keys()

      # Unregister any bundles that have gone missing.
      for identifier in identifiers:
        name = self.bundle_dict[identifier].name
        if name not in bundled_plugin_paths.keys() + plugin_paths.keys():
          del self.bundle_dict[identifier]
        else:
          # We know about it, remove from maps.
          if name in bundled_plugin_paths:
            del bundled_plugin_paths[name]
          if name in plugin_paths:
            del plugin_paths[name]

      # Nuke any non-bundled plugins with duplicate identifiers, preferring the ones in the bundled location.
      # Bundled plugins may have duplicate identifiers; PMS will decide which to load (based on feature flags, e.g.).
      #
      identifiers = []
      for plugin, path in bundled_plugin_paths.items() + plugin_paths.items():
        try:
          bundle = BundleInfo(self.system, path, plugin)
          if bundle.identifier in identifiers:
            if Core.storage.link_exists(path):
              Log('Found symbolic link at %s, ignoring duplicate plugin.' % path)
            elif Core.storage.dir_name(path) == Core.bundled_plugins_path:
              Log('Found duplicate identifier at %s for bundled plugin (%s), ignoring duplicate plugin.' % (path, bundle.identifier))
            else:
              Log('Deleting plugin at %s because it has a duplicate identifier (%s)' % (path, bundle.identifier))
              Core.storage.remove_tree(path)
          else:
            identifiers.append(bundle.identifier)
            
            # Update map.
            self.bundle_dict[bundle.identifier] = bundle
        except:
          Log.Exception('Exception adding bundle: %s' % plugin)

      # Update the list of bundled plugin identifiers.
      self.bundled_identifiers = [self.bundle_dict[bundle].identifier for bundle in self.bundle_dict if self.bundle_dict[bundle].bundled]

      if remove_unavailable_agents:
        Thread.Create(self.system.agentservice.remove_unavailable_agents)
      
    finally:
      self.first_scan = False
      self.update_lock.release()

