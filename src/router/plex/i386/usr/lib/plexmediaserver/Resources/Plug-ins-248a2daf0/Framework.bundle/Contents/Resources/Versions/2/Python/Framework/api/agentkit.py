#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os, weakref, urlparse

from base import BaseKit

class FakeMediaObject(unicode):
  """
    A faked-out object to use when no media object is available. Agent code should continue
    to run unchanged, but everything that attempts to access the unavailable parts of the
    media object should be a no-op.
  """

  
  def __getattr__(self, name):
    return self
  

  def __setattr__(self, name, value):
    pass
  

  def __getitem__(self, name):
    return self
  

  def __setitem__(self, name, value):
    pass
  

  def __delitem__(self, name):
    pass
  

  def __contains__(self, name):
    return False
  

  def __call__(self, *args, **kwargs):
    pass
  

  def __iter__(self):
    return [].__iter__()
  

  def validate_keys(self, names):
    pass
  

  def keys(self):
    return []
  

  def all_parts(self):
    return []
    
    

class MediaContentsDirectory(object):

  def __init__(self, core, path, readonly=True):
    self._core = core
    self._path = path
    self._readonly = readonly
    
    if not self._core.storage.dir_exists(self._path):
      self._core.storage.make_dirs(self._path)
      

  def _item_path(self, item):
    if '/' in item or item == '..':
      raise KeyError('Invalid item name: %s' % str(item))
    return self._core.storage.join_path(self._path, item)
    

  def __contains__(self, item):
    return self._core.storage.file_exists(self._item_path(item))
    

  def __getitem__(self, item):
    path = self._item_path(item)
    if self._core.storage.file_exists(path):
      return self._core.storage.load(path)
    raise KeyError("The file named '%s' does not exist" % str(item))
    

  def __setitem__(self, item, data):
    if self._readonly:
      raise Framework.exceptions.FrameworkException("The directory '%s' is read-only" % self._path)
    path = self._item_path(item)
    self._core.storage.save(path, data)
    

    
