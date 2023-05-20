#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import mimetypes
import urllib

from base import BaseKit

class ResourceKit(BaseKit):

  _included_policies = [
    Framework.policies.CodePolicy,
  ]
  

  def _init(self):
    self.Hosted = self._core.runtime.hosted_resource_url
    self._publish(self.ExternalPath, name='R')
    self._publish(self.SharedExternalPath, name='S')

    
  def ExternalPath(self, itemname):
    """
      Generates an externally accessible path for the named resource. This method is usually used to
      assign bundled images to object attributes.
      
      .. note:: The alias :func:`R()` is equivalent to this method.
    """
    return self._core.runtime.external_resource_path(itemname, self._sandbox)
  

  def SharedExternalPath(self, itemname):
    """
      Generates an externally accessible path for the named shared resource. A list of valid
      shared resource names is provided below.
      
      .. note:: The alias :func:`S()` is equivalent to this method.
    """
    self._core.log.warn("Resource.SharedExternalPath() (and the 'S' alias) are deprecated. All resource path generation can now be done via Resource.ExternalPath() (and the 'R' alias). Please update your code.")
    return self.ExternalPath(itemname)
  

  def Load(self, itemname, binary=True):
    """
      Loads the named resource file and returns the data as a string.
    """
    return self._core.storage.load_resource(itemname, binary, self._sandbox)
    

  def LoadShared(self, itemname, binary=True):
    """
      Loads the named shared resource file and returns the data as a string.
    """
    self._core.log.warn("Resource.LoadShared() is deprecated. All resource loading can now be done via Resource.Load(). Please update your code.")
    return self.Load(itemname, binary)
    

  def GuessMimeType(self, path):
    """
      Attempts to guess the MIME type of a given path based on its extension.
    """
    return Framework.utils.guess_mime_type(path)
    

  @BaseKit._include_in(Framework.policies.BundlePolicy)
  @BaseKit._exclude_from(Framework.policies.ModernPolicy)
  def AddMimeType(self, mimetype, extension):
    """
      Assigns the specified MIME type to the given extension.
    """
    if extension[0] != '.': extension = '.' + extension
    mimetypes.add_type(mimetype, extension)


  def ContentsOfURLWithFallback(self, url, fallback=None, hosted_fallback=None):
    # Find the handler function.
    func = self._core.runtime.find_handler(Framework.handlers.ResourceRequestHandler).contents_of_url_with_fallback

    # Put single strings inside a list
    if isinstance(url, basestring):
      url = [url]

    # Generate a callback path. Format url lists as a comma-separated string, and include an identifier if this isn't the core sandbox.
    kwargs = {}
    kwargs['urls'] = ','.join([urllib.quote(u) for u in url]) if isinstance(url, list) else None
    
    if fallback != None:
      kwargs['fallback'] = fallback

    if self._sandbox.identifier != self._core.identifier:
      kwargs['identifier'] = urllib.quote(self._sandbox.identifier)
        
    if hosted_fallback != None:
      kwargs['hosted_type'] = hosted_fallback[0]
      kwargs['hosted_group'] = hosted_fallback[1]
      if len(hosted_fallback) > 2:
        kwargs['hosted_identifier'] = hosted_fallback[2]
        
    return self._core.runtime.generate_callback_path(func, **kwargs)
      
