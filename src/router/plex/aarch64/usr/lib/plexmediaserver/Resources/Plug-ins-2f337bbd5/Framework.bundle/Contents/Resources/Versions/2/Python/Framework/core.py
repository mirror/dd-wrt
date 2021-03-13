#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

import os
import plistlib
import logging
import logging.handlers
import traceback
import sys
import signal
import urllib
import simplejson
import re

import policies
import components
import handlers


POLICY_KEY                        = 'PlexPluginCodePolicy'
WHITELIST_KEY                     = 'PlexPluginModuleWhitelist'
API_EXCLUSION_KEY                 = 'PlexPluginAPIExclusions'
CONSOLE_LOGGING_KEY               = 'PlexPluginConsoleLogging'
LOG_LEVEL_KEY                     = 'PlexPluginLogLevel'
CLASS_KEY                         = 'PlexPluginClass'
AUDIO_CODEC_KEY                   = 'PlexAudioCodec'
VIDEO_CODEC_KEY                   = 'PlexVideoCodec'
MEDIA_CONTAINER_KEY               = 'PlexMediaContainer'
MINIMUM_SERVER_VERSION_KEY        = 'PlexMinimumServerVersion'
FLAGS_KEY                         = 'PlexFrameworkFlags'
BUNDLE_VERSION_KEY                = 'PlexBundleVersion'
BUNDLE_COMPAT_VERSION_KEY         = 'PlexBundleCompatibleVersion'
TITLE_KEY                         = 'PlexPluginTitle'
ICON_RESOURCE_NAME_KEY            = 'PlexPluginIconResourceName'
ART_RESOURCE_NAME_KEY             = 'PlexPluginArtResourceName'
TITLE_BAR_RESOURCE_NAME_KEY       = 'PlexPluginTitleBarResourceName'
LEGACY_PREFIX_KEY                 = 'PlexPluginLegacyPrefix'
CLIENT_PLATFORM_EXCLUSIONS_KEY    = 'PlexClientPlatformExclusions'

DEFAULT_ART_RESOURCE_NAME         = 'art-default.jpg'
DEFAULT_ICON_RESOURCE_NAME        = 'icon-default.png'
DEFAULT_TITLE_BAR_RESOURCE_NAME   = 'titlebar-default.png'

START_FUNCTION_NAME               = 'Start'
MAIN_FUNCTION_NAME                = 'Main'


class LogFilter(logging.Filter):
  def filter(self, record):
    return 0



class TransactionLogFilter(logging.Filter):

  def __init__(self, core):
    logging.Filter.__init__(self)
    self._core = core

  def filter(self, record):
    txn_id = self._core.sandbox.context.txn_id
    if txn_id:
      record.msg = '[T:%s] %s' % (txn_id, record.msg)
    return record
  


class LogFormatter(logging.Formatter):

  def format(self, record):
    for key in record.__dict__:
      if key[0] != '_' and isinstance(record.__dict__[key], str):
        record.__dict__[key] = uni(record.__dict__[key])
    return logging.Formatter.format(self, record)


class PerRequestLogHandler(logging.Handler):

  def __init__(self, core):
    logging.Handler.__init__(self)
    self._core = core

  def emit(self, record):
    if hasattr(self._core, 'sandbox') and self._core.sandbox.context.request != None:
      self._core.sandbox.context.log.append(self.format(record))
    


class FrameworkLogger(logging.Logger):

  def findCaller(self):
    """
    Find the stack frame of the caller so that we can note the source
    file name, line number and function name.
    """
    f = logging.currentframe()
    #On some versions of IronPython, currentframe() returns None if
    #IronPython isn't run with -X:Frames.
    if f is not None:
      f = f.f_back
    rv = "(unknown file)", 0, "(unknown function)"
    while hasattr(f, "f_code"):
      co = f.f_code
      filename = os.path.normcase(co.co_filename)
      if filename == logging._srcfile or (filename.endswith('/core.py') and co.co_name.startswith('log_')) or (filename.endswith('/api/logkit.py')):
        f = f.f_back
        continue
      rv = (filename, f.f_lineno, co.co_name)
      break
    return rv