class MediaProxyContentsDirectory(MediaContentsDirectory):

  def __init__(self, core, path, readonly=True, el=None, parent=None):
    MediaContentsDirectory.__init__(self, core, path, readonly)
    self._proxies = dict()
    self._parent = weakref.proxy(parent)
    
    if el == None:
      return
      
    # If we were given an XML element, parse it
    for child in el:
      attrs = dict(child.attrib)
      if 'file' in attrs:
        p_type = 'LocalFile'
        p_value = attrs['file']
      elif 'media' in attrs:
        p_type = 'Media'
        # Store the extension in p_value
        p_value = str(attrs['media']).rsplit('.')[-1]
      else:
        continue
      p_name = attrs['name']

      p_sort = attrs.get('sort_order')
      p_index = attrs.get('index')  
      p_codec = attrs.get('codec')
      p_format = attrs.get('format')
      p_default = attrs.get('default')
      p_forced = attrs.get('forced')
      
      # Create a tuple of the values
      tup = (p_type, p_value, p_sort, p_index, p_codec, p_format, p_default, p_forced)
      
      # If there isn't an item with this name in the dict already, add it
      if p_name not in self._proxies:
        self._proxies[p_name] = tup
        
      # Otherwise, convert the existing item + the current item into a list
      else:
        if isinstance(self._proxies[p_name], list):
          self._proxies[p_name].append(tup)
        else:
          self._proxies[p_name] = [self._proxies[p_name], tup]
  
  
  def _element(self, tag='Directory', item_tag='Item'):

    def item_el_for_tuple(tup):
      p_type, p_value, p_sort, p_index, p_codec, p_format, p_default, p_forced = tup
      
      item_el = self._core.data.xml.element(item_tag)
      item_el.set('name', item)
      if p_sort:
        item_el.set('sort_order', p_sort)
      
      if p_type == 'LocalFile':
        item_el.set('file', p_value)
      
      elif p_type == 'Media':
        name_hash = self._core.data.hashing.sha1(item)
        if p_value:
          name_hash += '.' + p_value
        item_el.set('media', name_hash)
        
      if p_index:
        item_el.set('index', p_index)
        
      if p_codec:
        item_el.set('codec', p_codec)

      if p_format:
        item_el.set('format', p_format)

      if p_default:
        item_el.set('default', p_default)

      if p_forced:
        item_el.set('forced', p_forced)
        
      return item_el
        
    el = self._core.data.xml.element(tag)
    
    for item in self._proxies:
      # If this is a tuple, append an element for it
      if isinstance(self._proxies[item], tuple):
        el.append(item_el_for_tuple(self._proxies[item]))
      
      # If this is a list, append an element for each tuple in the list
      elif isinstance(self._proxies[item], list):
        for list_item in self._proxies[item]:
          el.append(item_el_for_tuple(list_item))
        
    return el
    
        
  def _save(self):
    if self._parent and hasattr(self._parent, '_save'):
      self._parent._save()
        
        
  def __setitem__(self, item, value):
    
    def make_tuple(item, value):
      p_type = value._proxy_name
      p_sort = value._sort_order
      p_index = value._index
      p_codec = value._codec
      p_format = value._format
      p_default = value._default
      p_forced = value._forced
      
      # Set the codec to the file extension if one isn't provided
      if p_codec == None:
        # Check for a manually specified extension first
        if value._ext:
          p_codec = value._ext
          
        # If there isn't one, try to extract it from the item name
        else:
          pos = item.rfind('.')
          if pos > -1:
            p_codec = item[pos+1:]
        
      
      if p_type == 'LocalFile':
        p_value = value._data
        
      elif p_type == 'Media':
        name_hash = self._core.data.hashing.sha1(item)
        for ext in value._extras:
          extra_name = name_hash + '.' + ext
          MediaContentsDirectory.__setitem__(self, extra_name, value._extras[ext])
        if value._ext != None:
          name_hash += '.' + value._ext
        MediaContentsDirectory.__setitem__(self, name_hash, value._data)
        p_value = value._ext
        
      else:
        raise ValueError("Invalid proxy type '%s'" % value._proxy_name)
      
      return (p_type, p_value, p_sort, p_index, p_codec, p_format, p_default, p_forced)
    
    if isinstance(value, Framework.modelling.attributes.ProxyObject):
      
      # Add the proxy to the dict...
      self._proxies[item] = make_tuple(item, value)
      
      # Tell the parent object to save itself
      self._save()
    
    elif isinstance(value, list):
      tuple_list = []
      
      # Convert a list of proxies into a list of tuples
      for list_item in value:
        if isinstance(list_item, Framework.modelling.attributes.ProxyObject):
          tuple_list.append(make_tuple(item, list_item))
          
      self._proxies[item] = tuple_list
      
      # Tell the parent object to save itself
      self._save()
      
    else:
      raise ValueError("Can't set object of type '%s'" % str(type(value)))
      
        
  def __getitem__(self, item):
    
    # Helper function for retrieving data based on the contents of a tuple
    def item_from_tuple(tup):
      p_type, p_value, p_sort, p_index, p_codec, p_format, p_default, p_forced = tup
      
      # We're accessing a local file proxy - load and return the contents of the file
      if p_type == 'LocalFile':
        return self._core.storage.load(p_value)
        
      # We're accessing a piece of media stored in the directory - call the superclass getitem method with the hash of the item name
      elif p_type == 'Media':
        name_hash = self._core.data.hashing.sha1(item)
        if p_value:
          name_hash += '.' + p_value
        return MediaContentsDirectory.__getitem__(self, name_hash)
    
    # Check that the item exists
    if item in self._proxies:
      
      # If the item is a single tuple, return it
      if isinstance(self._proxies[item], tuple):
        return item_from_tuple(self._proxies[item])
        
      # If we have a list, return a list of data items
      elif isinstance(self._proxies[item], list):
        data_list = []
        for tup in self._proxies[item]:
          data_list.append(item_from_tuple(tup))
        return data_list
    
    raise KeyError("No item found with name '%s'" % item)
    
    
  def __contains__(self, item):
    return item in self._proxies

    
  def __iter__(self):
    return self._proxies.__iter__()
    
    
  def _del(self, item):  
    def delete_item_for_tuple(tup):
      p_type, p_value, p_sort, p_index, p_codec, p_format, p_default, p_forced = tup
      
      # If it's a media proxy, remove the file on disk
      if p_type == 'Media':
        # Calculate the SHA1 hash of the name
        name_hash = self._core.data.hashing.sha1(item)
        if p_value:
          name_hash += '.' + p_value
      
        self._core.storage.remove(self._core.storage.join_path(self._path, name_hash))
      
    # Check that the item exists in the proxy dict
    if item in self._proxies:
      # If it's a tuple, remove it
      if isinstance(self._proxies[item], tuple):
        delete_item_for_tuple(self._proxies[item])
        
      # If it's a list, remove each item in the list
      if isinstance(self._proxies[item], list):
        for tup in self._proxies[item]:
          delete_item_for_tuple(tup)
      
      # Remove the proxy
      del self._proxies[item]
      
    else:
      raise KeyError("No item found with name '%s'" % item)
  
  
  def __delitem__(self, item):
    self._del(item)
    self._save()  
    
    
  def validate_keys(self, valid_keys):
    # Check each proxy key to ensure it's in the list of valid keys, and remove it if not
    for key in self._proxies.keys():
      if key not in valid_keys:
        self._del(key)
    
    # Tell the parent object to save itself
    self._save()
        
  
    
