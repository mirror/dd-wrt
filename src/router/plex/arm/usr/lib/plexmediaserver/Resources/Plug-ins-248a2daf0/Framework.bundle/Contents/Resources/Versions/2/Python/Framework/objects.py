#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import types, base64, urllib, operator, cerealizer

class ObjectFactory(object):
  def __init__(self, core, object_class):
    self._object_class = object_class
    self._core = core
    self._attributes = {}
    
  def __call__(self, *args, **kwargs):
    for name in self._attributes:
      if name not in kwargs or kwargs[name] == None:
        kwargs[name] = self._attributes[name]
    return self._object_class(self._core, *args, **kwargs)

  def __setattr__(self, name, value):
    if name[0] == '_':
      return object.__setattr__(self, name, value)
    else:
      self._attributes[name] = value
    
  def __getattr__(self, name):
    if name[0] != '_' and name in self._attributes:
      return self._attributes[name]
    return self.__getattribute__(name)
    
  def is_instance(self, obj):
    return isinstance(obj, self._object_class)
    
class Object(Framework.Serializable):
  def __init__(self, core, **kwargs):
    self._core = core
    
    # Add all class attributes that aren't private or functions
    for key in self.__class__.__dict__:
      if key[0] != "_" and self.__class__.__dict__[key].__class__.__name__ != "function":
        self.__dict__[key] = self.__class__.__dict__[key]
    self.__headers = {}

    # Add all supplied argument attributes, unless the attribute is None and would overwrite an existing attribute
    for key in kwargs:
      if not (self.__dict__.has_key(key) and kwargs[key] == None):
        self.__dict__[key] = kwargs[key]
        
  def _bind(self, core):
    self._core = core
    
  def _release(self):
    self._core = None

  def Content(self):
    return None

  def Status(self):
    return 200

  def Headers(self):
    return self.__headers
    
  def SetHeader(self, name, value):
    self.__headers[name] = value

  def __repr__(self):
      keys = self.__dict__.keys()
      repr_str = self.__class__.__name__ + "(%s)"
      arg_str = ""
      for i in range(len(keys)):
        key = keys[i]
        obj = self.__dict__[key]
        if key[0] != "_" and key != "tagName":
          if arg_str != "": arg_str += ", "
          if type(obj).__name__ == "function":
            arg_str += key + "=" + str(obj.__name__)
          else:
            arg_str += key + "=" + repr(obj)
      return repr_str % arg_str
        
        
class Container(Object):

  def __init__(self, core, **kwargs):
    Object.__init__(self, core, **kwargs)
    self.__items__ = []
    
  def _bind(self, core):
    Object._bind(self, core)
    for item in self.__items__:
      item._bind(core)
    
  def _release(self):
    Object._release(self)
    for item in self.__items__:
      item._release()
    
  def __iter__(self):
    return iter(self.__items__)
    
  def __len__(self):
    return len(self.__items__)
    
  def __getitem__(self, key):
    return self.__items__[key]
    
  def __setitem__(self, key, value):
    self.__items__[key] = value
    
  def __delitem__(self, key):
    del self.__items__[key]
    
  def Append(self, obj):
    if obj != None:
      return self.__items__.append(obj)

  def Count(self, x):
    return self.__items__.count(x)
    
  def Index(self, x):
    return self.__items__.index(x)
  
  def Extend(self, x):
    return self.__items__.extend(x)
    
  def Insert(self, i, x):
    if i != None:
      return self.__items__.insert(i, x)
    
  def Pop(self, i):
    return self.__items__.pop(i)
  
  def Remove(self, x):
    return self.__items__.remove(x)
  
  def Reverse(self):
    self.__items__.reverse()
  
  def Sort(self, attr, descending=False):
    self.__items__.sort(key=operator.attrgetter(attr))
    if descending:
      self.__items__.reverse()
      
  def Clear(self):
    self.__items__ = []
      
    
