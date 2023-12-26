#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, Plugin, XML, String, Data, Hash, __objectManager as ObjectManager
import operator, random, urllib, types, cerealizer

testMode = False
testDefaults = {}

# Decorator to make function return immediatley
def test(f):
  def foo(*args, **kw):
    return
  if testMode:
    return f
  else:
    return foo

@test
def setTestDefaults(minElements=None, maxElements=None, recursePercent=None):
  if minElements != None:
    testDefaults['minElements'] = minElements
  if maxElements != None:
    testDefaults['maxElements'] = maxElements
  if recursePercent != None:
    testDefaults['recursePercent'] = recursePercent

####################################################################################################

class Object(object):

  def __init__(self, **kwargs):
    # Add all class attributes that aren't private or functions
    for key in self.__class__.__dict__:
      if key[0] != "_" and self.__class__.__dict__[key].__class__.__name__ != "function":
        self.__dict__[key] = self.__class__.__dict__[key]
    self.__headers = {}
        
    # Add all supplied argument attributes, unless the attribute is None and would overwrite an existing attribute
    for key in kwargs:
      if not (self.__dict__.has_key(key) and kwargs[key] == None):
        self.__dict__[key] = kwargs[key]
        
  def Content(self):
    return None
    
  def Status(self):
    return "200 OK"
    
  def Headers(self):
    headerString = ""
    for name in self.__headers:
      headerString += "%s: %s\r\n" % (name, self.__headers[name])
    return headerString
  
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

####################################################################################################

class Container(Object):

  def __init__(self, **kwargs):
    Object.__init__(self, **kwargs)
    self.__items__ = []
    
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
    return self.__items__.append(obj)

  def Count(self, x):
    return self.__items__.count(x)
    
  def Index(self, x):
    return self.__items__.index(x)
  
  def Extend(self, x):
    return self.__items__.extend(x)
    
  def Insert(self, i, x):
    return self.__items__.insert(i, x)
    
  def Pop(self, i):
    return self.__items__.pop(i)
  
  def Remove(self, x):
    return self.__items__.remove(x)
  
  def Reverse(self):
    self.__items__.reverse()
  
  def Sort(self, attr):
    self.__items__.sort(key=operator.attrgetter(attr))

####################################################################################################
 
class XMLObject(Object):
  def __init__(self, **kwargs):
    self.tagName = self.__class__.__name__
    Object.__init__(self, **kwargs)
    self.SetHeader("Content-Type", "application/xml")
    
  def SetTagName(self, tagName):
    self.tagName = tagName
  
  def ToElement(self):
    el = XML.Element(self.tagName)
    for key in self.__dict__:
      if key[0] != "_" and self.__dict__[key].__class__.__name__ != "function" and key != "tagName" and self.__dict__[key] != None:
        value = self.__dict__[key]
        if key == "ratingKey":
          el.set(key, String.Quote(String.Encode(unicode(self.__dict__[key]))))
        elif value == True:
          el.set(key, "1")
        elif value == False:
          el.set(key, "0")
        else:
          el.set(key, unicode(self.__dict__[key]))
    return el
        
  def Content(self):
    return XML.StringFromElement(self.ToElement())
    
  def __str__(self):
    if "key" in self.__dict__:
      return self.key
    else:
      return repr(self)

####################################################################################################

class XMLContainer(XMLObject, Container):
  def __init__(self, **kwargs):
    self.tagName = self.__class__.__name__
    Container.__init__(self, **kwargs)
    self.SetHeader("Content-Type", "application/xml")
  
  def ToElement(self):
    root = XMLObject.ToElement(self)
    for item in self.__items__:
      if ObjectManager.ObjectHasBase(item, XMLObject):
        root.append(item.ToElement())
    return root

####################################################################################################

class DataObject(Object):
  def __init__(self, data, contentType):
    Object.__init__(self, data=data, contentType=contentType)
    self.SetHeader("Content-Type", contentType)

  def Content(self):
    return self.data

####################################################################################################

class ItemInfoRecord(object):
  def __init__(self):
    self.title1 = None
    self.title2 = None
    self.art = None
    self.itemTitle = None

####################################################################################################

class ContextMenu(XMLContainer):
  def __init__(self, **kwargs):
    XMLContainer.__init__(self, **kwargs)
    
  def ToElement(self, sender, contextKey, **kwargs):
    root = XMLObject.ToElement(self)
    for item in self.__items__:
      if ObjectManager.ObjectHasBase(item, Function):
        item.AddFunctionArguments(sender=sender, key=contextKey, **kwargs)
        root.append(item.ToElement())
    return root

####################################################################################################
    