class SubtitlesDirectory(object):
  
  def __init__(self, core, path):
    self._core = core
    self._path = path
    self._lang_dirs = dict()
    
    self._core.storage.ensure_dirs(self._path)
    self._load()
    
    
  def _load(self):
    # Check that the XML file exists
    path = self._path + '.xml'
    if not self._core.storage.file_exists(path):
      return
      
    data = self._core.storage.load(path)
    el = self._core.data.xml.from_string(data)
    
    # For each language in the file, create a contents directory object and allow it to parse the element
    for lang_el in el.xpath('//Language'):
      lang = lang_el.get('code')
      lang_path = self._core.storage.join_path(self._path, lang)
      proxy_dir = MediaProxyContentsDirectory(self._core, lang_path, readonly=False, el=lang_el, parent=self)
      self._lang_dirs[lang] = proxy_dir
      
    
  def __getitem__(self, lang):
    # Check that the language code is valid
    if not self._core.localization.language_code_valid(lang):
      raise KeyError("The language code '%s' is invalid")
      
    # If we don't have the language dir loaded, create a new object
    if lang not in self._lang_dirs:
      path = self._core.storage.join_path(self._path, lang)
      self._lang_dirs[lang] = MediaProxyContentsDirectory(self._core, path, readonly=False, parent=self)
      
    return self._lang_dirs[lang]
    
    
  def __contains__(self, item):
    return item in self._lang_dirs


  def __iter__(self):
    return self._lang_dirs.__iter__()
    
    
  def keys(self):
    return self._lang_dirs.keys()
    
    
  def _save(self):
    # Write an XML file containing all languages and proxy info
    root_el = self._core.data.xml.element('Subtitles')
    for lang in self._lang_dirs:
      lang_dir = self._lang_dirs[lang]
      el = lang_dir._element('Language', 'Subtitle')
      el.set('code', lang)
      root_el.append(el)
    
    data = self._core.data.xml.to_string(root_el)
    path = self._path + '.xml'
    self._core.storage.save(path, data)
    
    

class MediaStream(object):
  def __init__(self, core, el):
    self._core = core
    
    for key, value in dict(el.attrib).items():
      if value == None:
        setattr(self, key, value)
      else:
        if key in ('index', 'type', 'id'):
          attr_type = long
        else:
          attr_type = str
        setattr(self, key, attr_type(value))


    
class MediaPart(object):
  def __init__(self, core, el):
    self._core = core
    self.streams = []
    
    for key, value in dict(el.attrib).items():
      if value == None:
        setattr(self, key, value)
      else:
        if key == 'size':
          attr_type = long
        else:
          attr_type = str
        setattr(self, key, attr_type(value))
    
    try:
      self._path = self._core.storage.join_path(self._core.app_support_path, 'Media', 'localhost', self.hash[0], self.hash[1:] + '.bundle')
    
    except:
      self._path = None
      self._core.log.error('We seem to be missing the hash for media item [%s]', self.file)
    
    # See if we have a track. We don't want to do the media bundle thing for tracks.
    is_track = hasattr(self, 'metadataType') and self.metadataType == '10'
    
    if self._path and is_track == False:
      self.thumbs = MediaContentsDirectory(self._core, self._core.storage.join_path(self._path, 'Contents', 'Thumbnails'))
      self.art = MediaContentsDirectory(self._core, self._core.storage.join_path(self._path, 'Contents', 'Art'))
      self.subtitles = SubtitlesDirectory(self._core, self._core.storage.join_path(self._path, 'Contents', 'Subtitle Contributions', self._core.identifier))
    else:
      self.thumbs = FakeMediaObject()
      self.art = FakeMediaObject()
      self.subtitles = FakeMediaObject()
      
    for child in el:
      if child.tag == 'MediaStream':
        stream = MediaStream(self._core, child)
        self.streams.append(stream)


