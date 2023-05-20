import Framework
from Framework.components.services import TEST_URLS_FUNCTION_NAME, TEST_QUERIES_FUNCTION_NAME
from base import BaseHandler, InternalRequestHandler
import tornado.httpserver

URL_SERVICE_TEST = dict(set_name = 'url_services', list_attr = 'test_urls', function_name = TEST_URLS_FUNCTION_NAME)
SEARCH_SERVICE_TEST = dict(set_name = 'search_services', list_attr = 'test_queries', function_name = TEST_QUERIES_FUNCTION_NAME)

class ServiceRequestHandler(InternalRequestHandler):

  # Instruction from the System bundle to reload services
  @BaseHandler.route('/reloadServices') # Legacy route
  @BaseHandler.route('/services/reload')
  def reload_services(self):
    self._core.log.debug('Reloading services')
    self._core.services.load_all_services()
    return True

  # External request for a URL lookup
  @BaseHandler.route('/url/lookup') # Legacy route
  @BaseHandler.route('/services/url/lookup')
  def url_lookup(self, url, limit=None, **kwargs):
    self._core.log.debug("Looking up URL '%s'", url)
    
    # Get the metadata object for the given URL
    c = self._core.services.metadata_object_for_url(url, in_container=True)

    if c != None:
      obj = c.objects[0]
      self._core.log.debug("TAG: %s", obj.xml_tag)
      if isinstance(obj, Framework.api.objectkit.DirectoryObject) or (isinstance(obj, Framework.modelling.objects.Object) and obj._template.xml_tag == 'Directory'):
        req = tornado.httpserver.HTTPRequest('GET', str(obj.key), headers=self._core.sandbox.context.request.headers)
        status, headers, body = self._core.runtime.handle_request(req)
        if status == 200:
          self._core.sandbox.context.response_headers.update(headers)
          c = body
        else:
          raise Framework.exceptions.FrameworkException("Status != 200")
        
      else:
        # Move the user agent and http cookies (if set) to the container instead of the object
        # TODO: Move this to metadata_object_for_url
        try:
          if obj.user_agent:
            c.user_agent = obj.user_agent
            obj.user_agent = None
        except:
          pass
          
        try:
          if obj.http_cookies:
            c.http_cookies = obj.http_cookies
            obj.http_cookies = None
        except:
          pass
          
        # TODO: Handle custom HTTP headers here too.
  
        if limit and len(obj) > int(limit):
          obj.items = obj.items[:int(limit)]
        
      return c

  # Search request
  @BaseHandler.route('/serviceSearch') # Legacy route
  @BaseHandler.route('/services/search')
  def search(self, query, identifier, name=None):
    return self._core.services.search(query, identifier, name)
    
  # Request for known service tests
  @BaseHandler.route('/serviceTestURLs', **URL_SERVICE_TEST) # Legacy route
  @BaseHandler.route('/serviceTestURLs/{identifier}', **URL_SERVICE_TEST) # Legacy route
  @BaseHandler.route('/serviceTestURLs/{identifier}/{service_name}', **URL_SERVICE_TEST) # Legacy route
  @BaseHandler.route('/serviceTestQueries', **SEARCH_SERVICE_TEST) # Legacy route
  @BaseHandler.route('/serviceTestQueries/{identifier}', **SEARCH_SERVICE_TEST) # Legacy route
  @BaseHandler.route('/serviceTestQueries/{identifier}/{service_name}', **SEARCH_SERVICE_TEST) # Legacy route

  @BaseHandler.route('/services/tests/urls', **URL_SERVICE_TEST)
  @BaseHandler.route('/services/tests/queries', **SEARCH_SERVICE_TEST)
  def tests(self, set_name, list_attr, function_name, identifier=None, service_name=None):
    sets = getattr(self._core.services, set_name)
    results = {}
    # Filter service sets based on identifier if one was provided
    service_sets = sets if identifier == None else {identifier: sets[identifier]}

    # Iterate over the service sets
    for identifier, service_set in service_sets.items():
      result = {}

      # Filter services based on name if one was provided
      services = service_set if service_name == None else {service_name: service_set[service_name]}

      # Iterate over services
      for name, service in services.items():
        
        # Get the list of test queries defined in the plist
        tests = list(getattr(service, list_attr))

        # Extend the list with tests defined in the code file, if found
        try:
          tests_from_code = self._core.services._call_named_function_in_service(function_name, service, [], {})
          if tests_from_code and isinstance(tests_from_code, list):
            tests.extend(tests_from_code)
        except:
          pass

        # Add the list of tests to the service's result dictionary
        result[name] = tests

      # Add the result dictionary to the full set of results
      results[identifier] = result
    
    # Return the results to the client
    return results


  def strip_extension_from_name(self, f_name):
    pos = f_name.find('.')
    if pos > -1:
      f_name = f_name[:pos]
    return f_name

  def unpack_arg_objects(self, args, kwargs, other_kwargs={}):
    f_args = Framework.utils.unpack(args) if args else []
    f_kwargs = Framework.utils.unpack(kwargs) if kwargs else {}    
    f_kwargs.update(other_kwargs)
    return f_args, f_kwargs

  # Request for a function call inside a service
  @BaseHandler.route('/serviceFunction/{service_type}/{service_identifier}/{service_name}/{f_name}') # Legacy route
  @BaseHandler.route('/services/url/function/{service_identifier}/{service_name}/{f_name}', service_type = 'url')
  @BaseHandler.route('/services/search/function/{service_identifier}/{service_name}/{f_name}', service_type = 'search')
  @BaseHandler.route('/services/relatedContent/function/{service_identifier}/{service_name}/{f_name}', service_type = 'related_content')
  def call_service_function(self, service_type, service_identifier, service_name, f_name, args=None, kwargs=None, **other_kwargs):
    # Strip the extension if one is found and extract args & kwargs for the function from the querystring args
    f_name = self.strip_extension_from_name(f_name)
    f_args, f_kwargs = self.unpack_arg_objects(args, kwargs, other_kwargs)
      
    # Get the correct dictionary of services
    attr_name = service_type + '_services'
    if not hasattr(self._core.services, attr_name):
      self._core.log.error("No services exist for type '%s'", service_type)
      return None
    services = getattr(self._core.services, attr_name)
    
    if service_identifier in services:
      service_set = services[service_identifier]
      if service_name in service_set:
        service = service_set[service_name]
        result = self._core.services._call_named_function_in_service(f_name, service, f_args, f_kwargs)
        return result


  # Request for a function call inside shared service code
  @BaseHandler.route('/sharedServiceFunction/{mod_name}/{f_name}') # Legacy route
  @BaseHandler.route('/services/shared/function/{mod_name}/{f_name}')
  def call_shared_service_function(self, mod_name, f_name, args=None, kwargs=None, **other_kwargs):
    # Strip the extension if one is found and extract args & kwargs for the function from the querystring args
    f_name = self.strip_extension_from_name(f_name)
    f_args, f_kwargs = self.unpack_arg_objects(args, kwargs, other_kwargs)

    # Call the function and return the result
    return self._core.services._call_named_function_in_shared_service_code(f_name, mod_name, f_args, f_kwargs)

