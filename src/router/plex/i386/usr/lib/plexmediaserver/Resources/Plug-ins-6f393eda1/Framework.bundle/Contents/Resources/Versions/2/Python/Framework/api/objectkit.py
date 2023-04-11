#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import urllib
import urlparse
from Framework.modelling.objects import generate_class, generate_model_interface_class, generate_model_interface_container_class
from Framework.components.runtime import callback_string, indirect_callback_string
from base import BaseKit

class WebkitURL(unicode): pass

class HLSURL(unicode): pass
class CallbackHLSURL(callback_string, HLSURL): pass
class IndirectHLSURL(indirect_callback_string, HLSURL): pass

class RTMPURL(unicode): pass
class CallbackRTMPURL(callback_string, RTMPURL): pass
class IndirectRTMPURL(indirect_callback_string, RTMPURL): pass

class EmbedURL(unicode): pass
class CallbackEmbedURL(callback_string, EmbedURL): pass
class IndirectEmbedURL(indirect_callback_string, EmbedURL): pass

class ObjectWithHTTPHeaders(Framework.modelling.objects.Object):
  _attribute_list = ['http_headers', 'user_agent', 'http_cookies']
  
  @classmethod
  def _convert_header_dict_to_string(cls, d):
    return '&'.join(['%s=%s' % (name, urllib.quote(value)) for name, value in d.items()])
    
  @classmethod
  def _apply_http_headers(cls, self, el):
    # Get the HTTP headers
    if hasattr(self, 'http_headers') and self.http_headers:
      header_dict = dict(self.http_headers)
    else:
      header_dict = dict()
      
    # Set old-style attributes for backwards compatibility with older clients
    if 'User-Agent' in header_dict:
      el.set('userAgent', header_dict['User-Agent'])
    if 'Cookie' in header_dict:
      el.set('httpCookies', header_dict['Cookie'])
      
    # Add old-style HTTP headers to the dictionary
    if hasattr(self, 'user_agent') and self.user_agent:
      header_dict['User-Agent'] = self.user_agent
    if hasattr(self, 'http_cookies') and self.http_cookies:
      header_dict['Cookie'] = self.http_cookies
      
    # Convert the dictionary to a HTTP header string and set the attribute on the element
    if header_dict:
      el.set('httpHeaders', ObjectWithHTTPHeaders._convert_header_dict_to_string(header_dict))
    
  def _to_xml(self):
    el = Framework.modelling.objects.Object._to_xml(self)
    ObjectWithHTTPHeaders._apply_http_headers(self, el)
    return el
    

class ObjectWithPOSTCallback(ObjectWithHTTPHeaders):

  @classmethod
  def _apply_post_headers(cls, self, el):
    if hasattr(self, 'post_headers') and self.post_headers:
      el.set('postHeaders', ObjectWithHTTPHeaders._convert_header_dict_to_string(self.post_headers))
      
  @classmethod
  def _apply_post_url_and_headers(cls, self, el):
    # If the key is an instance of callback_string, check for the extra POST callback
    # properties and add them to the XML if found.
    if isinstance(self, callback_string) and self.post_url:
      el.set('postURL', self.post_url)
      ObjectWithPOSTCallback._apply_post_headers(self, el)
    
  def _to_xml(self):
    el = ObjectWithHTTPHeaders._to_xml(self)
    ObjectWithPOSTCallback._apply_post_url_and_headers(self.key, el)

    """
      NASTY FIX FOR iOS

      Versions of the iOS app earlier than 2.5 insert the postURL parameter at the end of the URL,
      after the X-Plex-* parameters. PMS strips everything after the first X-Plex-* parameter,
      meaning the post URL never makes it back to the framework, causing the POST data to be
      ignored and the function to not be found (since it's registered as a GET, not a POST).

      Therefore, we need to check for iOS <2.5 and add the postURL argument ourselves.

    """

    if isinstance(self.key, callback_string) and \
      self.key.post_url and self._core.sandbox.context.platform == 'iOS' and \
      self._core.sandbox.context.product == 'Plex/iOS' and \
      Framework.utils.version_at_least(self._core.sandbox.context.client_version, 2,5) == False:

      new_key = self.key + ('&' if '?' in self.key else '?') + 'postURL=' + urllib.quote(self.key.post_url)
      el.set('key', new_key)
    
    return el
    
    
class ProviderObject(Framework.modelling.objects.Object):
  xml_tag = 'Provider'
  _attribute_list = ['key', 'title', 'type', 'machine_identifier', 'source_icon']


class MetadataItem(Framework.modelling.objects.Container):
  xml_tag = 'MetadataItem'
  _attribute_list = ['type', 'id', 'title', 'guid', 'index', 'originally_available_at', 'score', 'thumb', 'matched']