class Setting(object):
  
  def __init__(self, core, el):
    self._core = core
    self.id = el.get('id')
    self.value = el.get('value')

      
class MediaItem(object):

  def __init__(self, core, el):
    self._core = core
    self.parts = []
    for child in el:
      if child.tag == 'MediaPart':
        part = MediaPart(self._core, child)
        self.parts.append(part)
        


class MediaDict(dict):

  def __contains__(self, item):
    ret = dict.__contains__(self, item)
    if ret:
      return True
    else:
      return dict.__contains__(self, str(item))
  

  def __getitem__(self, item):
    if dict.__contains__(self, str(item)):
      return dict.__getitem__(self, str(item))
    return dict.__getitem__(self, item)
       


class MediaTree(object):

  def __init__(self, core, el, level_names=[], child_id=None, level_attribute_keys=[]):
    self._core = core
    self.items = []
    self.settings = {}
    self.children = []

    # Copy attributes from the XML element to the tree object
    [setattr(self, key, value) for key, value in dict(el.attrib).items()]
    
    # Rename
    if hasattr(self, 'originallyAvailableAt'):
      self.originally_available_at = self.originallyAvailableAt
    if hasattr(self, 'addedAt'):
      self.added_at = self.addedAt
    if hasattr(self, 'refreshedAt'):
      self.refreshed_at = self.refreshedAt
    
    level_name = None
    subitems = {}
    next_level_names = []
    next_level_attribute_keys = []
    
    if len(level_names) > 0:
      level_name = level_names[0]
      if len(level_names) > 1:
        next_level_names = level_names[1:]
      if len(level_attribute_keys) > 1:
        next_level_attribute_keys = level_attribute_keys[1:]
      subitems = MediaDict()
      
    for child in el:
      if child.tag == 'MetadataItem':
        # If we were given a child ID, ignore all non-matching children.
        if child_id and child.get('id') != child_id:
          continue

        if subitems == None:
          print "No subitems can be set for level", level_name
          continue
        index = child.get(level_attribute_keys[0] if len(level_attribute_keys) > 0 else 'index')
        subitem = MediaTree(self._core, child, next_level_names, level_attribute_keys=next_level_attribute_keys)
        subitems[index] = subitem
        self.children.append(subitem)
      elif child.tag == 'MediaItem':
        item = MediaItem(self._core, child)
        self.items.append(item)
      elif child.tag == 'Setting':
        setting = Setting(self._core, child)
        self.settings[setting.id] = setting.value
      else:
        self._core.log.error('Unknown tag: %s', child.tag)
        
    # Store the current level name so we can access sub-items programmatically
    if level_name:
      self._level_name = level_name
      setattr(self, level_name, subitems)
    else:
      self._level_name = None
      

  def all_parts(self):
    """ Return an array of all parts owned by this item and its' sub-items """
    parts = []
    
    # Add all parts from all items to the list
    [[parts.append(part) for part in item.parts] for item in self.items]

    # If we have sub-items, get the parts from each sub-item and add it to the list
    if self._level_name:
      subitems = getattr(self, self._level_name)
      [parts.extend(subitems[index].all_parts()) for index in subitems]
      
    return parts
    


