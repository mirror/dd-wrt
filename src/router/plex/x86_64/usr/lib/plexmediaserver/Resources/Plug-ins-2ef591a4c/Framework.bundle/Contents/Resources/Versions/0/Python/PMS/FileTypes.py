#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  WIP: Classes to assist with creation of files for returning from plugins.
"""
import Plugin

####################################################################################################

class PMSFileType:
  #
  # Base class for file types; should not be instantiated directly
  #
  def __init__(self, content_type):
  	self.ContentType = content_type
  
  def Content():
    return None
    
####################################################################################################
 
class PLS(PMSFileType):
  """
    Class for creating PLS-formatted playlists.
  """
  def __init__(self):
    """
      Creates a new PLS playlist object.
      
      @return: PLS
    """
    PMSFileType.__init__(self, "audio/x-scpls")
    self.__tracks = []
  
  def AppendTrack(self, url, title, length=-1):
    """
      Appends a track with the given URL, title and length to the playlist.
      
      @param url: The URL of the track
      @type url: string
      @param title: The title of the track
      @type title: string
      @param length: The length of the track in seconds (optional)
      @type length: int
    """
    self.__tracks.append({"url": url, "title": title, "length": str(length)})
    
  def Content(self):
    """
      Returns the content of the object in the PLS file format.
      
      @return: string
    """
    pls_file = "[playlist]\n\nNumberOfEntries=%i\n\n" % len(self.__tracks)
    for i in range(len(self.__tracks)):
      pls_file += "File%i=%s\n" % (i+1, self.__tracks[i]["url"])
      pls_file += "Title%i=%s\n" % (i+1, self.__tracks[i]["title"])
      pls_file += "Length%i=%s\n\n" % (i+1, self.__tracks[i]["length"])
    pls_file += "Version=2\n"
    return pls_file
    
####################################################################################################