class SearchResult(Framework.modelling.objects.Container):
  xml_tag = 'SearchResult'
  _attribute_list = ['type', 'id', 'name', 'guid', 'index', 'year', 'score', 'thumb', 'matched', 'parentName', 'parentID', 'parentGUID', 'parentIndex']

class AudioStreamObject(Framework.modelling.objects.Object):
  xml_tag = 'Stream'
  _attribute_list = [
    'id',
    'language_code',
    'sampling_rate',
    'stream_type',
    'index',
    'selected',

    # Attributes migrated from MediaObject
    'bitrate',
    'codec',
    'duration',
    'channels',
  ]

  def __init__(self, **kwargs):
    if 'index' not in kwargs:
      kwargs['index'] = '1'
    if 'id' not in kwargs:
      kwargs['id'] = '2'
    Framework.modelling.objects.Object.__init__(self, stream_type=2, **kwargs)



class VideoStreamObject(Framework.modelling.objects.Object):
  xml_tag = 'Stream'
  _attribute_list = [
    'id',
    'stream_type',
    'frame_rate',
    'level',
    'profile',
    'index',
    'selected',

    # Attributes migrated from MediaObject
    'bitrate',
    'width',
    'height',
    'codec',
    'duration',
  ]

  def __init__(self, **kwargs):
    if 'index' not in kwargs:
      kwargs['index'] = '0'
    if 'id' not in kwargs:
      kwargs['id'] = '1'
    Framework.modelling.objects.Object.__init__(self, stream_type=1, **kwargs)


class PartObject(Framework.modelling.objects.Container, ObjectWithPOSTCallback):
  xml_tag = 'Part'

  _children_attr_name = 'streams'
  _child_types = [AudioStreamObject, VideoStreamObject]
  _attribute_list = [
    'key',
    'file',
    'duration',
    'http_headers',

    # New attributes migrated from MediaObject
    'container',
    'optimized_for_streaming',
    'duration',
    'protocol',
  ]

  def __init__(self, **kwargs):
    if 'file' not in kwargs:
      kwargs['file'] = ''
    Framework.modelling.objects.Container.__init__(self, **kwargs)

  def _to_xml(self):
    el = Framework.modelling.objects.Container._to_xml(self)
    ObjectWithPOSTCallback._apply_post_url_and_headers(self.key, el)
    return el

        