class MediaObject(object):

  """
    A MediaObject represents a media item discovered by PMS and encapsulates any information
    provided by the server. It is intended to provide hints to metadata agents when
    finding metadata to download.
  """
  
  _attrs = dict()
  _model_name = None
  _versioned_model_names = {}
  _media_type_name = None
  _parent_model_name = None
  _parent_link_name = None
  _parent_set_attr_name = None
  _type_id = 0
  _level_names = []
  _level_attribute_keys = []
  

  def __init__(self, access_point, version=None, **kwargs):
    self._access_point = access_point
    self.primary_agent = None
    self.primary_metadata = None
    self.guid = None
    self.filename = None
    self.parent_metadata = None
    self.parentGUID = None
    self.tree = None
    self.id = None
    self.hash = None
    self.originally_available_at = None
    
    cls = type(self)
    for name in cls._attrs:
      setattr(self, name, cls._attrs[name])
    
    for name in kwargs:
      if hasattr(self, name):
        setattr(self, name, kwargs[name])

    # Get the media tree if we got an ID passed down.
    if self.id != None:
      try:
        setattr(self, 'tree', Media.TreeForDatabaseID(self.id, type(self)._level_names, level_attribute_keys=type(self)._level_attribute_keys))
      except:
        self._access_point._core.log_exception("Exception when constructing media object")

    # Load primary agent's metadata.
    if self.primary_agent != None and self.guid != None:
      primary_access_point = self._access_point._accessor.get_access_point(self.primary_agent, read_only=True)
      model_cls = getattr(primary_access_point, cls._versioned_model_names.get(version) or cls._model_name)
      self.primary_metadata = model_cls[self.guid]
      
    # Load the parent's metadata.
    if self.parentGUID and cls._parent_model_name:
      model_cls = getattr(self._access_point, cls._parent_model_name)
      self.parent_metadata = model_cls[self.parentGUID]
      
    del self.parentGUID
    

  def __getattr__(self, name):
    if hasattr(self, 'tree') and hasattr(self.tree, name):
      return getattr(self.tree, name)
    else:
      return object.__getattr__(self, name)
      
    

class Media(object):

  _core = None
  
  @classmethod
  def _class_named(cls, media_type):
    if hasattr(cls, media_type):
      media_class = getattr(cls, media_type)
      if isinstance(media_class, type):
        return media_class
        
  @classmethod
  def TreeForDatabaseID(cls, dbid, level_names=[], host='127.0.0.1', parent_id=None, level_attribute_keys=[]):
    xml_str = cls._core.networking.http_request('http://%s:32400/library/metadata/%s/tree' % (host, str(parent_id or dbid)), cacheTime=0, immediate=True)
    xml_obj = cls._core.data.xml.from_string(xml_str)
    tree = MediaTree(cls._core, xml_obj[0], level_names, child_id=dbid if parent_id else None, level_attribute_keys=level_attribute_keys)
    if tree.title == None:
      xml_str = cls._core.networking.http_request('http://%s:32400/library/metadata/%s' % (host, str(dbid)), cacheTime=0, immediate=True)
      xml_obj = cls._core.data.xml.from_string(xml_str)
      try:
        tree.title = xml_obj.xpath('//Video')[0].get('title')
      except:
        cls._core.log.error('Unable to set title for metadata item %s', str(dbid))
    return tree

      
  class Movie(MediaObject):
    _model_name = 'Movie'
    _type_id = 1
    _attrs = dict(
      primary_metadata = None,
      name = None,
      openSubtitlesHash = None,
      year = None,
      duration = None,
    )
    

  class TV_Show(MediaObject):
    _model_name = 'TV_Show'
    _type_id = 2
    _attrs = dict(
      show = None,
      season = None,
      episode = None,
      name = None,
      openSubtitlesHash = None,
      year = None,
      duration = None,
      episodic = True
    )
    _level_names = ['seasons', 'episodes']
    

  class Album(MediaObject):
    _model_name = 'LegacyAlbum'
    _media_type_name = 'Album'
    _parent_model_name = 'Artist'
    _parent_link_name = 'artist'
    _parent_set_attr_name = 'albums'
    _type_id = 9
    _attrs = dict(
      name = None,
      artist = None,
      album = None,
      track = None,
      index = None,
      parentGUID = None
    )
    _level_names = ['tracks']
    

  class Artist(MediaObject):
    _model_name = 'LegacyArtist'
    _versioned_model_names = {
      2: 'ModernArtist'
    }
    _media_type_name = 'Artist'
    _type_id = 8
    _attrs = dict(
      artist = None,
      album = None,
      track = None,
      index = None
    )
    _level_names = ['albums', 'tracks']
    _level_attribute_keys = ['guid']
    

  class PhotoAlbum(MediaObject):
    _model_name = 'PhotoAlbum'
    _type_id = 12
    _attrs = dict()
    _level_names = ['photos']
    

  class Photo(MediaObject):
    _model_name = 'Photo'
    _type_id = 13
    _attrs = dict()
    


