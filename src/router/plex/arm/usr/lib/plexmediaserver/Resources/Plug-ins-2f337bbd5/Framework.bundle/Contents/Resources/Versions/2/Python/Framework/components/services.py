import Framework
import re
import plistlib
import urllib

from base import BaseComponent

# TODO: Remove the code that loads old-style services at some point.

NORMALIZE_URL_FUNCTION_NAME = 'NormalizeURL'
METADATA_OBJECT_FUNCTION_NAME = 'MetadataObjectForURL'
MEDIA_OBJECTS_FUNCTION_NAME = 'MediaObjectsForURL'
SEARCH_FUNCTION_NAME = 'Search'
RELATED_CONTENT_FUNCTION_NAME = 'RelatedContentForMetadata'
TEST_URLS_FUNCTION_NAME = 'TestURLs'
TEST_QUERIES_FUNCTION_NAME = 'TestQueries'
      
class ServiceRecord(Framework.CoreObject):
  def __init__(self, core, path, name, identifier, source_title):
    Framework.CoreObject.__init__(self, core)
    self.name = name
    self.path = self._core.storage.join_path(path, name)
    self.identifier = identifier
    self.source_title = source_title
    self._sandbox = None
    self._mtime = 0
    
  @property
  def sandbox(self):
    service_code_path = self._core.storage.join_path(self.path, 'ServiceCode.pys')
    mtime = self._core.storage.last_modified(service_code_path)

    self._core.services.handle_shared_code_modification()

    if self._sandbox and mtime == self._mtime:
      return self._sandbox
    
    if self._sandbox:
      self._core.log.debug("Reloading service code for %s (%s)", self.name, type(self).__name__)
    else:
      self._core.log.debug("Loading service code for %s (%s)", self.name, type(self).__name__)
          
    code = self._core.loader.load(service_code_path)
    sandbox = Framework.code.Sandbox(self._core, service_code_path, Framework.policies.ServicePolicy, self._core.services._service_flags.get(self.identifier, []), identifier=self.identifier)
    sandbox.custom_paths.append(self._core.storage.abs_path(self._core.storage.join_path(self.path, '..', '..', 'Shared Code')))
    sandbox.resource_paths.append(self._core.storage.abs_path(self._core.storage.join_path(self.path, '..', '..', 'Resources')))

    # Substitute the default Callback() function with service path generators
    sandbox.environment['Callback'] = self._generate_callback_path
    
    sandbox.execute(code)
    
    self._sandbox = sandbox
    self._mtime = mtime
    
    return sandbox

  def _generate_callback_path(self, function, ext=None, post_url=None, post_headers={}, *args, **kwargs):
    #TODO: Dedupe
    """ Generate a callback path for a function inside a service """
    # Check for an IndirectFunction and extract the actual function from the object if necessary
    if isinstance(function, Framework.components.runtime.IndirectFunction):
      function = function._f
      indirect = True
    else:
      indirect = False
      
    service_type = self._core.services._type_for_service(self)
    
    # Format the extension, if provided
    if ext and ext[0] != '.':
      ext = '.'+ext
    elif ext == None:
      ext = ''

    path = "/:/plugins/%s/serviceFunction/%s/%s/%s/%s%s?args=%s&kwargs=%s" % (
      self._core.identifier,
      service_type,
      str(self.identifier),
      urllib.quote(str(self.name)),
      function.__name__,
      ext,
      Framework.utils.pack(args),
      Framework.utils.pack(kwargs)
    )
    
    # Convert to a callback_string object
    if indirect:
      path = Framework.components.runtime.indirect_callback_string(path + "&indirect=1")
    else:
      path = Framework.components.runtime.callback_string(path)
      
    # Add POST args
    path.post_url = post_url
    path.post_headers = post_headers
      
    return path

class URLServiceRecord(ServiceRecord):
  def __init__(self, core, path, name, identifier, patterns, fallback=False, source_title=None, test_urls=[], priority=50):
    ServiceRecord.__init__(self, core, path, name, identifier, source_title)
    self.uncompiled_patterns = patterns
    self.patterns = [re.compile(pattern, re.IGNORECASE)for pattern in patterns]
    self.fallback = fallback
    self.test_urls = test_urls
    self.priority = priority
    