class MediaObject(Framework.modelling.objects.Container):
  xml_tag = 'Media'
  _logged_warnings = []

  def _warn(self, msg, *args):
    if msg not in type(self)._logged_warnings:
      type(self)._logged_warnings.append(msg)
      self._core.log.warn(msg, *args)


  _children_attr_name = 'parts'
  _child_types = [PartObject]
  _attribute_list = [
    'protocols',
    'platforms',
    'bitrate',
    'aspect_ratio',
    'audio_channels',
    'audio_codec',
    'video_codec',
    'video_resolution',
    'container',
    'video_frame_rate',
    'duration',
    'width',
    'height',
    'protocol',
    'optimized_for_streaming'
  ]
  
  def __init__(self, **kwargs):
    
    def get_list(name):
      
      if name in kwargs:
        result = kwargs[name]
        del kwargs[name]
        return result
      else:
        return []
        
    self._platforms = get_list('platforms')
    self._protocols = get_list('protocols')

    if self.platforms != None and len(self.platforms) > 0:
      self._warn("The 'platforms' attribute is deprecated and should not be assigned (%s).", str(self.platforms))

    if self.protocols != None and len(self.protocols) > 0:
      self._warn("The 'protocols' attribute is deprecated and should not be assigned (%s).", str(self.protocols))
    
    # If the core attribute for the given name contains one value, return it
    def set_default_attr(attr_name, attr_key):
      if hasattr(self._core, 'attributes') and attr_name not in kwargs and attr_key in self._core.attributes and len(self._core.attributes[attr_key]) == 1:
        kwargs[attr_name] = self._core.attributes[attr_key][0]
    
    if Framework.code.context.flags.indirect not in self._context.flags:
      set_default_attr('audio_codec', Framework.core.AUDIO_CODEC_KEY)
      set_default_attr('video_codec', Framework.core.VIDEO_CODEC_KEY)
      set_default_attr('container', Framework.core.MEDIA_CONTAINER_KEY)
    
    Framework.modelling.objects.Container.__init__(self, **kwargs)
    
  @property
  def platforms(self):
    return self._platforms
    
  @property
  def protocols(self):
    return self._protocols

  def to_xml(self):
    return self._to_xml()
    
  def _to_xml(self):
    if len(self.parts) > 0:
      part = self.parts[0]

      if isinstance(part.key, WebkitURL):
        if self.protocol == None: self.protocol = 'webkit'
        
      elif isinstance(part.key, HLSURL):
        if self.protocol == None: self.protocol = 'hls'
        if self.audio_codec == None: self.audio_codec = 'aac'
        if self.video_codec == None: self.video_codec = 'h264'
        if self.container == None: self.container = 'mpegts'

      elif isinstance(part.key, RTMPURL):
        if self.protocol == None: self.protocol = 'rtmp'
      
      elif isinstance(part.key, EmbedURL):
        if self.protocol == None: self.protocol = 'embed'

    # Convert resolution to width/height if not already set.
    if self.video_resolution and self.width == None and self.height == None:
      height = 360 if self.video_resolution == 'sd' else int(self.video_resolution)
      aspect_ratio = float(self.aspect_ratio) if self.aspect_ratio != None and float(self.aspect_ratio) > 0 else float(16) / float(9)

      self.height = height
      self.width = int(8 * round((self.height * aspect_ratio)/8))

    # Copy old-style attributes to parts and create stream objects if none exist.
    for part in self.parts:
      if part.container == None and self.container != None: part.container = self.container
      if part.optimized_for_streaming == None and self.optimized_for_streaming != None: part.optimized_for_streaming = self.optimized_for_streaming
      if part.duration == None and self.duration != None: part.duration = self.duration

      if len(self.parts) == 1:
        if part.duration == None and self.duration != None: part.duration = self.duration
      else:
        self._warn("Media object has multiple parts - unable to synthesize duration.")

      if len(part.streams) == 0:
        self._warn("Media part has no streams - attempting to synthesize")

        bitrate = int(self.bitrate) if self.bitrate is not None else None

        if 'VideoStreamObject' in self._sandbox.environment:
          video_stream = self._sandbox.environment['VideoStreamObject'](
            width = self.width,
            height = self.height,
            codec = self.video_codec,
          )
          if len(self.parts) == 1:
            video_stream.duration = self.duration

          if bitrate > 256:
            video_stream.bitrate = bitrate - 256
          part.add(video_stream)

        if 'AudioStreamObject' in self._sandbox.environment:
          audio_stream = self._sandbox.environment['AudioStreamObject'](
            codec = self.audio_codec,
            duration = self.duration,
            channels = self.audio_channels,
          )
          if len(self.parts) == 1:
            audio_stream.duration = self.duration
          if bitrate > 256:
            audio_stream.bitrate = 256
          part.add(audio_stream)

          # Add media info to the URL for indirect callbacks.
          if isinstance(part.key, indirect_callback_string):
            media_info = dict(
              bitrate = self.bitrate,
              aspect_ratio = self.aspect_ratio,
              audio_channels = self.audio_channels,
              audio_codec = self.audio_codec,
              video_codec = self.video_codec,
              video_resolution = self.video_resolution,
              container = self.container,
              video_frame_rate = self.video_frame_rate,
              duration = self.duration,
              width = self.width,
              height = self.height,
              protocol = self.protocol,
              optimized_for_streaming = self.optimized_for_streaming
            )

            # Rewrite the key to include media info, making sure we copy POST properties.
            new_key = indirect_callback_string(part.key + ('&' if '?' in part.key else '?') + 'mediaInfo=' + urllib.quote(self._core.data.json.to_string(media_info)))

            new_key.post_url = part.key.post_url
            new_key.post_headers = part.key.post_headers

            part.key = new_key

      # If this part has stream info added, make sure that one of each type is flagged as selected.
      all_video_streams = [stream for stream in part.streams if isinstance(stream, VideoStreamObject)]
      all_audio_streams = [stream for stream in part.streams if isinstance(stream, AudioStreamObject)]

      for stream_list in (all_video_streams, all_audio_streams):
        if len(stream_list) > 0:
          selected_count = len([stream for stream in stream_list if stream.selected is True])
          if selected_count == 0:
            stream_list[0].selected = True
          elif selected_count > 1:
            self._core.log.warn("Part with key '%s' has multiple streams of the same type that are marked as selected.", part.key)


    el = Framework.modelling.objects.Container._to_xml(self)

    # If the first media part is an IndirectFunction instance, flag it as 'indirect'
    if isinstance(self._objects[0].key, indirect_callback_string):
      el.set('indirect', '1')
  
    return el


class DirectoryObject(ObjectWithPOSTCallback):
  xml_tag = 'Directory'
  _attribute_list = ['key', 'title', 'tagline', 'summary', 'thumb', 'art', 'duration', 'http_headers']


class PageObject(DirectoryObject):
  def _to_xml(self):
    el = DirectoryObject._to_xml(self)
    el.set('paging', '1')
    return el