class MetadataModelClassWrapper(object):

  def __init__(self, cls):
    self._cls = cls
  

  @property
  def _core(self):
    return self._cls._core
    

  @property
  def _access_point(self):
    return self._cls._access_point
    

  def search(self, lang, **kwargs):
    media_class = Media._class_named(self._cls.__name__)
    if media_class == None:
      return
    media_type = media_class._type_id
    identifier = self._access_point._identifier
    return self._core.messaging.call_external_function(
      '..system',
      '_AgentService:Search',
      kwargs = dict(
        identifier = identifier,
        mediaType = media_type,
        lang = lang,
        **kwargs
      )
    )
  

  def __getattr__(self, name):
    return getattr(self._cls, name)
  

  def __setattr__(self, name, value):
    if name[0] != '_':
      setattr(self._cls, name, value)
    else:
      object.__setattr__(self, name, value)
  

  def __getitem__(self, name):
    return self._cls[name]
  

  def __call__(self, *args, **kwargs):
    return self._cls(*args, **kwargs)
    


class MetadataAccessPointWrapper(object):

  def __init__(self, access_point):
    self._access_point = access_point
  

  @property
  def _core(self):
    return self._access_point._core
  

  def __getattr__(self, name):
    metadata_class = getattr(self._access_point, name)
    return MetadataModelClassWrapper(metadata_class)
  

  def __getitem__(self, name):
    new_access_point = self._access_point[name]
    return MetadataAccessPointWrapper(new_access_point)
  