class XMLObject(Object):
  def __init__(self, core, **kwargs):
    self.tagName = self.__class__.__name__
    Object.__init__(self, core, **kwargs)
    self.SetHeader("Content-Type", "application/xml")
    
  def SetTagName(self, tagName):
    self.tagName = tagName
  
  def ToElement(self):
    el = self._core.data.xml.element(self.tagName)
    for key in self.__dict__:
      if key[0] != "_" and self.__dict__[key].__class__.__name__ != "function" and key != "tagName" and self.__dict__[key] != None:
        value = self.__dict__[key]
        if key == "ratingKey":
          el.set(key, urllib.quote("b64://"+base64.urlsafe_b64encode(self.__dict__[key])))
        elif value == True:
          el.set(key, "1")
        elif value == False:
          el.set(key, "0")
        elif not (isinstance(self.__dict__[key], list) or isinstance(self.__dict__[key], dict)):
          value = self.__dict__[key]
          el.set(key, unicode(value))
    return el
        
  def Content(self):
    return self._core.data.xml.to_string(self.ToElement())
    
  def __str__(self):
    if "key" in self.__dict__ and self.key is not None:
      return self.key
    else:
      return repr(self)
      
class XMLContainer(XMLObject, Container):
  def __init__(self, core, **kwargs):
    self.tagName = self.__class__.__name__
    Container.__init__(self, core, **kwargs)
    self.SetHeader("Content-Type", "application/xml")

  def ToElement(self):
    root = XMLObject.ToElement(self)
    for item in self.__items__:
      if isinstance(item, XMLObject):
        el = item.ToElement()
        if el != None:
          root.append(el)
    return root
    

class DataObject(Object):
  def __init__(self, core, data, contentType):
    Object.__init__(self, core, data=data, contentType=contentType)
    self.SetHeader("Content-Type", contentType)

  def Content(self):
    return self.data

class ItemInfoRecord(object):
  def __init__(self):
    self.title1 = None
    self.title2 = None
    self.art = None
    self.thumb = None
    self.itemTitle = None

class ContextMenu(XMLContainer):
  def __init__(self, core, **kwargs):
    XMLContainer.__init__(self, core, **kwargs)

  def ToElement(self, sender, contextKey, **kwargs):
    root = XMLObject.ToElement(self)
    for item in self.__items__:
      if isinstance(item, Function):
        item.AddFunctionArguments(sender=sender, key=contextKey, **kwargs)
        el = item.ToElement()
        if el != None:
          root.append(el)
    return root