class NextPageObject(PageObject):
  _unique = True
  
  def __init__(self, **kwargs):
    # Default search thumb.
    if 'thumb' not in kwargs:
      kwargs['thumb'] = self._core.runtime.external_resource_path('NextPage.png', self._sandbox)

    if 'title' not in kwargs:
      kwargs['title'] = "More..."
      
    PageObject.__init__(self, **kwargs)
  
  
class PlaylistObject(DirectoryObject):
  _attribute_list = ['key', 'title', 'tagline', 'summary', 'thumb', 'art', 'duration', 'http_headers', 'radio']
  
  def _to_xml(self):
    el = DirectoryObject._to_xml(self)
    el.set('type', 'playlist')
    return el  


class InputDirectoryObject(DirectoryObject):
  _attribute_list = ['key', 'title', 'tagline', 'summary', 'thumb', 'art', 'prompt', 'http_headers']
  
  def _to_xml(self):
    el = Framework.modelling.objects.Object._to_xml(self)
    el.set('search', '1')
    return el
    
    
class SearchDirectoryObject(InputDirectoryObject):
  _attribute_list = ['key', 'title', 'tagline', 'summary', 'thumb', 'art', 'prompt', 'http_headers', 'term']

  def __init__(self, name=None, identifier=None, **kwargs):
    # Check that a 'key' attribute hasn't been passed
    if 'key' in kwargs:
      raise Framework.exceptions.FrameworkException("The 'key' attribute for SearchDirectoryObject instances is generated automatically.")
    
    # If no identifier has been given, use the current plug-in's identifier
    if identifier == None:
      identifier = self._core.identifier
      
    if identifier not in self._core.services.search_services:
      raise Framework.exceptions.FrameworkException("No search services are available for the identifier '%s'", identifier)
      
    # If no name has been given, use the first available service
    if name == None:
      name = self._core.services.search_services[identifier].keys()[0]
      
    # Default search thumb.
    if 'thumb' not in kwargs:
      kwargs['thumb'] = self._core.runtime.external_resource_path('Search.png', self._sandbox)
    
    # Generate a key & init the superclass
    key = '/:/plugins/%s/serviceSearch?identifier=%s&name=%s' % (self._core.identifier, identifier, urllib.quote(name))
    InputDirectoryObject.__init__(self, key=key, **kwargs)
  
    
class PopupDirectoryObject(DirectoryObject):
  _attribute_list = ['key', 'title', 'tagline', 'summary', 'thumb', 'art', 'duration']

  def _to_xml(self):
    el = DirectoryObject._to_xml(self)
    el.set('popup', '1')
    return el
    
    
class PrefsObject(Framework.modelling.objects.Object):
  xml_tag = 'Directory'
  _attribute_list = ['title', 'tagline', 'summary', 'thumb', 'art']
  _unique = True
  
  def __init__(self, **kwargs):
    # Default search thumb.
    if 'thumb' not in kwargs:
      kwargs['thumb'] = self._core.runtime.external_resource_path('Prefs.png', self._sandbox)
      
    Framework.modelling.objects.Object.__init__(self, **kwargs)
    

  def _to_xml(self):
    el = Framework.modelling.objects.Object._to_xml(self)
    el.set('key', '/:/plugins/%s/prefs' % self._core.identifier)
    el.set('settings', '1')
    return el
    
    