class AgentKit(BaseKit):

  """
    The AgentKit API class handles commmunication between plug-ins and the agent service,
    responding to incoming requests and forwarding them to custom Agent classes as appropriate.
  """
  
  _agents = list()
  _shared_instance = None

  _included_policies = [
    Framework.policies.BundlePolicy,
  ]

  _excluded_policies = [
    Framework.policies.CloudPolicy,
  ]
  
  def _init(self):
    # Generate the agent classes
    self.TV_Shows = BaseAgent.generate(self._core, 'TV_Shows', Media.TV_Show)
    self.Movies = BaseAgent.generate(self._core, 'Movies', Media.Movie)
    self.Artist = BaseAgent.generate(self._core, 'Artist', Media.Artist)
    self.Album = BaseAgent.generate(self._core, 'Album', Media.Album)
    self.Photos = BaseAgent.generate(self._core, 'Photos', Media.Photo)
    
    self._setup_complete = False
    self._pushing_info_lock = self._core.runtime.lock()
    type(self)._shared_instance = self
    Media._core = self._core
    
    self._publish(Media),
    self._publish(self._metadata_access_point_wrapper, 'Metadata')
    
    
  def _setup(self):
    if self._setup_complete:
      return
      
    # Create a model access point
    self._accessor = self._core._metadata_model_accessor
    self._access_point = self._accessor.get_access_point(self._core.identifier)
    
    # Expose functions via the messaging component for access by the agent service
    self._core.messaging.expose_function(self._search, '_AgentKit:Search')
    self._core.messaging.expose_function(self._update, '_AgentKit:UpdateMetadata')
    self._core.messaging.expose_function(self._erase, '_AgentKit:EraseMetadata')
    
    self._access_point_wrapper = MetadataAccessPointWrapper(self._access_point)


  @property
  def _metadata_access_point_wrapper(self):
    self._setup()
    return self._access_point_wrapper

    
  def _push_agent_info(self):
    self._pushing_info_lock.acquire()
    try:
      agents = []
      for agent in AgentKit._agents:
      
        media_types = []
        for media_class in agent._media_types:
          name = media_class._media_type_name if media_class._media_type_name else media_class._model_name
          media_types.append(name)

        should_add = True
        for lang in agent.languages:
          if not self._core.localization.language_code_valid(lang):
            self._core.log.error("The agent named '%s' contains the invalid language code '%s' and will not be exposed to the media server.", agent.name, lang)
            should_add = False
            break
      
        if should_add:
          info_dict = dict(
            name = agent.name,
            languages = agent.languages,
            media_types = media_types,
            contributes_to = agent.contributes_to,
            accepts_from = agent.accepts_from,
            primary_provider = agent.primary_provider,
            fallback_agent = agent.fallback_agent,
            persist_stored_files = agent.persist_stored_files,
            prefs = self._core.storage.file_exists(self._core.storage.join_path(self._core.bundle_path, 'Contents', 'DefaultPrefs.json')),
            version = agent.version
          )
          agents.append(info_dict)
      
      if len(agents) > 0:
        self._core.log.debug("Updating agent information: %s", agents)
      
        self._core.messaging.call_external_function(
          '..system',
          '_AgentService:UpdateInfo',
          kwargs = dict(
            identifier = self._core.identifier,
            agent_info = agents
          )
        )
    finally:
      self._pushing_info_lock.release()
    

  def _search(self, media_type, lang, manual, kwargs, version=0, primary=True):
    """
      Received a search request - find an agent that handles the given media type, create a
      Media object from the given arguments and forward the request to the agent instance.
    """
    try:
      cls = Media._class_named(media_type)
      for agent in AgentKit._agents:
        if cls in agent._media_types and agent.version == version:
          try:
            self._core.log.info("Searching for matches for "+str(kwargs))

            if agent.version > 0:
              hints = cls(self._access_point, version=agent.version, **kwargs)
              tree = Media.TreeForDatabaseID(kwargs['id'], level_names=cls._level_names, parent_id=kwargs.get('parentID'), level_attribute_keys=cls._level_attribute_keys)
              results = self._core.sandbox.environment['ObjectContainer']()
              f_args = (results, tree, hints, lang)
              f_kwargs = {}

              # Add the manual parameter if the function expects it.
              if Framework.utils.function_accepts_arg(agent.search, 'manual'):
                f_kwargs['manual'] = manual

              if Framework.utils.function_accepts_arg(agent.search, 'partial'):
                f_kwargs['partial'] = 'parentID' in kwargs

              if Framework.utils.function_accepts_arg(agent.search, 'primary'):
                f_kwargs['primary'] = primary

              agent.search(*f_args, **f_kwargs)

              results.objects.sort(key=lambda item: -item.score)
              return self._core.data.xml.to_string(results._to_xml())

            else:
              media = cls(self._access_point, **kwargs)
              f_kwargs = {}

              # Check to see if the 'manual' arg should be passed.
              if Framework.utils.function_accepts_arg(agent.search, 'manual'):
                f_kwargs['manual'] = manual

              if Framework.utils.function_accepts_arg(agent.search, 'primary'):
                f_kwargs['primary'] = primary

              # If an agent accepts a tree, assume that it's also going to return an ObjectContainer of SearchResults.
              if Framework.utils.function_accepts_arg(agent.search, 'tree'):
                f_kwargs['tree'] = Media.TreeForDatabaseID(kwargs['id'], level_names=cls._level_names, parent_id=kwargs.get('parentID'), level_attribute_keys=cls._level_attribute_keys)

                results = self._core.sandbox.environment['ObjectContainer']()
                f_args = (results, media, lang)

                agent.search(*f_args, **f_kwargs)

                return self._core.data.xml.to_string(results._to_xml())
              
              else:
                results = Framework.objects.MediaContainer(self._core)
                f_args = (results, media, lang)

                agent.search(*f_args, **f_kwargs)

                results.Sort('year')
                results.Sort('score', descending=True)
                return results

          except:
            self._core.log_exception("Exception in the search function of agent named '%s', called with keyword arguments %s", agent.name, str(kwargs))
    except:
      self._core.log_exception("Exception finding an agent for type %s", media_type)


  def _update(self, media_type, guid, id, lang, dbid=None, parentGUID=None, force=False, version=0, parentID=None, periodic=False):
    """
      Received an update request. Find the agent that handles the given media type, get a
      metadata object with the specified model & GUID, instruct the agent to update it,
      then serialize it before returning.
    """
    try:
      cls = Media._class_named(media_type)
      for agent in AgentKit._agents:
        if cls in agent._media_types and agent.version == version:
          if version > 0 and parentGUID is not None:
            assert parentID is not None
            child_guid = guid
            child_id = id
            guid = parentGUID
            id = parentID
          else:
            child_guid = None
            child_id = None

          model_name = cls._versioned_model_names[version] if version > 0 else cls._model_name
          metadata_cls = getattr(self._access_point, model_name)
          obj = metadata_cls[guid]

          if id != None:
            if obj._id == None:
              obj._id = id
            elif obj._id != id:
              self._core.log.debug("Whacking existing data because the ID changed (%s -> %s)" % (obj._id, id))
              obj = metadata_cls()
              obj._uid = guid
              obj._id = id
            
          media = None
          prefs = {}
          try:
            if dbid:
              media = Media.TreeForDatabaseID(dbid,
                                              level_names=cls._level_names,
                                              parent_id=parentID if version > 0 else None,
                                              level_attribute_keys=cls._level_attribute_keys)

              # Start with defaults and update with incoming values.
              keys = ['artistBios', 'albumReviews', 'popularTracks', 'concerts', 'genres', 'albumPosters']
              prefs = dict((key, 1) for key in keys)

              if hasattr(media, 'librarySectionPrefs'):
                # Read prefs and convert values to integers.
                incomingPrefs = dict(urlparse.parse_qsl(media.librarySectionPrefs))
                incomingPrefs = { k : int(v) for k, v in incomingPrefs.items() }
                prefs.update(incomingPrefs)
            else:
              media = FakeMediaObject()
              self._core.log.error("No database ID provided - faking the media object")
          except:
            self._core.log_exception("Exception when constructing media object for dbid %s", dbid)
              
          try:
            kwargs = {}
            if Framework.utils.function_accepts_arg(agent.update, 'force'):
              kwargs['force'] = force
            if Framework.utils.function_accepts_arg(agent.update, 'child_guid'):
              kwargs['child_guid'] = child_guid
            if Framework.utils.function_accepts_arg(agent.update, 'child_id'):
              kwargs['child_id'] = child_id
            if Framework.utils.function_accepts_arg(agent.update, 'periodic'):
              kwargs['periodic'] = periodic
            if Framework.utils.function_accepts_arg(agent.update, 'prefs'):
              kwargs['prefs'] = prefs

            agent.update(obj, media, lang, **kwargs)
          except:
            self._core.log_exception("Exception in the update function of agent named '%s', called with guid '%s'", agent.name, guid)
          obj._write()
    except:
      self._core.log_exception("Exception updating %s instance '%s'", media_type, guid)
      

  def _erase(self, media_type, guid):
    cls = Media._class_named(media_type)
    for agent in AgentKit._agents:
      if cls in agent._media_types:
        metadata_cls = getattr(self._access_point, cls._model_name)
        metadata_cls.erase(guid)
    

  @classmethod
  def _register_agent_class(cls, agent_class):
    """
      Registers an instance of a newly created agent class with AgentKit.
    """
    cls._shared_instance._setup()
    cls._agents.append(agent_class())
    cls._shared_instance._push_agent_info()
   

   
