#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService
import re

class FlagService(SystemService):

  def __init__(self, system):
    SystemService.__init__(self, system)
    Log.Debug("Starting the media flag service")
    
    self.patterns = {}
    self.pattern_update_time = None
    self.identifier = "com.plexapp.resources.mediaflags"
    
    Thread.Create(self.start)
    
  def start(self):
    # Running locally
    if not Core.config.daemonized:
      if self.identifier not in self.system.bundleservice.bundles:
        Log.Error("Unable to find the media flag bundle (%s)" % self.identifier)
        return
        
      bundle = self.system.bundleservice.bundles[self.identifier]
      self.bundle_path = bundle.path

    # Running in the cloud
    else:
      self.bundle_path = Core.storage.join_path(Core.app_support_path, Core.config.bundles_dir_name, self.identifier)
    
    self.add_resource_type('Aspect Ratio')
    self.add_resource_type('Audio Channels')
    self.add_resource_type('Audio Codec')
    self.add_resource_type('Media')
    self.add_resource_type('Studio')
    self.add_resource_type('Content Rating')
    self.add_resource_type('Record Label')
    self.add_resource_type('Video Codec')
    self.add_resource_type('Video Frame Rate')
    self.add_resource_type('Video Resolution')
    
    Log.Debug("Started the media flag service")
    
    
  def add_resource_type(self, resource_type):
    path_components = resource_type.split(' ')
    path_components[0] = path_components[0].lower()
    Route.Connect('/system/bundle/media/flags/%s/{name}' % ''.join(path_components), self.resource, resource_type=resource_type)
    Route.Connect('/system/bundle/media/flags/%s/{lang}/{name}' % ''.join(path_components), self.resource_with_subdir, resource_type=resource_type)

  def resource_path(self, resource_type, subdir, name):
    return Core.storage.join_path(self.bundle_path, 'Contents', 'Resources', resource_type, subdir, name + '.png')
  
  def resource_exists(self, resource_type, subdir, name):
    return Core.storage.file_exists(self.resource_path(resource_type, subdir, name))
    
  def resource_substitution_path(self, resource_type, name):
    
    # Reload patterns if needed.
    xml_path = Core.storage.join_path(self.bundle_path, 'Contents', 'Resources', 'substitutions.xml')
    pattern_file_time = Core.storage.last_modified(xml_path)
    node_name = resource_type.replace(' ', '')
    
    if self.pattern_update_time is None or self.pattern_update_time < pattern_file_time:
      self.pattern_update_time = pattern_file_time
      
      # Load the file.
      xml_str = Core.storage.load(xml_path)
      xml = Core.data.xml.from_string(xml_str)
      
      for child in xml:
        self.patterns[child.tag] = []
        for node in child.xpath('match'):
          self.patterns[child.tag].append((node.get('name'), re.compile(node.get('expression'), re.IGNORECASE)))
        
    # Check the patterns.
    if self.patterns.has_key(node_name):
      for (resource,pattern) in self.patterns[node_name]:
        if re.search(pattern, name):
          return self.resource_path(resource_type, '', resource)
          
    # Last, but not least, return a blank image so it'll stop asking.
    return self.resource_path('', '', 'blank')
  
  def resource_with_subdir(self, resource_type, lang, name, t=None):
    if self.resource_exists(resource_type, lang, name):
      path = self.resource_path(resource_type, lang, name)
    else:
      path = self.resource_substitution_path(resource_type, name)
    if path:
      data = Core.storage.load(path)
      return DataObject(data, 'image/png')
  
  def resource(self, resource_type, name, t=None):
    return self.resource_with_subdir(resource_type, '', name, t)