class MetadataObject(Framework.modelling.objects.ModelInterfaceObject):
  _attribute_list = [
    'url',
    'file',
    'http_cookies',
    'user_agent',
    'http_headers',
    'deferred',
    'source_icon',
  ]
  
  def _to_xml(self):
    # Check that the object has valid attributes set and raise an exception if they are
    # None or an empty string.
    #
    # All objects require either a URL to be set, so the framework can automatically generate
    # unique identifiers (rating_key) and metadata URLs (key). If the URL is not provided,
    # developers must set the key and rating_key attributes manually and provide a valid
    # callback for full metadata objects.
    #
    empty = ('', None)

    # If the metadata object has a 'file' attribute, synthesize a MediaObject and PartObject for it.
    if hasattr(self, 'file') and self.file:
      # Only allow agents to do this.
      assert self._core.plugin_class == 'Agent'

      def check(attr):
        if getattr(self, attr, None):
          raise AttributeError("Conflict with 'file' attribute", attr)

      check('url')
      check('key')
      check('rating_key')

      if len(self.items):
        raise AttributeError("The 'file' attribute cannot be provided for objects with a media & part hierarchy.")

      self.add(self._sandbox.environment[MediaObject.__name__](
        parts=[self._sandbox.environment[PartObject.__name__](file=self.file)]
      ))
      self.file = None

      has_part_with_file = True

    else:
      has_part_with_file = len(self.items) and len(self.items[0].parts) and self.items[0].parts[0].file

    if self._template.require_key_and_rating_key:
      if ((hasattr(self, 'url') and self.url not in empty) or \
          (hasattr(self, 'key') and self.key not in empty and hasattr(self, 'rating_key') and self.rating_key not in empty)) == False and not \
          (Framework.code.context.flags.indirect in self._context.flags or self._context.request.uri.startswith('/:/plugins/')):

        if not has_part_with_file:
          raise Framework.exceptions.AttributeException('If no URL is provided, the key and rating_key attributes must be set.')


    # When sending an indirect response, check for an empty key attribute and try to set it.
    # Missing keys make the LGTV unhappy.
    if Framework.code.context.flags.indirect in self._context.flags and self.key in empty and len(self.items) > 0 and len(self.items[0].parts) > 0:
      self.key = self.items[0].parts[0].key


    # If no source icon is defined, use a hosted resource with the current sandbox's identifier.
    if self.source_icon == None and not (hasattr(self._template, 'suppress_source_icon') and self._template.suppress_source_icon):
      self.source_icon = self._core.runtime.hosted_resource_url('image', 'source', self._sandbox.identifier)

    
    el = Framework.modelling.objects.ModelInterfaceObject._to_xml(self)
    
    # If the URL attribute is defined, add some URL Service magic where required
    if hasattr(self, 'url') and self.url != None:
      url = self._attributes['url']
      
      lookup_key = self._core.services.lookup_url_for_media_url(url, syncable = Framework.code.context.flags.syncable in self._core.sandbox.context.flags)
      
      # If a key isn't set, synthesize one using the URL service lookup method
      if hasattr(self, 'key') and self.key == None:
        el.set('key', lookup_key)
        
      # If a ratingKey isn't set, use the URL
      if hasattr(self, 'rating_key') == False or (hasattr(self, 'rating_key') and self.rating_key == None):
        el.set('ratingKey', self.url)
      
      # If there are no child objects, and this class expects children, add them by calling the URL service
      if len(self._objects) == 0 and len(type(self)._child_types) > 0:
        
        # If the media objects function flagged as deferred, set the deferred attribute on the metadata object
        service = self._core.services.service_for_url(url)
        if self._core.services.function_in_service_is_deferred(Framework.components.services.MEDIA_OBJECTS_FUNCTION_NAME, service):
          self.deferred = True
          
        # If the item is deferred, add a synthetic item for older clients
        if self.deferred:
          el.set('deferred', '1')
          self._append_children(
            el,
            [self._sandbox.environment[MediaObject.__name__](
              parts = [
                self._sandbox.environment[PartObject.__name__](key = indirect_callback_string(lookup_key + "&indirect=1&limit=1"))
              ]
            )]
          )
          
        # If the item is not deferred, add its children normally
        else:
          items = self._core.services.media_objects_for_url(url, allow_deferred=True, metadata_class=type(self))
          if items:
            self._append_children(el, items)

    elif has_part_with_file:
      part = self.items[0].parts[0]
      file_url = urlparse.urljoin('file:', urllib.pathname2url(part.file))
      el.set('ratingKey', file_url)
          
          
    # If this object supports setting HTTP headers, it's a non-media object - apply HTTP headers and POST callback info
    if 'http_headers' in self._attribute_list:
      ObjectWithHTTPHeaders._apply_http_headers(self, el)
      ObjectWithPOSTCallback._apply_post_url_and_headers(self.key, el)
        
    return el
    
    