class AgentMetaclass(type):

  base_class = None
  
  def __new__(meta, classname, bases, dct):
    """
      Called when creating a new agent class - registers the new class automatically with
      AgentKit. Make sure we don't call this when defining BaseAgent or its direct subclasses.
    """
    cls = type.__new__(meta, classname, bases, dct)
    if not (AgentMetaclass.base_class in bases or object in bases):
      if cls._core:
        cls._core.log.debug('Creating new agent class called %s', classname)
      AgentKit._register_agent_class(cls)
    return cls 

    
class BaseAgent(object):

  __metaclass__ = AgentMetaclass
  _class_media_types = []
  _core = None
  
  name = 'Unnamed Agent'
  languages = []
  primary_provider = True
  contributes_to = None
  accepts_from = None
  fallback_agent = None
  persist_stored_files = True
  version = 0
  
  def __init__(self):
    self._media_types = list(type(self)._class_media_types)

  # Functions that agents should implement
  def search(self, media, lang): pass
  def update(self, metadata, media, lang): pass
    

  @classmethod
  def generate(cls, core, name, media_type):
    """
      Generates a new agent class with the given name that handles the given media type (to be used
      as a base class for custom agents)
    """
    media_types = list(cls._class_media_types)
    media_types.append(media_type)
    return type(name, (cls,), dict(__metaclass__ = cls.__metaclass__, _class_media_types = media_types, _core=core))


# We need to set this here because BaseAgent doesn't exist when AgentMetaclass is defined.
AgentMetaclass.base_class = BaseAgent
