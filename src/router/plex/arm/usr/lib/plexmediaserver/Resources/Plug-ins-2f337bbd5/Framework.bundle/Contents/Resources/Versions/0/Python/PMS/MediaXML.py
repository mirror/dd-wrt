#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  WIP: Classes to assist with construction and serialization of media items.
"""

# TODO: Correct web media definitions when PMS has WebKit support

import XML, Plugin, Log, urllib

ViewModes = {"List": 65586, "InfoList": 65592, "MediaPreview": 458803, "Showcase": 458810, "Coverflow": 65591, 
             "PanelStream": 131124, "WallStream": 131125, "Songs": 65593, "Seasons": 65593, "Albums": 131123, 
             "Episodes": 65590,"ImageStream":458809,"Pictures":131123}

####################################################################################################

class MediaItem:
  """
    MediaItem is a class for simplifying the creation and serialization of media objects for
    returning results from plugins as XML.
  """
  def __init__(self, tag="MediaItem"):
    """
      Creates a custom MediaItem with the given tag
      
      @param tag: The tag for the MediaItem
      @type tag: string
      @return: MediaItem
    """
    self.root = XML.Element(tag)

  def ToXML(self):
    """
      Serialize the item as XML.
      
      @return: string
    """
    return XML.ElementToString(self.root)
  
  def GetAttr(self, name):
    """
      Get the named attribute value of the item.
      
      @param name: The attribute name
      @type name: string
      @return: string
    """
    return self.root.get(name)
  
  def SetAttr(self, name, value):
    """
      Set the named attribute of the item to the given value.
      
      @param name: The attribute name
      @type name: string
      @param value: The attribute value
      @type value: string
      @return: string
    """
    if value is not None:
      self.root.set(name, value)
      return True
    return False
    
####################################################################################################

class MediaObject(MediaItem):
  """
    MediaObject is a subclass of MediaItem that allows child items and elements to be added.
  """
  def AppendItem(self, item):
    """
      Append a MediaItem to the object.
      
      @param item: The MediaItem to append
      @type item: MediaItem
      @return: Boolean
    """
    if item is not None:
      self.AppendElement(item.root)
      return True
    return False
    
  def AppendElement(self, element):
    """
      Append an XML element to the object
      
      @param element: The element to append
      @type element: Element
      @return: Boolean
    """
    if element is not None:
      self.root.append(element)
      return True
    return False
    
  def ChildCount(self):
    """
      Returns the number of child elements of the object.
      
      @return int
    """
    return len(self.root.getchildren())
    
####################################################################################################

class MediaContainer(MediaObject):
  """
    MediaContainer is a subclass of MediaObject that includes an automatically generated
    "size" attribute. This is the standard object type for returning directories from plugins.
  """
  def __init__(self, art=None, viewGroup=None, title1=None, title2=None, noHistory=False, replaceParent=False):
    """
      Creates an empty MediaContainer object.
      @param art: The name of an exposed artwork image in the resources directory.  
      @return: MediaContainer
    """
    MediaObject.__init__(self, "MediaContainer")
    self.SetAttr("size", "0")
    self.SetAttr("art", Plugin.ExposedResourcePath(art))
    self.SetAttr("title1", title1)
    self.SetAttr("title2", title2)
    if noHistory: self.SetNoHistory(True)
    if replaceParent: self.SetReplaceParent(True)
    if viewGroup is not None:
      self.SetViewGroup(viewGroup)
    
  def AppendElement(self, element):
    MediaObject.AppendElement(self, element)
    self.SetAttr("size", str(int(self.GetAttr("size"))+1))
  
  def SetViewGroup(self, viewGroup):
    if Plugin.ViewGroups.has_key(viewGroup):
      self.SetAttr("viewmode", Plugin.ViewGroups[viewGroup]["ViewMode"])
      self.SetAttr("content", Plugin.ViewGroups[viewGroup]["ContentType"])
    else:
      Log.Add("(Framework) Error: Invalid view group.")
    
  def SetMessage(self, header, message):
    """
      Sets the message to be shown when displaying the MediaContainer. If the MediaContainer
      contains no items, Plex will return to the previous directory after displaying the
      message.
      
      @param header: The header of the message dialog
      @type header: string
      @param message: The body of the message dialog
      @type message: string
    """
    self.SetAttr("header", header)
    self.SetAttr("message", message)
    
  def SetNoHistory(self, noHistory=False):
    if noHistory:
      self.SetAttr("noHistory", "1")
    else:
      self.SetAttr("noHistory", "0")
      
  def SetReplaceParent(self, replaceParent=False):
    if replaceParent:
      self.SetAttr("replaceParent", "1")
    else:
      self.SetAttr("replaceParent", "0")
    
####################################################################################################

class MessageContainer(MediaContainer):
  def __init__(self, header, message, art=None, title1=None, title2=None):
    MediaContainer.__init__(self, art, None, title1, title2)
    self.SetMessage(header, message)
    
####################################################################################################    
    
class DirectoryItem(MediaItem):
  """
    DirectoryItem is a subclass of MediaItem. This is the standard object type for returning
    directory items from plugins (contained in a MediaContainer).
  """
  def __init__(self, key, name, thumb="", summary=None):
    """
      Creates a new DirectoryItem with the given key, name and thumbnail path.
      
      @param key: The unique key for the directory
      @type key: string
      @param name: The name of the directory
      @type name: string
      @param thumb: The thumbnail path for the directory
      @type thumb: string
      @return: DirectoryItem
    """
    MediaItem.__init__(self, "Directory")
    self.SetAttr("key", key)
    self.SetAttr("name", name)
    self.SetAttr("thumb", thumb)
    self.SetAttr("summary", summary)
    
  def SetSearch(self, isSearchDir, searchPrompt=""):
    """
      Defines whether the current item is a search item & sets the displayed prompt.
      
      @param isSearchDir: Specifies whether the item is a search directory
      @type isSearchDir: boolean
      @param searchPrompt: The prompt to display
      @type searchPrompt: string
    """
    if isSearchDir:
      self.SetAttr("search", "1")
      self.SetAttr("prompt", searchPrompt)
    else:
      self.SetAttr("search", "0")
      
####################################################################################################

class PopupDirectoryItem(DirectoryItem):
  def __init__(self, key, name, thumb="", summary=""):
    DirectoryItem.__init__(self, key, name, thumb, summary)
    self.SetAttr("popup", "1")

####################################################################################################

class SearchDirectoryItem(DirectoryItem):
  """
    SearchDirectoryItem is a subclass of DirectoryItem. This is the standard object type for returning
    search directory items from plugins (contained in a MediaContainer).
  """
  def __init__(self, key, name, prompt, thumb="", summary=""):
    """
      Creates a new SearchDirectoryItem with the given key, name, prompt and thumbnail path.
      
      @param key: The unique key for the directory
      @type key: string
      @param name: The name of the directory
      @type name: string
      @param prompt: The search prompt to display
      @type prompt: string
      @param thumb: The thumbnail path for the directory
      @type thumb: string
      @return: DirectoryItem
    """
    DirectoryItem.__init__(self, key, name, thumb, summary)
    self.SetSearch(True, prompt)
      
####################################################################################################

class VideoItem(MediaItem):
  """
    VideoItem is a subclass of MediaItem. This is the standard object type for returning
    video items from plugins (contained in a MediaContainer).
  """
  def __init__(self, key, title, summary, duration, thumb):
    """
      Creates a new VideoItem with the given key, title, summary, duration and thumbnail path.
      
      @param key: The unique key for the video
      @type key: string
      @param title: The title of the video
      @type title: string
      @param summary: A short description of the video
      @type summary: string
      @param duration: The duration of the video (in milliseconds)
      @type duration: string
      @param thumb: The thumbnail path for the video
      @type thumb: string
      @return: DirectoryItem
    """
    MediaItem.__init__(self, "Video")
    self.SetAttr("key", key)
    self.SetAttr("title", title)
    self.SetAttr("summary", summary)
    self.SetAttr("duration", duration)
    self.SetAttr("thumb", thumb)
    
  def SetTelevisonMetadata(self, show, season, episode):
    self.SetAttr("show", show)
    self.SetAttr("season", season)
    self.SetAttr("episode", episode)

####################################################################################################

class PhotoItem(MediaItem):
  """
    PhotoItem is a subclass of MediaItem. This is the standard object type for returning
    photo/picture items from plugins (contained in a MediaContainer).
  """
  def __init__(self, key, title, summary, thumb):
    """
      Creates a new PhotoItem with the given key, title, summary, and thumbnail path.

      @param key: The unique key for the video
      @type key: string
      @param title: The title of the video
      @type title: string
      @param summary: A short description of the video
      @type summary: string
      @param thumb: The thumbnail path for the video
      @type thumb: string
      @return: DirectoryItem
    """
    MediaItem.__init__(self, "Photo")
    self.SetAttr("key", key)
    self.SetAttr("title", title)
    self.SetAttr("summary", summary)
    self.SetAttr("thumb", thumb)

####################################################################################################

class TrackItem(MediaItem):
  """
    TrackItem is a subclass of MediaItem. This is the standard object type for returning
    audio track items from plugins (contained in a MediaContainer).
  """
  def __init__(self, key, track, artist, album, index, rating, duration, size, thumb):
    """
      Creates a new TrackItem with the given key, title, summary, and thumbnail path.

      @param key: The unique key for the track
      @type key: string
      @param track: The title of the track
      @type track: string
      @param artist: The artist's name
      @type artist: string
      @param album: The album the track belongs to
      @type album: string
      @param index: The track number
      @type index: string
      @param rating: The track's rating
      @type rating: string
      @param duration: The duration of the track (in milliseconds)
      @type duration: string
      @param size: The size of the track file
      @type size: string
      @param thumb: The thumbnail path for the track
      @type thumb: string
      @return: TrackItem
    """
    MediaItem.__init__(self, "Track")
    self.SetAttr("key", key)
    self.SetAttr("track", track)
    self.SetAttr("artist", artist)
    self.SetAttr("album", album)
    self.SetAttr("index", index)
    self.SetAttr("rating", rating)
    self.SetAttr("duration", duration)
    self.SetAttr("size", size)
    self.SetAttr("thumb", thumb)

####################################################################################################

class WebVideoItem(VideoItem):
  """
    WebVideoItem is a subclass of MediaItem. This is the standard object type for returning
    web video items from plugins (contained in a MediaContainer).
  """
  def __init__(self, url, title, summary, duration, thumb):
    """
      Creates a new WebVideoItem with the given URL, title, summary, duration and thumbnail path.
      
      @param url: The URL of the web page containing the video
      @type url: string
      @param title: The title of the video
      @type title: string
      @param summary: A short description of the video
      @type summary: string
      @param duration: The duration of the video (in milliseconds)
      @type duration: string
      @param thumb: The thumbnail path for the video
      @type thumb: string
      @return: DirectoryItem
    """
    prefix = Plugin.Prefixes()[0]
    key = "plex://127.0.0.1/video/:/webkit?url=%s&prefix=%s" % (urllib.quote_plus(url), prefix)
    VideoItem.__init__(self, key, title, summary, duration, thumb)

####################################################################################################