class MediaContainer(XMLContainer):

  def __init__(self, art=None, viewGroup=None, title1=None, title2=None, noHistory=False, replaceParent=False, disabledViewModes=None, **kwargs):
    XMLContainer.__init__(self, art=art, title1=title1, title2=title2, noHistory=noHistory, replaceParent=replaceParent, **kwargs)
    self.addTest(**testDefaults)
    if viewGroup is not None:
      if viewGroup in Plugin.ViewGroups().keys():
        self.viewGroup = viewGroup
      else:
        PMS.Log("(Framework) Couldn't assign view group '%s' to a MediaContainer - group doesn't exist" % viewGroup)

    if type(disabledViewModes).__name__ == "list":
      dvString = ""
      for view in disabledViewModes:
        if view in Plugin.ViewModes:
          if len(dvString) > 0: dvString += ","
          dvString += str(Plugin.ViewModes[view])
      self.disabledViewModes = dvString
  
  def __valueOrNone(self, key):
    if self.__dict__.has_key(key):
      return self.__dict__[key]
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
        

      if ObjectManager.ObjectHasBase(item, XMLObject):
        info = ItemInfoRecord()
        info.title1 = self.__valueOrNone("title1")
        info.title2 = self.__valueOrNone("title2")
        info.art = self.__valueOrNone("art")
        if item.__dict__.has_key("title"):
          info.itemTitle = item.title
        elif item.__dict__.has_key("name"):
          info.itemTitle = item.name
        else:
          info.itemTitle = None
        
        # Add sender information to functions
        if ObjectManager.ObjectHasBase(item, Function):
          item.AddFunctionArguments(sender=info)
          
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
    if self.__dict__.has_key("viewGroup"):
      root.set("viewmode", str(Plugin.ViewGroups()[self.viewGroup]["ViewMode"]))
      root.set("contenttype", str(Plugin.ViewGroups()[self.viewGroup]["MediaType"]))
    
    root.set("identifier", str(Plugin.Identifier))
    
    self.contextMenu = __containerContextMenu
    return root
    
  @test
  def addTest(self, minElements=None, maxElements=None, recursePercent=None):
    if minElements != None:
      self.test_minElements = minElements
    if maxElements != None:
      self.test_maxElements = maxElements
    if recursePercent != None:
      self.test_recursePercent = recursePercent


####################################################################################################

class MessageContainer(XMLContainer):
  def __init__(self, header, message, title1=None, title2=None, **kwargs):
    XMLContainer.__init__(self, header=header, message=message, title1=title1, title2=title2, **kwargs)
    self.tagName = "MediaContainer"

####################################################################################################

class DirectoryItem(XMLObject):
  def __init__(self, key, title, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, key=key, name=title, subtitle=subtitle, summary=summary, thumb=thumb, art=art, **kwargs)
    self.tagName = "Directory"

####################################################################################################

class PopupDirectoryItem(XMLObject):
  def __init__(self, key, title, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, key=key, name=title, subtitle=subtitle, summary=summary, thumb=thumb, popup=True, art=art, **kwargs)
    self.tagName = "Directory"

####################################################################################################

class InputDirectoryItem(XMLObject):
  def __init__(self, key, title, prompt, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, key=key, name=title, subtitle=subtitle, summary=summary, thumb=thumb, search=True, prompt=prompt, art=art, **kwargs)
    self.tagName = "Directory"
    
  @test
  def testQuery(self, query, expected=None):
    self.query = query
    if expected != None:
      self.test_expected = expected

####################################################################################################

