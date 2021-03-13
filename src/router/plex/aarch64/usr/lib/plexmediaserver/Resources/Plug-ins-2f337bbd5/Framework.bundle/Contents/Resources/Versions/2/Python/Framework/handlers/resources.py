import Framework
import urllib

from base import BaseHandler, InternalRequestHandler

class ResourceRequestHandler(InternalRequestHandler):

  @BaseHandler.route('/resources/contentWithFallback')
  def contents_of_url_with_fallback(self, fallback=None, urls=None, identifier=None, hosted_type=None, hosted_group=None, hosted_identifier=None, **kwargs):
    # Check whether we should be using temporary redirects or not.
    # PMS <0.9.6.6 can only transcode images from permanent redirects.
    temporary = self._core.server_version_at_least(0,9,6,6)

    sandbox = None

    # If an identifier was given, find a matching service sandbox
    if identifier:
      all_services = self._core.services.get_all_services(identifier)

      if len(all_services) > 0:
        sandbox = all_services[0].sandbox

    # Otherwise, use the core identifier & sandbox.
    else:
      identifier = self._core.identifier
      sandbox = self._core.sandbox

    # Convert None or a comma-separated string to a list.
    if urls == None:
      urls = []
    elif isinstance(urls, basestring):
      urls = [urllib.unquote(u) for u in urls.split(',')]
    
    for url in urls:
      try:
        request = self._core.networking.http_request(url, cacheTime=0, sandbox=sandbox)

        # Try to get headers first. If that fails, try to get content. If both fail, we'll loop
        # again, but if either succeeds we'll redirect to the final URL.
        try:
          request.headers
        except:
          request.content
        
        return Framework.objects.Redirect(self._core, url, temporary)

      except:
        pass

    # Nothing returned so far? Use the fallback resource.

    path = None

    # Check for a bundled fallback first.
    if fallback:
      path = self._core.runtime.external_resource_path(fallback, sandbox)

    # Next, check for hosted resource info.
    elif hosted_group and hosted_type:
      path = self._core.runtime.hosted_resource_url(hosted_type, hosted_group, hosted_identifier)

    # If neither was provided, use the source image for the current identifier.
    else:
      path = self._core.runtime.hosted_resource_url('image', 'source', identifier)

    if path:
        return Framework.objects.Redirect(self._core, path, temporary)


  @BaseHandler.route('/resources/{resource_name}')
  def resource(self, resource_name, t=None, identifier=None):
    # If an identifier was provided, try to find a matching service sandbox, otherwise use the core sandbox.
    sandbox = None
    if identifier:
      all_services = self._core.services.get_all_services(identifier)
      if len(all_services) == 0:
        return None
      sandbox = all_services[0].sandbox
    
    # Attempt to load and return a resource accessible to the selected sandbox.
    resource = self._core.storage.load_resource(resource_name, sandbox=sandbox)
    if resource:
      return Framework.objects.DataObject(self._core, resource, Framework.utils.guess_mime_type(resource_name))