class ObjectContainer(Framework.modelling.objects.ModelInterfaceObjectContainer):
  
  _attribute_list = [
    'view_group',
    'content',
    'art',
    'title_bar',
    'title1',
    'title2',
    'user_agent',
    'http_cookies',
    'no_history',
    'replace_parent',
    'mixed_parents',
    'no_cache',
    'header',
    'message',
    'http_headers',
    'post_headers',
    'identifier',
    'shuffle_key',
    'source_title'
  ]
  
  def _set_attribute(self, el, name, value):
    if name == 'view_group':
      grp = self._core.runtime.view_groups[value]
      if grp.viewMode: el.set("viewmode", str(grp.viewMode))
      if grp.mediaType: el.set("contenttype", str(grp.mediaType))
      if grp.viewType: el.set("viewType", str(grp.viewType))
      if grp.viewMenu != None:
        if grp.viewMenu:
          el.set("viewMenu", "1")
        else:
          el.set("viewMenu", "0")
      if grp.viewThumb != None:
        if grp.viewThumb:
          el.set("viewThumb", "1")
        else:
          el.set("viewThumb", "0")
      if grp.viewCols != None: el.set("viewCols", str(grp.viewCols))
      if grp.viewRows != None: el.set("viewRows", str(grp.viewRows))
      if grp.viewSummary != None: el.set("viewSummary", str(grp.viewSummary))
    
    elif name == 'content':
      Framework.modelling.objects.ModelInterfaceObjectContainer._set_attribute(self, el, "viewGroup", value)

    else:
      Framework.modelling.objects.ModelInterfaceObjectContainer._set_attribute(self, el, name, value)
      
      
  def _to_xml(self):
    el = Framework.modelling.objects.ModelInterfaceObjectContainer._to_xml(self)
    if not (hasattr(self, 'identifier') and self.identifier != None):
      Framework.modelling.objects.ModelInterfaceObjectContainer._set_attribute(self, el, "identifier", self._core.identifier)

    # If a source title isn't provided, find the first prefix handler and use its name.
    if not (hasattr(self, 'source_title') and self.source_title != None) and self._core.identifier != 'com.plexapp.system':
      handlers = [x for x in self._core.runtime._handlers if isinstance(x, Framework.handlers.PrefixRequestHandler)]
      if len(handlers) > 0:
        Framework.modelling.objects.ModelInterfaceObjectContainer._set_attribute(self, el, "sourceTitle", handlers[0].name)
    
    # Set the prefix & version for media flags
    Framework.modelling.objects.ModelInterfaceObjectContainer._set_attribute(self, el, "mediaTagPrefix", "/system/bundle/media/flags/")
    sub_file_path = self._core.storage.join_path(self._core.app_support_path, "Plug-ins", "Media-Flags.bundle", "Contents", "Resources", "substitutions.xml")
    if self._core.storage.file_exists(sub_file_path):
      last_modified = self._core.storage.last_modified(sub_file_path)
      Framework.modelling.objects.ModelInterfaceObjectContainer._set_attribute(self, el, "mediaTagVersion", str(int(last_modified)))
    
    # Set default HTTP headers
    ObjectWithHTTPHeaders._apply_http_headers(self, el)
    
    # Set default headers to use for requests for POST callback data
    ObjectWithPOSTCallback._apply_post_headers(self, el)

    # If returning a container from the core sandbox, add prefs & search
    # attributes.
    #
    if self._sandbox.identifier == self._core.sandbox.identifier:
      services = self._core.services.get_all_services(self._sandbox.identifier)

      if self._core.prefs_available(self._sandbox.identifier, services=services):
        el.set("prefsKey", "/:/plugins/%s/prefs" % self._sandbox.identifier)

      searches = [service for service in services if self._core.services._type_for_service(service) == 'search']
      if len(searches) > 0:
        el.set("searchesKey", "/system/services/searches?identifier=%s" % self._sandbox.identifier)

    return el
      

class LegacyObjectKit(BaseKit):

  _root_object = False

  _excluded_policies = [
    Framework.policies.ModernPolicy,
  ]

  _children = [
      Framework.objects.XMLObject,
      Framework.objects.XMLContainer,
      Framework.objects.MediaContainer,
      Framework.objects.MessageContainer,
      Framework.objects.DirectoryItem,
      Framework.objects.PopupDirectoryItem,
      Framework.objects.SearchDirectoryItem,
      Framework.objects.InputDirectoryItem,
      Framework.objects.VideoItem,
      Framework.objects.WebVideoItem,
      Framework.objects.RTMPVideoItem,
      Framework.objects.WindowsMediaVideoItem,
      Framework.objects.PhotoItem,
      Framework.objects.TrackItem,
      Framework.objects.Function,
      Framework.objects.IndirectFunction,
      Framework.objects.PrefsItem,
      Framework.objects.ContextMenu,
      Framework.objects.MetadataSearchResult,
  ]