class FrameworkCore(object):
  def __init__(self, bundle_path, framework_path, config_module, plist=None):

    # Initialize attributes.
    self.config = config_module
    self.whitelist = list(self.config.module_whitelist)
    self.attributes = {}
    
    # Perform setup functions.
    self._setup_paths(bundle_path, framework_path)
    self._setup_framework()
    self._setup_bundle(bundle_path, plist)
    self._setup_storage()
    self._setup_policy()
    self._setup_components()
    self._setup_models()
    self._setup_libraries()
    self._setup_code()
    self._setup_handlers()
    self._setup_services()

    # Get information about the current server.
    self.runtime.create_thread(self.get_server_info)
    
    # Add logging filters.
    self.log.addFilter(TransactionLogFilter(self))

    # Get the last known Framework version, and store the current one if different
    self.last_version = self.get_value('LastVersion')
    if self.last_version != self.version:
      self.set_value('LastVersion', self.version)

    self.log.debug("Finished starting framework core")


  def _setup_paths(self, bundle_path, framework_path):
    self.framework_path = framework_path
    
    if bundle_path:
      self.bundle_path = bundle_path
      self.code_path = os.path.join(bundle_path, 'Contents', 'Code')
      self.init_path = os.path.join(self.code_path, '__init__.py')
    else:
      import tempfile
      self.bundle_path = tempfile.gettempdir()
      self.code_path = None
      self.init_path = None

    if self.config.root_path:
      self.app_support_path = self.config.root_path
      
      # Set defaults if not provided
      if not self.config.bundles_dir_name:
        self.config.bundles_dir_name = self.config.bundle_files_dir
      if not self.config.plugin_support_dir_name:
        self.config.plugin_support_dir_name = self.config.plugin_support_files_dir
    
    else:
      if 'PLEX_MEDIA_SERVER_APPLICATION_SUPPORT_DIR' in os.environ:
        self.app_support_path = os.path.join(os.environ['PLEX_MEDIA_SERVER_APPLICATION_SUPPORT_DIR'], 'Plex Media Server')
      elif sys.platform.find('linux') == 0 and 'PLEXLOCALAPPDATA' in os.environ:
        self.app_support_path = os.path.join(os.environ['PLEXLOCALAPPDATA'], 'Plex Media Server')
      elif sys.platform == "win32":
        if 'PLEXLOCALAPPDATA' in os.environ:
          key = 'PLEXLOCALAPPDATA'
        else:
          key = 'LOCALAPPDATA'
        self.app_support_path = os.path.join(os.environ[key], 'Plex Media Server')
      else:
        self.app_support_path = os.path.join(os.environ["HOME"], 'Library', 'Application Support', 'Plex Media Server')
        
      # Set defaults if not provided
      if not self.config.bundles_dir_name:
        self.config.bundles_dir_name = 'Plug-ins'
      if not self.config.plugin_support_dir_name:
        self.config.plugin_support_dir_name = 'Plug-in Support'

    self.plugin_support_path = os.path.join(self.app_support_path, self.config.plugin_support_dir_name)
    self.bundled_plugins_path = os.environ['PLEXBUNDLEDPLUGINSPATH'] if 'PLEXBUNDLEDPLUGINSPATH' in os.environ else None
    
  def _setup_framework(self):
    # Read the framework plist file
    framework_plist_path = os.path.abspath(os.path.join(self.framework_path, '..', '..', '..', 'Info.plist'))
    framework_plist = plistlib.readPlist(framework_plist_path)
    self.version = framework_plist[BUNDLE_VERSION_KEY]
    self.compatible_version = framework_plist.get(BUNDLE_COMPAT_VERSION_KEY, '2.0a1')
    self.build_info = 'Unavailable'
    

  def _setup_bundle(self, bundle_path, plist=None):
    # Read the bundle plist file.
    if plist:
      self.plist_path = None
      bundle_plist = plist
    elif bundle_path:
      self.plist_path = os.path.join(self.bundle_path, 'Contents', 'Info.plist')
      bundle_plist = plistlib.readPlist(self.plist_path)
    else:
      print "ERROR: No bundle or plist provided"
      
    self.identifier = bundle_plist["CFBundleIdentifier"]
    self.title = bundle_plist.get(TITLE_KEY)
    self.icon_resource_name = bundle_plist.get(ICON_RESOURCE_NAME_KEY, DEFAULT_ICON_RESOURCE_NAME)
    self.art_resource_name = bundle_plist.get(ART_RESOURCE_NAME_KEY, DEFAULT_ART_RESOURCE_NAME)
    self.title_bar_resource_name = bundle_plist.get(TITLE_BAR_RESOURCE_NAME_KEY, DEFAULT_TITLE_BAR_RESOURCE_NAME)

    self.api_exclusions = list(bundle_plist.get(API_EXCLUSION_KEY, []))
    self.plugin_class = bundle_plist.get(CLASS_KEY, Framework.constants.plugin_class.content)
    self.flags = bundle_plist.get(FLAGS_KEY, [])
    self.policy_name = bundle_plist.get(POLICY_KEY, 'Standard')
    self.legacy_prefix = bundle_plist.get(LEGACY_PREFIX_KEY)

    # Get the list of excluded client platforms from the plist. We use these later to warn users that the channel
    # is unsupported.
    client_platform_exclusions = bundle_plist.get(CLIENT_PLATFORM_EXCLUSIONS_KEY)
    if isinstance(client_platform_exclusions, basestring):
        self.client_platform_exclusions = client_platform_exclusions.split(',')
    elif isinstance(client_platform_exclusions, list):
        self.client_platform_exclusions = client_platform_exclusions
    else:
        self.client_platform_exclusions = []

    try:
      self.minimum_server_version = [int(x) for x in bundle_plist[MINIMUM_SERVER_VERSION_KEY].split('.')] if MINIMUM_SERVER_VERSION_KEY in bundle_plist else None
    except:
      self.log_exception("Exception parsing minimum version key.")
      self.minimum_server_version = None

    # Copy default codec and container attributes from the plist
    [self.copy_attribute(bundle_plist, attr_name) for attr_name in (AUDIO_CODEC_KEY, VIDEO_CODEC_KEY, MEDIA_CONTAINER_KEY)]

    # Override config variables.
    if CONSOLE_LOGGING_KEY in bundle_plist: self.config.console_logging = str(bundle_plist[CONSOLE_LOGGING_KEY]) == '1'
    if LOG_LEVEL_KEY in bundle_plist: self.config.log_level = bundle_plist[LOG_LEVEL_KEY]  

    # Check for a whitelist in the plist file (if allowed)
    self.bundle_whitelist = bundle_plist.get(WHITELIST_KEY)
    

  def _setup_storage(self):
    # Start the storage component.
    self._start_component(components.Storage)

    # Set up logging.
    logging.basicConfig()
    logging.setLoggerClass(FrameworkLogger)
    logger = logging.getLogger()
    logger.handlers[0].addFilter(LogFilter())
    
    self.log = logging.getLogger(self.identifier)
    
    if self.config.log_level == 'Critical':
      self.log.setLevel(logging.CRITICAL)
    elif self.config.log_level == 'Error':
        self.log.setLevel(logging.ERROR)
    elif self.config.log_level == 'Warning':
      self.log.setLevel(logging.WARNING)
    elif self.config.log_level == 'Info':
      self.log.setLevel(logging.INFO)
    else:
      self.log.setLevel(logging.DEBUG)
    
    if self.config.log_file:
      log_dir = os.path.dirname(self.config.log_file)
    elif self.config.root_path:
      log_dir = os.path.join(self.config.root_path, self.config.log_files_dir)
    elif 'PLEX_MEDIA_SERVER_LOG_DIR' in os.environ:
      log_dir = os.path.join(os.environ['PLEX_MEDIA_SERVER_LOG_DIR'], 'PMS Plugin Logs')
    elif sys.platform.find('linux') == 0 and 'PLEXLOCALAPPDATA' in os.environ:
      log_dir = os.path.join(os.environ['PLEXLOCALAPPDATA'], 'Plex Media Server', 'Logs', 'PMS Plugin Logs')
    elif sys.platform == 'win32':
      if 'PLEXLOCALAPPDATA' in os.environ:
        key = 'PLEXLOCALAPPDATA'
      else:
        key = 'LOCALAPPDATA'
      log_dir = os.path.join(os.environ[key], 'Plex Media Server', 'Logs', 'PMS Plugin Logs')
    else:
      log_dir = os.path.join(os.environ['HOME'], 'Library', 'Logs', 'Plex Media Server', 'PMS Plugin Logs')

    log_config = dict(identifier = self.identifier, port = self.config.socket_interface_port)
    if self.config.root_path:
      log_file = os.path.join(log_dir, '%(identifier)s.%(port)d.log' % log_config)
    elif not self.config.log_file:
      log_file = os.path.join(log_dir, self.identifier+'.log')
    else:
      log_file = self.config.log_file % log_config
    
    self.storage.ensure_dirs(log_dir)
    rollover = os.path.exists(log_file)
    
    if self.config.console_logging == True:
      console_handler = logging.StreamHandler()
      console_formatter = LogFormatter('%(asctime)-15s - %(name)-32s (%(thread)x) :  %(levelname)s (%(module)s:%(lineno)d) - %(message)s')
      console_handler.setFormatter(console_formatter)
      self.log.addHandler(console_handler)
    
    file_handler = logging.handlers.RotatingFileHandler(log_file, mode='w', maxBytes=1048576, backupCount=5)
    file_formatter = LogFormatter('%(asctime)-15s (%(thread)x) :  %(levelname)s (%(module)s:%(lineno)d) - %(message)s')
    file_handler.setFormatter(file_formatter)
    
    if rollover:
      try:
        file_handler.doRollover()
      except:
        self.log_exception('Exception performing logfile rollover')
        
    self.log.addHandler(file_handler)

    # Set up per-request logging.
    per_request_handler = PerRequestLogHandler(self)
    per_request_formatter = LogFormatter('%(levelname)s (%(module)s:%(lineno)d) - %(message)s')
    per_request_handler.setFormatter(per_request_formatter)
    self.log.addHandler(per_request_handler)

    # Set up stored values.
    self._values = {}
    self._values_file_path = os.path.join(self.plugin_support_path, 'Data', self.identifier, 'StoredValues')
    if os.path.exists(self._values_file_path):
      try:
        data = self.storage.load(self._values_file_path)
        stored_values = simplejson.loads(data)
        self._values.update(stored_values)
      except:
        pass

    try:
      version_path = os.path.abspath(os.path.join(self.framework_path, '..', '..', '..', 'VERSION'))
      self.build_info = self.storage.load(version_path).strip()
    except:
      self.build_info = 'No build information available'
    
    self.log.info('Starting framework core - Version: %s, Build: %s', self.version, self.build_info)


  def _setup_policy(self):
    # Decide which security policy to use
    self.policy = policies.StandardPolicy
    
    policy_key = self.policy_name + 'Policy'
    if hasattr(policies, policy_key):
      self.policy = getattr(policies, policy_key)
      #TODO: Check the code signature

    self.log.debug("Using the %s policy", self.policy_name.lower())

    # Check for a whitelist from the bundle (if allowed)
    if self.policy.allow_whitelist_extension and self.bundle_whitelist:
      self.log.debug("Extending whitelist: %s", str(self.bundle_whitelist))
      self.whitelist.extend(self.bundle_whitelist)


  def _setup_libraries(self):
    # If we're loading code from a bundle, and loading external libraries is allowed, add the paths to sys.path
    if self.policy.allow_bundled_libraries and self.bundle_path:
      sys.path.insert(0, os.path.join(self.bundle_path, 'Contents', 'Libraries', self.runtime.os, self.runtime.cpu).encode('utf-8'))
      sys.path.insert(0, os.path.join(self.bundle_path, 'Contents', 'Libraries', 'Shared').encode('utf-8'))


  def _setup_components(self):
    self.loader = Framework.code.Loader()

    # Start each of the core components.
    [self._start_component(component) for component in [
      components.Runtime,
      components.Caching,
      components.Data,
      components.Networking,
      components.Localization,
      components.Messaging,
      components.Debugging,
      components.Services,
      components.MyPlex,
      components.Notifications,
    ]]


  def _setup_handlers(self):
    # Install request handlers.
    [self.runtime.install_handler(handler) for handler in [
      handlers.PluginRequestHandler,
      handlers.RuntimeRequestHandler,
      handlers.ResourceRequestHandler,
      handlers.ServiceRequestHandler,
      handlers.MessagingRequestHandler,
    ]]

    if Framework.constants.flags.enable_debugging in self.flags:
      self.runtime.install_handler(Framework.handlers.DebugRequestHandler)

    if self.policy.synthesize_defaults == True:
      self.runtime.add_prefix_handler(
        prefix = self.channel_prefix,
        handler = self._call_main,
        name = self.title,
        thumb = None,
        art = None,
        titleBar = None,
        share = False,
      )


  def _setup_code(self):
    # Create a code sandbox for the bundle code
    self.sandbox = Framework.code.Sandbox(self, self.code_path, self.policy, self.flags)
    self.sandbox.whitelist.extend(self.whitelist)
    self.sandbox.resource_paths.insert(0, self.storage.join_path(self.bundle_path, 'Contents', 'Resources'))
    assert self.sandbox.conforms_to_policy(Framework.policies.BundlePolicy), "The core code policy must conform to BundlePolicy."

    # Backwards-compatibility code for things that dig around inside the core. Remove this ASAP.
    self.host = self.sandbox


  def _setup_models(self):
    # Create a metadata model accessor for this plug-in
    metadata_root_path = self.config.root_path if self.config.root_path != None else self.app_support_path
    self._metadata_model_accessor = Framework.modelling.ModelAccessor(
      self,
      'metadata',
      self.storage.join_path(self.framework_path, 'Models', 'Metadata', '__init__.pym'),
      self.storage.join_path(metadata_root_path, 'Metadata')
    )


  def _setup_services(self):
    self.services_bundle_path = self.path_for_bundle(self.config.services_bundle_name, self.config.services_bundle_identifier)
    self.services.load()


  def _start_component(self, component):
    name = component.__name__.lower()
    try:
      if component != components.Storage:
        self.log.debug("Starting %s component.", name)
      setattr(self, name, component(self))
    except:
      if component != components.Storage:
        self.log_exception("Exception starting %s component.", name)
      else:
        raise


  def _call_main(self, *args, **kwargs):
    result = self.sandbox.call_named_function(MAIN_FUNCTION_NAME)
    
    # For modern policies, do some container processing for backwards compatibility with old clients.
    is_modern = self.sandbox.conforms_to_policy(Framework.policies.BundlePolicy, Framework.policies.ModernPolicy)
    is_container = isinstance(result, self.sandbox.environment.get('ObjectContainer'))
    is_old_client = True

    if is_modern and is_container and is_old_client:
      if self.identifier in self.services.search_services:
        services = self.services.search_services[self.identifier]
        if len(services) > 1:
          params = [(name, self.localization.localize("Search %s...") % name, self.localization.localize("Search %s") % name) for name in services]
        elif len(services) == 1:
          params = [(services.keys()[0], self.localization.localize("Search..."), self.localization.localize("Search %s") % services.keys()[0])]
        else:
          params = []

        for name, title, prompt in params:
          result.add(self.sandbox.environment['SearchDirectoryObject'](
            identifier = self.identifier,
            name = name,
            title = title,
            prompt = prompt,
          ))

      # TODO: Enable this when modern channels support prefs.
      # Synthesize a PrefsObject so old clients can get to preferences.
      #if self.storage.file_exists(self.sandbox.preferences.default_prefs_path):
      #  result.add(self.sandbox.environment['PrefsObject'](title=self._core.localization.localize("Preferences...")))

      pass

    return result


  @property
  def channel_prefix(self):
    return self.legacy_prefix if (self.config.daemonized == False and self.legacy_prefix != None) else '/plugins/' + self.identifier


  def server_version_at_least(self, *args):
    if self.config.daemonized:
      return True
      
    version = self.get_server_attribute('serverVersion')
    return Framework.utils.version_at_least(version, *args)
    

  def get_server_attribute(self, name, default=None):
    # Return the cached attribute if we have it already
    if name in self.attributes and self.attributes[name] != None:
      return self.attributes[name]
      
    # Otherwise, try to get it from the server
    root_xml = self.data.xml.from_string(self.networking.http_request('http://127.0.0.1:32400'))
    root_el_list = root_xml.xpath('//MediaContainer')
    try:
      if len(root_el_list) > 0:
        root_el = root_el_list[0]
        attr = root_el.get(name)
        if attr != None:
          self.log.debug("Attribute '%s' set to '%s'", name, attr)
          self.attributes[name] = attr
        else:
          self.log.debug("Unable to read server attribute '%s'", name)
    except:
      self.log_exception("Exception reading server attribute '%s'", name)
        
    return self.attributes.get(name, default)
    

  def get_server_info(self):
    # Get the machine identifier and server version
    try:
      root_xml = self.data.xml.from_string(self.networking.http_request('http://127.0.0.1:32400'))
      root_el = root_xml.xpath('//MediaContainer')[0]
      self.attributes['machineIdentifier'] = root_el.get('machineIdentifier')
      self.attributes['serverVersion'] = root_el.get('version')
      self.log.debug("Machine identifier is %s", self.attributes.get('machineIdentifier'))
      self.log.debug("Server version is %s", self.attributes.get('serverVersion'))
    except:
      self.log.warn('Unable to retrieve the machine identifier or server version.')
    

  def get_value(self, key, default=None):
    return self._values.get(key, default)
    

  def set_value(self, key, value):
    self._values[key] = value
    json_str = self.data.json.to_string(self._values)
    self.storage.save(self._values_file_path, json_str)
    

  def copy_attribute(self, bundle_plist, attr_name):
    if attr_name in bundle_plist:
      self.attributes[attr_name] = [x.replace('.', '').lower() for x in bundle_plist[attr_name]]
    

  def load_code(self, elevated=False):
    self.log.debug("Loading plug-in code")
    try:
      if self.storage.file_exists(self.init_path):
        self.init_code = self.loader.load(self.init_path, elevated, use_xpython = Framework.constants.flags.use_xpython in self.sandbox.flags)
      else:
        self.init_code = None
      self.log.debug("Finished loading plug-in code")
      return True
    except:
      self.log_exception('Exception while loading code')
      return False
    

  def log_exception(self, fmt, *args):
    self.log.critical(self.traceback(fmt % tuple(args)))


  def log_except(self, txn_id, fmt, *args):
    #TODO: Backwards compatibility; remove eventually
    self.log.critical(self.traceback(fmt % tuple(args)))
    

  def log_stack(self):
    stack = ''
    lines = traceback.format_stack()[3:-3]
    for line in lines:
      if sys.prefix not in line:
        stack += '  %s\n' % line.strip()
    self.log.debug("Current stack:\n" + stack)
    

  def traceback(self, msg='Traceback'):
    exceptionType, exceptionValue, exceptionTraceback = sys.exc_info()
    full_entry_list = traceback.extract_tb(exceptionTraceback)
    if not self.config.show_internal_traceback_frames:
      entry_list = []
      for entry in full_entry_list:
        if not ('/%s.bundle/' % self.config.framework_bundle_name in entry[0] or '/%s/' % self.config.framework_bundle_identifier in entry[0]):
          entry_list.append(entry)
    else:
      entry_list = full_entry_list
    traceback_str = ''.join(traceback.format_list(entry_list)) + ''.join(traceback.format_exception_only(exceptionType, exceptionValue))
    return "%s (most recent call last):\n%s" % (msg, traceback_str)
    

  def start(self):
    try:
      if self.init_code:
        self.sandbox.execute(self.init_code)
        
      self.sandbox.call_named_function(START_FUNCTION_NAME)
      self.log.info("Started plug-in")
      return True
      
    except:
      self.log_exception("Exception starting plug-in")
      return False
      

  def path_for_bundle(self, name, identifier):
    # Check if a cloud path exists - if not, use the local naming format
    if self.bundled_plugins_path is not None:
      bundles_path = self.bundled_plugins_path
    else:
      bundles_path = self.storage.join_path(self.app_support_path, self.config.bundles_dir_name)
    cloud_path = self.storage.join_path(bundles_path, identifier)
    if self.storage.dir_exists(cloud_path):
      return cloud_path
    return self.storage.join_path(bundles_path, name + '.bundle')


  @property
  def debug(self):
    Framework.constants.flags.enable_debugging in self.flags

  def prefs_available(self, identifier, services=None):
    if identifier == self.identifier:
      has_bundle_prefs = self.storage.file_exists(self.storage.join_path(self.bundle_path, 'Contents', 'DefaultPrefs.json'))
    else:
      has_bundle_prefs = False

    if identifier != 'com.plexapp.system':
      if services is None:
        services = self.services.get_all_services(identifier)
      service_paths = [self.storage.join_path(service.path, 'ServicePrefs.json') for service in services]
      has_service_prefs = True in [self.storage.file_exists(path) for path in service_paths]
    else:
      has_service_prefs = False

    return has_bundle_prefs or has_service_prefs