class SearchServiceRecord(ServiceRecord):
  def __init__(self, core, path, name, identifier, source_title=None, test_queries=[]):
    ServiceRecord.__init__(self, core, path, name, identifier, source_title)
    self.test_queries = test_queries
    
class RelatedContentServiceRecord(ServiceRecord):
  def __init__(self, core, path, name, identifier, source_title=None):
    ServiceRecord.__init__(self, core, path, name, identifier, source_title)
    
class Services(BaseComponent):
  def _init(self):
    self._services_loaded = self._core.runtime.event()
    self._shared_code_loaded = self._core.runtime.event()
    self.root_paths = []
    self.shared_mtimes = {}

    self._url_services = {}
    self._search_services = {}
    self._related_content_services = {}
    self._service_flags = {}
    
  def load(self):
    self._core.runtime.create_thread(self.load_all_services)
    self._core.runtime.create_thread(self._setup_shared_code_sandbox)

    
  def get_services_from_bundle(self, bundle_path, plist=None):
    if plist == None:
      plist = plistlib.readPlist(self._core.storage.abs_path(self._core.storage.join_path(bundle_path, 'Contents', 'Info.plist')))
    bundle_identifier = plist['CFBundleIdentifier']
    
    old_services = dict()
    services = dict()
    service_sets = dict()
    service_flags = dict()

    # Load the plist from the bundle
    if Framework.constants.flags.log_service_loads in self._core.flags:
      self._core.log.debug("  -> Loading services from '%s'", bundle_path)
    
    # Get the separate service directories
    service_dir = self._core.storage.join_path(bundle_path, "Contents", "Services")
    service_set_dir = self._core.storage.join_path(bundle_path, "Contents", "Service Sets")
    
    # Needed to convert plistlib._InternalDict types to regular dictionaries, so they can be seralised correctly
    def convert_dicts(d):
      f = {}
      for k in d:
        v = d[k]
        f[k] = convert_dicts(v) if isinstance(v, dict) else v
      return f
    
    
    # Get the list of old-style services from the main plist
    def get_old_services(service_key):
      plist_key = 'Plex%sServices' % service_key
      if plist_key in plist:
        dct = convert_dicts(plist[plist_key])
        for service_name, service in dct.items():
          
          # Check for very-old-style keys and rewrite them using the slightly newer format.
          # Makes the service dictionaries fully compatible with new-style services, requiring
          # only one dict parsing function.
          #
          if plist_key == 'PlexURLServices':
            if 'LinkedPlugin' in service:
              service['Identifier'] = service['LinkedPlugin']
              del service['LinkedPlugin']
            if 'URLPattern' in service:
              url_patterns = service.get('URLPatterns', [])
              url_patterns.append(service['URLPattern'])
              service['URLPatterns'] = url_patterns
              del service['URLPattern']
            
          if plist_key == 'PlexSearchServices':
            if isinstance(service, basestring):
              dct[service_name] = {'Identifier' : service}
            
        old_services[service_key] = dct
  
    [get_old_services(key) for key in ('URL', 'Search', 'RelatedContent')]

    def get_services(key, pl=None):
      if pl == None:
        pl = plist
      if key not in pl:
        return {}
      return convert_dicts(pl[key])
    
    def load_services_from_plist(service_plist_path, dct, identifier):
      if self._core.storage.file_exists(service_plist_path):
        # Load the plist and see what services are supported
        if Framework.constants.flags.log_service_loads in self._core.flags:
          self._core.log.debug("      - Loading plist from '%s'", service_plist_path)
        service_plist = plistlib.readPlistFromString(self._core.storage.load(service_plist_path))
        for key in ('URL', 'Search', 'RelatedContent'):
          dct[key] = get_services(key, service_plist)

        flags = service_plist.get('PlexFrameworkFlags', [])
        if len(flags) > 0:
          current_flags = service_flags.get(identifier, [])
          current_flags.extend(flags)
          service_flags[identifier] = current_flags
          
    # Load services plist
    service_plist_path = self._core.storage.join_path(service_dir, "ServiceInfo.plist")
    load_services_from_plist(service_plist_path, services, bundle_identifier)
    
    # Load service set plists
    if self._core.storage.dir_exists(service_set_dir):
      for name in self._core.storage.list_dir(service_set_dir):
        if name not in service_sets:
          service_sets[name] = {}
        service_plist_path = self._core.storage.join_path(service_set_dir, name, "ServiceInfo.plist")
        load_services_from_plist(service_plist_path, service_sets[name], name)
        
    return {
      bundle_identifier: {
        'Path': bundle_path,
        'OldServices': old_services,
        'Services': services,
        'ServiceSets': service_sets,
        'ServiceFlags': service_flags,
      }
    }
    
    
  def load_all_services(self):
    self._url_services = {}
    self._search_services = {}
    self._related_content_services = {}
    self._service_flags = {}
    services_dict = {}
    
    # Get the services dict from the system bundle
    try:
      def load_from_bundles(*paths):
        [services_dict.update(self.get_services_from_bundle(path)) for path in paths]

      # If a path to the services bundle was set in the config file, load services from the current bundle and the specified path.
      if self._core.config.services_bundle_path:
        self._core.log.debug('Services path specified - loading services directly from bundles')
        load_from_bundles(
          self._core.bundle_path,
          self._core.config.services_bundle_path,
        )
        
      # If the plug-in is daemonized, only load services from the current bundle and Services.bundle.
      elif self._core.config.daemonized:
        self._core.log.debug('Plug-in is daemonized - loading services directly from bundles')
        load_from_bundles(
          self._core.bundle_path,
          self._core.storage.join_path(self._core.app_support_path, 'bundle', 'com.plexapp.system.services'),
        )
        
      # Otherwise, get the list of services from System.bundle, falling back to just the current bundle if something goes wrong.
      else:
        self._core.log.debug('Plug-in is not daemonized - loading services from system')
        try:
          services_dict = self._core.messaging.call_external_function(
            '..system',
            '_BundleService:AllServices'
          )
        except:
          self._core.log.error('Unable to load services from system. Loading from the current bundle only.')
          load_from_bundles(
            self._core.bundle_path,
          )
        
      for identifier in services_dict:
        try:
          service_dict = services_dict[identifier]
          path = service_dict['Path']
          services = service_dict.get('Services', {})
          service_sets = service_dict.get('ServiceSets', {})
          old_services = service_dict.get('OldServices', {})
          self._service_flags.update(service_dict.get('ServiceFlags', {}))
        
          # Helper function for adding a service in a safe manner
          def add_service(root_dict, service_identifier, s_class, s_name, path_components, **kwargs):
            # Make sure a service set exists in the root dictionary for this identifier
            if service_identifier not in root_dict:
              root_dict[service_identifier] = {}
            
            try:
              root_dict[service_identifier][s_name] = s_class(
                self._core,
                self._core.storage.join_path(path, 'Contents', *path_components),
                s_name,
                service_identifier,
                **kwargs
              )
              #self._core.log.debug("Loaded %s service '%s' in %s from bundle %s", str(path_components), s_name, service_identifier, identifier)
            
            except:
              self._core.log_exception("Error loading %s service '%s' in %s from bundle %s", str(path_components), s_name, service_identifier, identifier)

            # If the service matches the core identifier and a resources folder exists, make sure the core sandbox knows about it.
            if service_identifier == self._core.identifier:
              resource_path = self._core.storage.abs_path(self._core.storage.join_path(path, '..', '..', 'Resources'))
              if resource_path not in self._core.sandbox.resource_paths:
                self._core.sandbox.resource_paths.append(resource_path)

        
          def load_service_dict(dct, service_identifier, path_components, old_style=False):
            for service_name, service in dct.get('URL', {}).items():
              if old_style:
                service_identifier = service['Identifier']
              add_service(
                self._url_services,
                service_identifier,
                URLServiceRecord,
                service_name,
                path_components + (['URL Services'] if old_style else ['URL']),
                patterns = service.get('URLPatterns', list()),
                fallback = service.get('Fallback', False),
                source_title = service.get('SourceTitle', None),
                test_urls = service.get('TestURLs', []),
                priority = int(service.get('Priority', 50))
              )
            
            for service_name, service in dct.get('Search', {}).items():
              if old_style:
                service_identifier = service['Identifier']
              add_service(
                self._search_services,
                service_identifier,
                SearchServiceRecord,
                service_name,
                path_components + (['Search Services'] if old_style else ['Search']),
                source_title = service.get('SourceTitle', None),
                test_queries = service.get('TestQueries', []),
              )
            
            for service_name, service in dct.get('RelatedContent', {}).items():
              if old_style:
                service_identifier = service['Identifier']
              add_service(
                self._related_content_services,
                service_identifier,
                'Related Content',
                RelatedContentServiceRecord,
                service_name,
                path_components + (['Related Content Services'] if old_style else ['Related Content']),
                source_title = service.get('SourceTitle', None)
              )
            
          load_service_dict(services, identifier, ['Services'])
          [load_service_dict(dct, service_identifier, ['Service Sets', service_identifier]) for service_identifier, dct in service_sets.items()]
          load_service_dict(old_services, identifier, [], old_style=True)
        
        except:
          self._core.log_exception("Error loading services from '%s'", identifier)
      
      self._core.log.debug('Loaded services')

    except:
      self._core.log_exception("Error loading services")
    
    finally:
      self._services_loaded.set()


  def _setup_shared_code_sandbox(self):
    # Clear the event while loading.
    self._shared_code_loaded.clear()

    # Wait for services to load so we have the correct flags.
    self._services_loaded.wait()

    # Set up a separate sandbox for shared code
    service_flags = self._service_flags.get(self._core.identifier, [])
    sandbox = Framework.code.Sandbox(self._core, None, Framework.policies.ServicePolicy, service_flags)
    
    self.root_paths = []
    self.shared_mtimes = {}

    try:
      # Get all owned services (any with a matching identifier)
      owned_services = []
      for dct in [self.url_services, self.search_services, self.related_content_services]:
        for identifier, services in dct.items():
          if identifier == self._core.identifier:
            for name, service in services.items():
              owned_services.append(service)

      # Find unique root paths
      resource_paths = []
      for service in owned_services:
        root_path = self._core.storage.abs_path(self._core.storage.join_path(service.path, '..', '..', 'Shared Code'))
        resource_path = self._core.storage.abs_path(self._core.storage.join_path(service.path, '..', '..', 'Resources'))
        if root_path not in self.root_paths:
          self.root_paths.append(root_path)
        if resource_path not in resource_paths:
          resource_paths.append(resource_path)
      
      # Add these paths to the sandbox
      sandbox.custom_paths.extend(self.root_paths)

      # Add the service resources path to the relevant sandboxes.
      for path in resource_paths:
        sandbox.resource_paths.insert(0, path)
        if path not in self._core.sandbox.resource_paths:
          self._core.sandbox.resource_paths.insert(0, path)

      # Override functions in the sandbox
      sandbox.environment['Callback'] = self._generate_shared_service_callback_path
      
      # Synthesise code to import any shared modules and execute it if some were found.
      setup_str = ''

      mod_count = 0
      for root_path in self.root_paths:
        if self._core.storage.dir_exists(root_path):
          for filename in self._core.storage.list_dir(root_path):
            if filename.endswith('.pys'):
              module_name = filename[:-4]
              setup_str += 'import %s\n' % module_name
              mod_count += 1

              full_path = self._core.storage.join_path(root_path, filename)
              mtime = self._core.storage.last_modified(full_path)
              self.shared_mtimes[full_path] = mtime

      if mod_count > 0:
        self._core.log.debug("Loading %d shared code modules", mod_count)
        code = compile(setup_str, '<shared>', 'exec')
        sandbox.execute(code)
        self._core.log.debug("Loaded shared code")
      else:
        self._core.log.debug("No shared code to load")

    except:
      self._core.log_exception("Error loading shared code")

    # Assign the sandbox object and unblock any waiting threads.
    finally:
      self._shared_code_sandbox = sandbox
      self._shared_code_loaded.set()


  @property
  def shared_code_modified(self):
    # Check known shared source file mtimes and return True if any of the current mtimes don't match.
    for path in self.shared_mtimes:
      last_mtime = self.shared_mtimes[path]
      current_mtime = self._core.storage.last_modified(path)
      if last_mtime != current_mtime:
        return True
    return False

  def handle_shared_code_modification(self):
    if self.shared_code_modified:
      self._core.log.debug("Detected a change to shared code files - reloading the shared code sandbox and associated services.")
      self._setup_shared_code_sandbox()

      # Kill mtimes for all services to force a reload.
      for service_identifier, service in (self.url_services.items() + self.search_services.items() + self.related_content_services.items()):
        for service_name, service_record in service.items():
          service_record._mtime = 0

  def _generate_shared_service_callback_path(self, function, ext=None, post_url=None, post_headers={}, *args, **kwargs):
    """ Generate a callback path for a function inside a service """
    # Check for an IndirectFunction and extract the actual function from the object if necessary
    if isinstance(function, Framework.components.runtime.IndirectFunction):
      function = function._f
      indirect = True
    else:
      indirect = False
      
    # Format the extension, if provided
    if ext and ext[0] != '.':
      ext = '.'+ext
    elif ext == None:
      ext = ''

    # Get the module name
    module_name = function.func_globals['__name__']

    path = "/:/plugins/%s/sharedServiceFunction/%s/%s%s?args=%s&kwargs=%s" % (
      self._core.identifier,
      urllib.quote(module_name),
      function.__name__,
      ext,
      Framework.utils.pack(args),
      Framework.utils.pack(kwargs)
    )
    
    # Convert to a callback_string object
    if indirect:
      path = Framework.components.runtime.indirect_callback_string(path + "&indirect=1")
    else:
      path = Framework.components.runtime.callback_string(path)
      
    # Add POST args
    path.post_url = post_url
    path.post_headers = post_headers
      
    return path

  @property
  def shared_code_sandbox(self):
    self._shared_code_loaded.wait()
    self.handle_shared_code_modification()
    return self._shared_code_sandbox
    
  @property
  def url_services(self):
    self._services_loaded.wait()
    return self._url_services
    
  @property
  def search_services(self):
    self._services_loaded.wait()
    return self._search_services
    
  @property
  def related_content_services(self):
    self._services_loaded.wait()
    return self._related_content_services

  def get_all_services(self, identifier=None):
    l = []
    def add(dct):
      for ident, services in dct.items():
        if identifier in (None, ident):
          for name, service in services.items():
            l.append(service)
    add(self.url_services)
    add(self.search_services)
    add(self.related_content_services)
    return l

    
  def _type_for_service(self, service):
    if isinstance(service, URLServiceRecord):
      service_type = 'url'
    elif isinstance(service, SearchServiceRecord):
      service_type = 'search'
    elif isinstance(service, RelatedContentServiceRecord):
      service_type = 'related_content'
    else:
      service_type = 'unknown'
    return service_type


  def function_in_service_is_deferred(self, fname, service):
    # Copy the current request to the service's execution context.
    values = self._core.sandbox.context.export_values()
    service.sandbox.context.import_values(values)
    
    # Check whether the function is deferred.
    return service.sandbox.function_is_deferred(fname)

  def _call_named_function_in_sandbox(self, sandbox, fname, mod_name, f_args=None, f_kwargs=None, allow_deferred=False, raise_exceptions=True, f_optional=None):
    if f_args == None: f_args = []
    if f_kwargs == None: f_kwargs = {}
    if f_optional == None: f_optional = {}
    
    # Copy the current request to the service's execution context.
    values = self._core.sandbox.context.export_values()
    sandbox.context.import_values(values)
    
    # Call the named function and return the result.
    if fname != 'NormalizeURL' or fname in sandbox.environment:
      result = sandbox.call_named_function(fname, allow_deferred=allow_deferred, raise_exceptions=raise_exceptions, args=f_args, kwargs=f_kwargs, mod_name=mod_name, optional_kwargs=f_optional)
    else:
      result = None
    return result

  def _call_named_function_in_shared_service_code(self, fname, mod_name, f_args=None, f_kwargs=None, allow_deferred=False, raise_exceptions=True):
    if f_args == None: f_args = []
    if f_kwargs == None: f_kwargs = {}
    
    return self._call_named_function_in_sandbox(self.shared_code_sandbox, fname, mod_name, f_args, f_kwargs, allow_deferred, raise_exceptions)

  def _call_named_function_in_service(self, fname, service, f_args=None, f_kwargs=None, allow_deferred=False, raise_exceptions=True, f_optional=None):
    """ Call a named function in the given service with the given arguments and return the result """
    if f_args == None: f_args = []
    if f_kwargs == None: f_kwargs = {}
    if f_optional == None: f_optional = {}
    return self._call_named_function_in_sandbox(service.sandbox, fname, None, f_args, f_kwargs, allow_deferred, raise_exceptions, f_optional)

  def service_for_url(self, url):
    """ Find the path to a service that can handle the given URL """
    # Iterate over service sets
    for service_identifier, service_set in self.url_services.items():
      # Iterate over each service in the set
      for service_name, service in service_set.items():
        # If the service has URL patterns defined, see if the URL matches a pattern, and return
        # the service if so
        if service.patterns:
          for pattern in service.patterns:
            if pattern.match(url):
              return service
    self._core.log.debug("No service found for URL '%s'", url)
    return None

  def all_services_for_url(self, url):
    """
      Returns a list containing a service matching the given url (if found) and all
      fallback services.
    """
    services = list()
    service = self.service_for_url(url)
    
    if service != None:
      services.append(service)
      self._core.log.debug("Found a service matching '%s' - %s (%s)", url, service.name, service.identifier)
    else:
      self._core.log.debug("No matching services found for '%s'", url)
      pass

    fallbacks = self.fallback_url_services()
    #self._core.log.debug("There are %d fallback services", len(fallbacks))
    services.extend(fallbacks)

    return services

  def fallback_url_services(self):
    """ Returns a list of all URL services with their fallback attribute set to True """
    l = list()
    for service_identifier, service_set in self.url_services.items():
      for service_name, service in service_set.items():
        if service.fallback:
          l.append(service)

    # Sort the list of services by their priority and reverse it so the service with the highest priority is first.
    l.sort(key = lambda service: service.priority)
    
    return l

  def metadata_object_for_url_from_service(self, url, service, add_items_automatically=True, allow_deferred=True, in_container=False):
    # Call the function in the service
    metadata = self._call_named_function_in_service(METADATA_OBJECT_FUNCTION_NAME, service, [url])
    if metadata == None:
      return None
      
    # Set the 'url' attribute of the returned object to the given URL
    setattr(metadata, 'url', url)
    
    # If the media objects function is deferred, set the deferred attribute
    if hasattr(metadata, 'deferred') and self.function_in_service_is_deferred(MEDIA_OBJECTS_FUNCTION_NAME, service):
      metadata.deferred = True
    
    # If no items were provided by the developer, and this item expects children, populate them using the service
    # Although this is also done when serializing to XML in ObjectKit, we do it here to because we already
    # have the exact service we want to use, and going through URL matching again is undesirable.
    if add_items_automatically and isinstance(metadata, Framework.modelling.objects.ModelInterfaceObject) and len(type(metadata)._child_types) > 0 and len(metadata) == 0 and (metadata.deferred == False or allow_deferred):
      result = self.media_objects_for_url_from_service(url, service, allow_deferred = allow_deferred, metadata_class = type(metadata))
      if result:
        for item in result:
          metadata.add(item)
          
    # Reset the deferred flag if deferring is allowed
    if hasattr(metadata, 'deferred') and metadata.deferred and allow_deferred:
      metadata.deferred = False

    # Set the source title if one isn't provided
    if metadata.source_title == None:
      if service.source_title:
        metadata.source_title = service.source_title
      else:
        metadata.source_title = service.name
        
    # Set the art if not provided.
    if metadata.art == None:
      if self._core.runtime.hash_for_hosted_resource('image', 'art', identifier=service.identifier):
        metadata.art = self._core.runtime.hosted_resource_url('image', 'art', identifier=service.identifier)
        
    if in_container:
      container = service.sandbox.environment['ObjectContainer']()
      container.add(metadata)
      return container

    else:
      return metadata

  def media_objects_for_url_from_service(self, url, service, allow_deferred=False, metadata_class=None):
    return self._call_named_function_in_service(MEDIA_OBJECTS_FUNCTION_NAME, service, [url], allow_deferred=allow_deferred, f_optional=dict(metadata_class=metadata_class))

  def lookup_url_for_media_url(self, url, syncable=False):
    url = self.normalize_url(url)
    prefix = 'https://node.plexapp.com:32443' if self._core.config.use_node_for_url_lookups else ''
    lookup_url = prefix + '/system/services/url/lookup?url='+urllib.quote(url, '')

    # Add the syncable argument if required.
    if syncable:
      lookup_url += '&syncable=1'

    return lookup_url
  
  def normalize_url(self, url):
    try:
      n_url = self._get_result_from_function_for_url(self._normalize_url_in_service, url)
    except:
      self._core.log_exception("Error when attempting to normalize URL '%s'", url)
      return None
    if n_url == None:
      self._core.log.info("No normalization function found for URL '%s'", url)
      n_url = url
    else:
      #self._core.log.debug("Normalized URL '%s' to '%s'", url, n_url)
      pass
    return n_url

  def _normalize_url_in_service(self, url, service):
    return self._call_named_function_in_service(NORMALIZE_URL_FUNCTION_NAME, service, [url])
  
  def media_objects_for_url(self, url, allow_deferred=True, do_normalization = True, metadata_class=None):
    return self._get_result_from_function_for_url(self.media_objects_for_url_from_service, url, do_normalization, dict(allow_deferred=allow_deferred, metadata_class=metadata_class))
  
  def metadata_object_for_url(self, url, add_items_automatically=True, allow_deferred=True, in_container=False, do_normalization=True):
    """
      Calls MetadataObjectForURL in the appropriate URL service and returns the result, setting
      the original URL on the object and populating media items if none were provided.
    """
    return self._get_result_from_function_for_url(self.metadata_object_for_url_from_service, url, do_normalization, dict(add_items_automatically = add_items_automatically, allow_deferred = allow_deferred, in_container = in_container))
    
  def _get_result_from_function_for_url(self, func, url, do_normalization = True, kwargs={}):
    if func != self._normalize_url_in_service and do_normalization:
      url = self.normalize_url(url)
      if url == None:
        return None
    
    # Iterate over services, calling the given function in each until one is able to return a response.
    services = self.all_services_for_url(url)
    for service in services:
      try:
        # Check for a deferred media objects function
        if 'allow_deferred' in kwargs and kwargs['allow_deferred'] == False and func == self.media_objects_for_url_from_service and self.function_in_service_is_deferred(MEDIA_OBJECTS_FUNCTION_NAME, service):
          return None
          
        result = func(url, service, **kwargs)
        if result != None:
          return result
      except:
        self._core.log.critical('Exception calling function in URL service "%s" (%s)', service.name, str(service.identifier))
        raise
        
    return None
    
  def search(self, query, identifier, name=None):
    if name == None:
      name = self.search_services[identifier].keys()[0]
    service = self.search_services[identifier][name]
    result = self._call_named_function_in_service(SEARCH_FUNCTION_NAME, service, f_kwargs = dict(query=query))

    if result != None:
      result.identifier = identifier
      try:
        if result.art == None and self._core.runtime.hash_for_hosted_resource('image', 'art', identifier=identifier):
          result.art = self._core.runtime.hosted_resource_url('image', 'art', identifier=identifier)
      except:
        pass
      
      # Set the source_title attribute of child objects if not set by the service
      for metadata in result.objects:
        if metadata.source_title == None:
          if service.source_title:
            metadata.source_title = service.source_title
          else:
            metadata.source_title = service.name
        
    return result

  def related_content_for_metadata(self, metadata):
    service = self._related_content_services[identifier][name]
    result = self._call_named_function_in_service(RELATED_CONTENT_FUNCTION_NAME, service, f_kwargs = dict(metadata=metadata))
    
    # Set the source_title attribute of child objects if not set by the service
    for metadata in result.objects:
      if metadata.source_title == None:
        if service.source_title:
          metadata.source_title = service.source_title
        else:
          metadata.source_title = service.name
    
    return result