class MediaContainer(XMLContainer):

  def __init__(self, core, art=None, viewGroup=None, title1=None, title2=None, noHistory=False, replaceParent=False, disabledViewModes=None, **kwargs):
    XMLContainer.__init__(self, core, art=art, title1=title1, title2=title2, noHistory=noHistory, replaceParent=replaceParent, **kwargs)
    
    if viewGroup is not None:
      if viewGroup in self._core.runtime.view_groups:
        self.viewGroup = viewGroup
      else:
        self._core.log.error("(Framework) Couldn't assign view group '%s' to a MediaContainer - group doesn't exist" % viewGroup)
        pass
        
    if type(disabledViewModes) == list:
      dvString = ""
      for view in disabledViewModes:
        if view in Framework.components.runtime.view_modes:
          if len(dvString) > 0: dvString += ","
          dvString += str(Framework.components.runtime.view_modes[view])
      self.disabledViewModes = dvString

  def __valueOrNone(self, key):
    if self.__dict__.has_key(key):
      return str(self.__dict__[key])
    else:
      return None

  def ToElement(self):
    if self.__dict__.has_key("contextMenu"):
      __containerContextMenu = self.contextMenu
      self.contextMenu = None
    else:
      __containerContextMenu = None
    root = XMLObject.ToElement(self)
    for item in self.__items__:
      if item.__dict__.has_key("contextMenu"):
        __itemContextMenu = item.contextMenu
        item.contextMenu = None
      else:
        __itemContextMenu = None

      if item.__dict__.has_key("contextKey"):
        __itemContextKey = item.contextKey
        item.contextKey = None
      else:
        __itemContextKey = None

      if item.__dict__.has_key("contextArgs"):
        __itemContextArgs = item.contextArgs
        item.contextArgs = None
      else:
        __itemContextArgs = None


      if isinstance(item, XMLObject):
        info = ItemInfoRecord()
        info.title1 = self.__valueOrNone("title1")
        info.title2 = self.__valueOrNone("title2")
        info.art = self.__valueOrNone("art")
        if item.__dict__.has_key("title"):
          info.itemTitle = item.title
        elif item.__dict__.has_key("name"):
          info.itemTitle = item.name
          
        
        if 'art' in item.__dict__ and item.art != None:
          info.art = str(item.art)
        
        if 'thumb' in item.__dict__ and item.thumb != None:
          info.thumb = str(item.thumb)
        
        # Add sender information to functions
        def add_sender_arg(obj):
          if isinstance(obj, Function):
            obj.AddFunctionArguments(sender=info)
          elif isinstance(obj, list):
            for x in obj:
              add_sender_arg(x)
          if hasattr(obj, '__items__'):
            add_sender_arg(obj.__items__)
          if hasattr(obj, '__dict__'):
            if 'key' in obj.__dict__:
              add_sender_arg(obj.key)
            if 'items' in obj.__dict__:
              add_sender_arg(obj.items)
            if 'parts' in obj.__dict__:
              add_sender_arg(obj.parts)
      
        add_sender_arg(item)
        
        __itemElement = item.ToElement()

        # Check if there's a context menu
        if __itemContextKey:
          __contextMenu = None
          if __itemContextMenu: __contextMenu = __itemContextMenu
          elif __containerContextMenu: __contextMenu = __containerContextMenu
          if __contextMenu:
            __itemElement.append(__contextMenu.ToElement(info, __itemContextKey, **__itemContextArgs))

        root.append(__itemElement)

      item.contextMenu = __itemContextMenu
      item.contextKey = __itemContextKey
      item.contextArgs = __itemContextArgs

    root.set("size", str(len(self)))
    
    if hasattr(self, "viewGroup"):
      grp = self._core.runtime.view_groups[self.viewGroup]
      if grp.viewMode: root.set("viewmode", str(grp.viewMode))
      if grp.mediaType: root.set("contenttype", str(grp.mediaType))
      if grp.viewType: root.set("viewType", str(grp.viewType))
      if grp.viewMenu != None:
        if grp.viewMenu:
          root.set("viewMenu", "1")
        else:
          root.set("viewMenu", "0")
      if grp.viewThumb != None:
        if grp.viewThumb:
          root.set("viewThumb", "1")
        else:
          root.set("viewThumb", "0")
      if grp.viewCols != None: root.set("viewCols", str(grp.viewCols))
      if grp.viewRows != None: root.set("viewRows", str(grp.viewRows))
      if grp.viewSummary != None: root.set("viewSummary", str(grp.viewSummary))
      
      
    root.set("identifier", str(self._core.identifier))

    self.contextMenu = __containerContextMenu
    return root
    
    
  def __getitem__(self, item):
    
    # If requesting a slice, construct a new container with the specified items
    if isinstance(item, slice):
      
      # Create a new container
      c = MediaContainer(self._core)
      
      # Copy all container attributes from old to new
      for key in self.__dict__:
        if key[0] != "_" and self.__dict__[key].__class__.__name__ != "function" and key != "tagName" and self.__dict__[key] != None:
          value = self.__dict__[key]
          c.__dict__[key] = value
      
      # Get the items specified by the slice, add them to the new container and return it
      c.__items__ = XMLContainer.__getitem__(self, item)
      return c
    
    # Otherwise, return a single item
    else:
      return XMLContainer.__getitem__(self, item)
         

class MessageContainer(XMLContainer):
  def __init__(self, core, header, message, title1=None, title2=None, **kwargs):
    XMLContainer.__init__(self, core, header=header, message=message, title1=title1, title2=title2, noCache=True, **kwargs)
    self.tagName = "MediaContainer"
    

####################################################################################################