class ObjectKit(BaseKit):

  _root_object = False

  _included_policies = [
    Framework.policies.CodePolicy,
  ]

  _children = [
    LegacyObjectKit,
    Framework.objects.Redirect,
    Framework.objects.DataObject,
  ]


  def _publish_model(self, model, media=True, name=None, **kwargs):
    self._publish(cls, name=name, **kwargs)


  def _generate_class(self, base, media=True, name=None, enable_attribute_synthesis=True, child_types=None, **kwargs):
    # Generate classes for non-model types.
    if Framework.modelling.model.Model not in base.__mro__:
      cls = generate_class(base, self._sandbox, child_types=child_types)

    # Generate interface classes for model types.
    else:

      # Synthesize a class name if one wasn't provided.
      if name == None: name = filter(lambda c: c != '_', base.__name__) + 'Object'

      if child_types is None:
        child_types = []
        
      # If this is a media-containing object, allow MediaObject child items
      if media:
        child_types.append(MediaObject)
      
      # Add extra attributes for URL services
      attribute_list = ['url', 'http_cookies', 'user_agent', 'source_icon']

      if self._core.plugin_class == 'Agent':
        attribute_list.append('file')
      
      # Add the http_headers attribute if this is a non-media object
      if not media:
        attribute_list.append('http_headers')
      
      # Add the deferred attribute if this is a media object
      else:
        attribute_list.append('deferred')
      
      cls = generate_model_interface_class(self._sandbox, base, superclass=MetadataObject, attribute_list = attribute_list, child_types = child_types, children_attr_name = 'items')
      cls.__name__ = name

    # Synthesize hosted attributes.
    try:
      if enable_attribute_synthesis and self._sandbox.identifier != 'com.plexapp.system' and self._core.runtime.hash_for_hosted_resource('image', 'source'):
        cls.thumb = self._core.runtime.hosted_resource_url('image', 'source')
    except:
      pass
    
    # Synthesize default attributes.
    if self._sandbox.policy.synthesize_defaults:
      try:
        cls.thumb = self._core.runtime.external_resource_path(self._core.icon_resource_name)
      except:
        pass

    return cls


  def _generate_container(self, *args, **kwargs):
    cls = generate_model_interface_container_class(*args, **kwargs)

    # Synthesize hosted attributes.
    try:
      if self._sandbox.identifier != 'com.plexapp.system' and self._core.runtime.hash_for_hosted_resource('image', 'art'):
        cls.art = self._core.runtime.hosted_resource_url('image', 'art')
    except:
      pass
    
    # Synthesize default attributes.
    if self._core.policy.synthesize_defaults:
      cls.title1 = self._core.title
      cls.art = self._core.runtime.external_resource_path(self._core.art_resource_name)
      cls.title_bar = self._core.runtime.external_resource_path(self._core.title_bar_resource_name)

    return cls
  

  def _web_video_url(self, url):
    prefix = self._core.runtime.current_prefix
    return WebkitURL("plex://127.0.0.1/video/:/webkit?url=%s&prefix=%s" % (urllib.quote_plus(url), prefix))
    
    
  def _windows_media_video_url(self, url, width=None, height=None):
    final_url = "http://www.plexapp.com/player/silverlight.php" + \
      "?stream=" + urllib.quote(url)
    if width:
      final_url += "&width=" + str(width)
    if height:
      final_url += "&height=" + str(height)
    
    return self._web_video_url(final_url)
    
  
  def _embed_url(self, url):
    if isinstance(url, indirect_callback_string):
      return IndirectEmbedURL(url)
    elif isinstance(url, callback_string):
      return CallbackEmbedURL(url)
    else:
      return EmbedURL(url)
    
    
  def _http_live_stream_url(self, url):
    # If this is a callback string, ensure it has a .m3u8 extension.
    if isinstance(url, callback_string):
      pos = url.find('?')
      if pos >= 0:
        path = url[:pos]
        query = url[pos:]
      else:
        path = url
        query = None
      
      # Make sure we don't have an extension already.
      if path.split('/')[-1].find('.') == -1:
        path += '.m3u8'
        post_url = url.post_url if hasattr(url, 'post_url') else None
        post_headers = url.post_headers if hasattr(url, 'post_headers') else {}
          
        url = type(url)(path if query == None else path + query)
        url.post_url = post_url
        url.post_headers = post_headers

      hls_url = IndirectHLSURL(url) if isinstance(url, indirect_callback_string) else CallbackHLSURL(url)
      hls_url.post_url = url.post_url
      hls_url.post_headers = url.post_headers
      return hls_url


    else:    
      return HLSURL(url)


  def _rtmp_video_url(self, url, clip=None, clips=None, width=None, height=None, live=False, swf_url=None, app=None, args=None, **kwargs):
    # Throw exceptions if unsupported arguments are used.
    if clips:
      raise Framework.exceptions.FrameworkException("The 'clips' argument is not supported.")
    if width:
      self._core.log.debug("The 'width' argument has no effect.")
    if height:
      self._core.log.debug("The 'height' argument has no effect.")

    ret = url
    if clip:
      ret += " playpath=%s" % clip

    if swf_url:
      ret += " swfUrl=%s swfVfy=1" % swf_url

    if live:
      ret += " live=1"

    if app:
      ret += " app=%s" % app

    if args:
      def make_arg(value, name=''):
        string_value, string_type = (None, None)
        if isinstance(value, bool):
          string_value, string_type = ('1' if value else '0', 'B')
        elif isinstance(value, (int, float)):
          string_value, string_type = (str(value), 'N')
        elif isinstance(value, basestring):
          string_value, string_type = (str(value), 'S')
        elif value == None:
          string_value, string_type = ('(null)', 'Z')

        if string_value and string_type:
          if name:
            name += ':'
            string_type = 'N' + string_type

          return 'conn=%s:%s%s' % (string_type, name, string_value)

        if isinstance(value, dict):
          return 'conn=O:1 ' + ' '.join([make_arg(v,k) for k,v in value.items()]) + ' conn=O:0'

        return ''

      ret += ' ' + ' '.join([make_arg(arg) for arg in args])

    if kwargs and len(kwargs) > 0:
      for key, value in kwargs.items():
        ret += ' %s=%s' % (key, value)

    if isinstance(url, callback_string):
      rtmp_url = IndirectRTMPURL(ret) if isinstance(url, indirect_callback_string) else CallbackRTMPURL(ret)
      rtmp_url.post_url = url.post_url
      rtmp_url.post_headers = url.post_headers
      return rtmp_url

    else:
      return RTMPURL(ret)


  def _indirect_response_generator(self, container_cls, item_cls, part_cls):
    def _indirect_response(cls, key, url=None, metadata_key=None, rating_key=None, metadata_kwargs={}, **kwargs):
      return container_cls(
        objects=[
          cls(
            key = metadata_key,
            rating_key = rating_key,
            url = url,
            items = [
              item_cls(
                parts = [
                  part_cls(key = key)
                ]
              )
            ],
            **metadata_kwargs
          )
        ],
        **kwargs
      )
    return _indirect_response
    
  
  def _init(self):
    access_point = self._core._metadata_model_accessor.get_access_point(self._core.identifier)
    
    # Object classes
    container_class = self._generate_container(
      self._sandbox,
      'MediaContainer',
      superclass = ObjectContainer,
      child_types = [
        Framework.modelling.objects.ModelInterfaceObject, 
        ProviderObject,
        DirectoryObject,
        NextPageObject,
        InputDirectoryObject,
        PopupDirectoryObject,
        PrefsObject,
        MetadataItem,
        SearchResult,
      ])

    media_class = self._generate_class(MediaObject)
    part_class = self._generate_class(PartObject)

    self._publish(container_class, name = 'ObjectContainer')
    self._publish(media_class, name='MediaObject')
    self._publish(part_class, name='PartObject')

    self._publish(self._generate_class(VideoStreamObject))
    self._publish(self._generate_class(AudioStreamObject))

    self._publish(self._generate_class(ProviderObject))
    self._publish(self._generate_class(DirectoryObject))
    self._publish(self._generate_class(NextPageObject))
    self._publish(self._generate_class(PlaylistObject))
    self._publish(self._generate_class(InputDirectoryObject))
    self._publish(self._generate_class(PopupDirectoryObject), excluded_policies=[Framework.policies.ModernPolicy])
    self._publish(self._generate_class(PrefsObject), excluded_policies=[Framework.policies.ModernPolicy])
    self._publish(self._generate_class(SearchDirectoryObject))
    self._publish(self._generate_class(MetadataItem, enable_attribute_synthesis=False, child_types=[MetadataItem]))
    self._publish(self._generate_class(SearchResult, enable_attribute_synthesis=False, child_types=[SearchResult]))

    self._publish(self._generate_class(access_point.Movie))
    self._publish(self._generate_class(access_point.VideoClip))
    self._publish(self._generate_class(access_point.Episode))
    self._publish(self._generate_class(access_point.Season, media=False))
    self._publish(self._generate_class(access_point.TV_Show, media=False))
    self._publish(self._generate_class(access_point.LegacyArtist, media=False, name='ArtistObject'))
    self._publish(self._generate_class(access_point.LegacyAlbum, media=False, name='AlbumObject'))
    self._publish(self._generate_class(access_point.Track))
    self._publish(self._generate_class(access_point.Photo))
    self._publish(self._generate_class(access_point.PhotoAlbum, media=False))

    # Extras
    self._publish(self._generate_class(access_point.Trailer))
    self._publish(self._generate_class(access_point.Interview))
    self._publish(self._generate_class(access_point.DeletedScene))
    self._publish(self._generate_class(access_point.BehindTheScenes))
    self._publish(self._generate_class(access_point.SceneOrSample))
    self._publish(self._generate_class(access_point.Featurette))
    self._publish(self._generate_class(access_point.Short))
    self._publish(self._generate_class(access_point.Other))
    self._publish(self._generate_class(access_point.MusicVideo))
    self._publish(self._generate_class(access_point.LiveMusicVideo))
    self._publish(self._generate_class(access_point.LyricMusicVideo))
    self._publish(self._generate_class(access_point.ConcertVideo))

    # Convenience functions.
    self._publish(self._web_video_url, name='WebVideoURL')
    self._publish(self._rtmp_video_url, name='RTMPVideoURL')
    self._publish(self._windows_media_video_url, name='WindowsMediaVideoURL')
    self._publish(self._http_live_stream_url, name='HTTPLiveStreamURL')
    self._publish(self._embed_url, name='EmbedURL')

    self._publish(self._indirect_response_generator(container_class, media_class, part_class), name='IndirectResponse')