class SearchDirectoryItem(InputDirectoryItem):
  def __init__(self, key, title, prompt, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    PMS.Log("(Framework) WARNING: SearchDirectoryItem is deprecated. Use InputDirectoryItem instead.")
    InputDirectoryItem.__init__(self, key=key, title=title, prompt=prompt, subtitle=subtitle, summary=summary, thumb=thumb, art=art, **kwargs)

####################################################################################################

class VideoItem(XMLObject):
  def __init__(self, key, title, subtitle=None, summary=None, duration=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, key=key, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, **kwargs)
    self.tagName = "Video"
    
####################################################################################################

class WebVideoItem(XMLObject):
  def __init__(self, url, title=None, subtitle=None, summary=None, duration=None, thumb=None, art=None, **kwargs):
    if Plugin.LastPrefix: prefix = Plugin.LastPrefix
    else: prefix = Plugin.Prefixes()[0]
    if isinstance(url, basestring):
      key = "plex://127.0.0.1/video/:/webkit?url=%s&prefix=%s" % (String.Quote(url, usePlus=True), prefix)
    else:
      key = url
    XMLObject.__init__(self, key=key, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, **kwargs)
    self.tagName = "Video"
    
####################################################################################################

class RTMPVideoItem(WebVideoItem):
  def __init__(self, url, clip=None, width=640, height=480, live=False, title=None, subtitle=None, summary=None, duration=None, thumb=None, art=None, **kwargs):
    if isinstance(url, basestring):
      final_url = "http://www.plexapp.com/player/player.php" + \
        "?url=" + String.Quote(url) + \
        "&clip=" + String.Quote(clip) + \
        "&width=" + str(width) + \
        "&height=" + str(height) + \
        "&live="
      if live:
        final_url += "true"
      else:
        final_url += "false"
    else:
      final_url = url
    WebVideoItem.__init__(self, final_url, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, **kwargs)

####################################################################################################

class WindowsMediaVideoItem(WebVideoItem):
  def __init__(self, url, width=720, height=576, title=None, subtitle=None, summary=None, duration=None, thumb=None, art=None, **kwargs):
    if isinstance(url, basestring):
      final_url = "http://www.plexapp.com/player/silverlight.php" + \
        "?stream=" + String.Quote(url) + \
        "&width=" + str(width) + \
        "&height=" + str(height)
    else:
      final_url = url
    WebVideoItem.__init__(self, final_url, title=title, subtitle=subtitle, summary=summary, duration=duration, thumb=thumb, art=art, **kwargs)

####################################################################################################

class PhotoItem(XMLObject):
  def __init__(self, key, title, subtitle=None, summary=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, key=key, title=title, subtitle=subtitle, summary=summary, thumb=thumb, art=art, **kwargs)
    self.tagName = "Photo"

####################################################################################################

class TrackItem(XMLObject):
  def __init__(self, key, title, artist=None, album=None, index=None, rating=None, duration=None, size=None, thumb=None, art=None, **kwargs):
    XMLObject.__init__(self, key=key, track=title, artist=artist, album=album, index=index, rating=rating, totalTime=duration, size=size, thumb=thumb, art=art, **kwargs)
    self.tagName = "Track"
    
####################################################################################################

class ProxyObject(object): pass

def create_query_string(kwargs):
  return 'function_args='+PMS.String.Quote(PMS.String.Encode(cerealizer.dumps(kwargs)))

class Function(XMLObject):
  def __init__(self, obj, ext=None, **kwargs):
    XMLObject.__init__(self)

    if type(obj).__name__ == "function":
      self.key = obj
    else:
      # Copy all attributes from the object to the function item
      for key in obj.__dict__:
        if key[0] != "_" and obj.__dict__[key].__class__.__name__ != "function" and obj.__dict__[key] != None:
          self.__dict__[key] = obj.__dict__[key]
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
    
    queryString = create_query_string(self.__kwargs)
    if queryString and len(queryString) > 0: queryString = '?' + queryString
    else: queryString = ''
    
    self.key = "%s/:/function/%s%s%s" % (Plugin.CurrentPrefix(), self.__obj.key.__name__, self.__ext, queryString)
    return XMLObject.ToElement(self)
    
  # Allow returning just the function path
  def __str__(self):
    if "key" in self.__dict__:
      if type(self.key).__name__ == "function":
        queryString = create_query_string(self.__kwargs)
        if queryString and len(queryString) > 0: queryString = '?' + queryString
        else: queryString = ''
        return "%s/:/function/%s%s%s" % (Plugin.CurrentPrefix(), self.key.__name__, self.__ext, queryString)
    return XMLObject.__str__(self)
    
  def __repr__(self):
    s = 'Function(%s' % (repr(self.__obj))
    for key, value in self.__kwargs.iteritems():
      s += ', %s=%s' % (key, repr(value))
    s += ')'
    return s
        
####################################################################################################

class PrefsItem(DirectoryItem):
  def __init__(self, title=None, subtitle=None, summary=None, thumb=None, **kwargs):
    DirectoryItem.__init__(self, "%s/:/prefs" % Plugin.Prefixes()[0], title=title, subtitle=subtitle, summary=summary, thumb=thumb, settings=True, **kwargs)

####################################################################################################

class Redirect(Object): 
  def __init__(self, url):
    Object.__init__(self)
    self.SetHeader("Location", str(url))
    
  def Status(self):
    return "301 Moved Permanently"

  def __repr__(self):
    return 'Redirect(%s)' % (repr(self.__dict__['_Object__headers']['Location']))
    
  def Content(self):
    return ""

####################################################################################################

cerealizer.register(Object)
cerealizer.register(Container)
cerealizer.register(XMLObject)
cerealizer.register(XMLContainer)
cerealizer.register(DataObject)
cerealizer.register(ItemInfoRecord)
cerealizer.register(ContextMenu)
cerealizer.register(MediaContainer)
cerealizer.register(MessageContainer)
cerealizer.register(DirectoryItem)
cerealizer.register(PopupDirectoryItem)
cerealizer.register(InputDirectoryItem)
cerealizer.register(VideoItem)
cerealizer.register(WebVideoItem)
cerealizer.register(RTMPVideoItem)
cerealizer.register(WindowsMediaVideoItem)
cerealizer.register(PhotoItem)
cerealizer.register(TrackItem)
cerealizer.register(Function)
cerealizer.register(PrefsItem)
cerealizer.register(Redirect)