class DirectoryItem(XMLObject):
  def __init__(self, core, key, title, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    #TODO: Remove 'name' attribute when appropriate - left in now for backwards compatibility with clients
    XMLObject.__init__(self, core, key=key, title=title, name=title, subtitle=subtitle, summary=summary, thumb=thumb, art=art, **kwargs)
    self.tagName = "Directory"

####################################################################################################

class PopupDirectoryItem(XMLObject):
  def __init__(self, core, key, title, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, core, key=key, name=title, subtitle=subtitle, summary=summary, thumb=thumb, popup=True, art=art, **kwargs)
    self.tagName = "Directory"

####################################################################################################

class InputDirectoryItem(XMLObject):
  def __init__(self, core, key, title, prompt, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, core, key=key, name=title, subtitle=subtitle, summary=summary, thumb=thumb, search=True, prompt=prompt, art=art, **kwargs)
    self.tagName = "Directory"

####################################################################################################

class SearchDirectoryItem(InputDirectoryItem):
  def __init__(self, core, key, title, prompt, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    InputDirectoryItem.__init__(self, core, key=key, title=title, prompt=prompt, subtitle=subtitle, summary=summary, thumb=thumb, art=art, **kwargs)
    self._core.log.warning("(Framework) WARNING: SearchDirectoryItem is deprecated. Use InputDirectoryItem instead.")
    
####################################################################################################

class VideoItem(XMLContainer):
  def __init__(self, core, key=None, title=None, subtitle=None, summary=None, duration=None, thumb=None, art=None, type='clip', items=[], **kwargs):
    XMLContainer.__init__(self, core, key=key, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, type=type, **kwargs)
    for item in items:
      self.Append(item)
    self.tagName = "Video"
    
  def Append(self, item):
    # If appending a new-style MediaItem to an object with an old-style 'key' attribute, convert
    # the key to a real MediaItem and MediaPart before continuing. Alternatively, if we're adding
    # a new-style MediaItem, use it's first part as the key for old clients to use
    if len(self) == 0:
      if self.key != None:
        new_item = MediaItem(self._core, [MediaPart(self._core, self.key)])
        XMLContainer.Append(self, new_item)
      else:
        self.key = item[0].key
    XMLContainer.Append(self, item)
    
  def ToElement(self):
    root = XMLObject.ToElement(self)
    for item in self.__items__:
      if isinstance(item, MediaItem):
        el = item.ToElement()
        if el != None:
          root.append(el)
    return root
    
####################################################################################################

class WebVideoItem(XMLObject):
  def __init__(self, core, url, title=None, subtitle=None, summary=None, duration=None, thumb=None, art=None, **kwargs):
    prefix = core.runtime.current_prefix
    
    if isinstance(url, basestring):
      key = "plex://127.0.0.1/video/:/webkit?url=%s&prefix=%s" % (urllib.quote_plus(url), prefix)
    else:
      key = url
    XMLObject.__init__(self, core, key=key, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, **kwargs)
    self.tagName = "Video"
    
####################################################################################################

class RTMPVideoItem(WebVideoItem):
  def __init__(self, core, url, clip=None, clips=None, width=None, height=None, live=False, title=None, subtitle=None, summary=None, duration=None, thumb=None, art=None, **kwargs):
    if isinstance(url, basestring):
      final_url = "http://www.plexapp.com/player/player.php" + \
        "?url=" + urllib.quote(url) + \
        "&live="
      if live:
        final_url += "true"
      else:
        final_url += "false"
      if clip:
        final_url += "&clip=" + urllib.quote(clip)
      if clips:
        for c in clips:
          final_url += "&clip[]=" + str(c)
      if width:
        final_url += "&width=" + str(width)
      if height:
        final_url += "&height=" + str(height)
  
    else:
      final_url = url
    WebVideoItem.__init__(self, core, final_url, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, **kwargs)

####################################################################################################

class WindowsMediaVideoItem(WebVideoItem):
  def __init__(self, core, url, width=None, height=None, title=None, subtitle=None, summary=None, duration=None, thumb=None, art=None, **kwargs):
    if isinstance(url, basestring):
      final_url = "http://www.plexapp.com/player/silverlight.php" + \
        "?stream=" + urllib.quote(url)
      if width:
        final_url += "&width=" + str(width)
      if height:
        final_url += "&height=" + str(height)
    else:
      final_url = url
    WebVideoItem.__init__(self, core, final_url, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, **kwargs)

####################################################################################################

class PhotoItem(XMLObject):
  def __init__(self, core, key, title, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, core, key=key, title=title, subtitle=subtitle, summary=summary, thumb=thumb, art=art, **kwargs)
    self.tagName = "Photo"

####################################################################################################

class TrackItem(XMLObject):
  def __init__(self, core, key, title, artist=None, album=None, index=None, rating=None, duration=None, size=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, core, key=key, track=title, artist=artist, album=album, index=index, rating=rating, totalTime=duration, size=size, thumb=thumb, art=art, **kwargs)
    self.tagName = "Track"
    
####################################################################################################

class Function(XMLObject):
  def __init__(self, core, obj, ext=None, **kwargs):
    XMLObject.__init__(self, core)

    if type(obj) == types.FunctionType:
      self.key = obj
    else:
      # Copy all attributes from the object to the function item
      for key in obj.__dict__:
        value = getattr(obj, key)
        if key[0] != "_" and type(value) != types.FunctionType and value != None:
          setattr(self, key, value)
      self.__obj = obj

    if ext is None:
      self.__ext = ""
    else:
      self.__ext = ".%s" % ext
    
    self.__kwargs = {}
    for kwarg in kwargs:
      if kwarg != "ext":
        self.__kwargs[kwarg] = kwargs[kwarg]
    
  def AddFunctionArguments(self, **kwargs):
    for key in kwargs:
      self.__kwargs[key] = kwargs[key]
  
  def ToElement(self):
    # Modify the key to call a function with the given kwargs
    queryString = self._core.runtime.create_query_string(self.__kwargs)
    if queryString and len(queryString) > 0: queryString = '?' + queryString
    else: queryString = ''
    
    self.key = "%s/:/function/%s%s%s" % (self._core.runtime.current_prefix, self.__obj.key.__name__, self.__ext, queryString)
    return XMLObject.ToElement(self)
    
  # Allow returning just the function path
  def __str__(self):
    if hasattr(self, 'key'):
      if type(self.key) == types.FunctionType:
        queryString = self._core.runtime.create_query_string(self.__kwargs)
        if queryString and len(queryString) > 0: queryString = '?' + queryString
        else: queryString = ''
        s = "%s/:/function/%s%s%s" % (self._core.runtime.current_prefix, self.key.__name__, self.__ext, queryString)
        return s
    return XMLObject.__str__(self)
    
  def __repr__(self):
    s = 'Function(%s' % (repr(self.__obj))
    for key, value in self.__kwargs.iteritems():
      s += ', %s=%s' % (key, repr(value))
    s += ')'
    return s

####################################################################################################

class IndirectFunction(Function):
  def __str__(self):
    return Function.__str__(self) + '&indirect=1'
  
####################################################################################################

class PrefsItem(DirectoryItem):
  def __init__(self, core, title=None, subtitle=None, summary=None, thumb=None, **kwargs):
    DirectoryItem.__init__(self, core, "%s/:/prefs" % core.runtime.current_prefix, title=title, subtitle=subtitle, summary=summary, thumb=thumb, settings=True, **kwargs)

####################################################################################################

class Redirect(Object): 
  def __init__(self, core, url, temporary=True):
    Object.__init__(self, core)
    self._temporary = temporary

    # Try to use an absolute URL for the Location.
    location = str(url)
    try:
      request = core.sandbox.context.request
      if request and request.host and location[0] == "/":
        location = request.protocol + "://" + request.host + location
    except:
      pass

    self.SetHeader("Location", location)

  def Status(self):
    # Special case for CORS requests, since CORS requests requiring preflight
    # can't be redirected.
    try:
      request = self._core.sandbox.context.request
      if request and 'Origin' in request.headers and not request.host in request.headers['Origin']:
        return 200
    except:
      pass

    return 302 if self._temporary else 301
    
  def Content(self):
    return ''

  def __repr__(self):
    return 'Redirect(%s)' % (repr(self.__dict__['_Object__headers']['Location']))

####################################################################################################

class MetadataSearchResult(XMLObject):
  def __init__(self, core, id, name=None, year=None, score=0, lang=None, thumb=None):
    XMLObject.__init__(self, core, id=id, thumb=thumb, name=name, year=year, score=score, lang=lang)
    self.tagName = "SearchResult"

cerealizer.register(ItemInfoRecord)
