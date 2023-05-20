#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BaseKit

class URLServiceKit(BaseKit):

  def MetadataObjectForURL(self, url, add_items_automatically=True, allow_deferred=True, in_container=False, do_normalization=True):
    """
      Calls MetadataObjectForURL in the appropriate URL service and returns the result, setting
      the original URL on the object and populating media items if none were provided.
    """
    return self._core.services.metadata_object_for_url(url, add_items_automatically, allow_deferred, in_container, do_normalization)
    
    
  def MediaObjectsForURL(self, url, allow_deferred=True, do_normalization = True):
    """ Calls MediaObjectsForURL in the appropriate URL service and returns the result. """
    return self._core.services.media_objects_for_url(url, allow_deferred, do_normalization)
    

  def LookupURLForMediaURL(self, url):
    return self._core.services.lookup_url_for_media_url(url)
    
    
  def NormalizeURL(self, url):
    """ Calls NormalizeURL in the appropriate URL service and returns the result. If an exception is thrown, or the URL service returns None, return the original URL """
    return self._core.services.normalize_url(url)
    

  def ServiceIdentifierForURL(self, url):
    """ Returns the identifier of a service that matches the given URL, or None if no match is found """
    service = self._core.services.service_for_url(url)
    if service == None:
      return None
    else:
      return service.identifier


  @property
  def AllPatterns(self):
    patterns = []
    us = self._core.services.url_services
    for identifier in us:
      for name in us[identifier]:
        record = us[identifier][name]
        patterns.extend(record.uncompiled_patterns)
    return patterns
    


class SearchServiceKit(BaseKit):
  
  def Query(self, query, identifier, name):
    return self._core.services.search(query, identifier, name)

  def List(self, identifier):
    services = self._core.services.get_all_services(identifier)
    l = []
    for service in services:
      if self._core.services._type_for_service(service) == 'search':
        l.append(service.name)

    return l
    

    
class RelatedContentServiceKit(BaseKit):
  
  def RelatedContentForMetadata(self, metadata, identifier, name):
    self._core.services.related_content_for_metadata(metadata, identifier, name)



class SharedCodeServiceKit(BaseKit):

  def __getattr__(self, name):
    if name[0] == '_':
      return BaseKit.__getattr__(self, name)

    obj = self._core.services.shared_code_sandbox.environment.get(name)
    if obj == None:
      raise Framework.exceptions.FrameworkException('No shared code object named "%s" was found.', name)
    return obj
    


class ServiceKit(BaseKit):
  
  _root_object = False

  _included_policies = [
    Framework.policies.CodePolicy,
  ]
  
  def _init(self):
    self._publish(URLServiceKit)
    self._publish(SearchServiceKit)
    self._publish(RelatedContentServiceKit)
    self._publish(SharedCodeServiceKit)
    
    