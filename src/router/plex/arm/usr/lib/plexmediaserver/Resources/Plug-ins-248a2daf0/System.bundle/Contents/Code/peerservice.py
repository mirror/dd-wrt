#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService
import os

# TODO: Verify this works, remove eventually when compatibility with old Frameworks isn't an issue.
try:
  VERBOSE = Framework.constants.flags.syslog_peer_service in Core.sandbox.flags
except:
  VERBOSE = hasattr(Core, 'host') and hasattr(Core.host, '_flags') and 'SystemLogVerbosePeerService' in getattr(Core.host, '_flags')

class Server(object):
  def __init__(self, identifier, name, version, address, port):
    self.identifier = identifier
    self.name = name
    self.version = version
    self.address = address
    self.port = port
    
    self.channels = dict()
    self.sections = dict()

  def update_dict(self, attr, url, attr_map):
    if VERBOSE:
      Log.Debug("Updating %s dictionary", attr)

    # Copy the current dictionary
    dct = dict(getattr(self, attr))

    # Create a list to store valid keys
    keys = list()

    # Get information from the server
    try:
      main_el = XML.ElementFromURL(url, cacheTime = 0)
      el_list = main_el.xpath('//Directory')
      if VERBOSE:
        Log.Debug('Found %d %s', len(el_list), attr)

      for el in el_list:
        attrs = None
        try:
          attrs = el.attrib
          key = attrs['key']

          kwargs = dict()
          for name in attr_map:
            kwargs[name] = attrs.get(attr_map[name])
          
          # Create a new instance if we don't have one for this key already
          if key not in dct:
            if VERBOSE:
              Log.Debug("Creating a new record with key %s", key)
            instance = dct[key] = dict(**kwargs)

          # If one already exists, lock it while we update the values
          else:
            if VERBOSE:
              Log.Debug("Updating an existing record with key %s", key)
            instance = dct[key]
            @lock(instance)
            def modify_instance():
              for k, v in kwargs.items():
                instance[k] = v
              
          # Add to the list of keys
          keys.append(key)
        
        except:
          Log.Exception("Exception adding record to %s with attributes %s", attr, str(attrs))
      
      # Remove sections that are no longer available
      for key in dct.keys():
        if key not in keys:
          if VERBOSE:
            Log.Debug("Removing instance with key '%s'", key)
          del dct[key]

    except Ex.URLError, e:
      Log.Error("Failed to fetch data from %s: %s", url, str(e.args))

    except:
      Log.Exception("Failed to fetch data from %s", url)

    # Replace the current dictionary
    @lock(self)
    def modify_self():
      setattr(self, attr, dct)
      
  @property
  def local(self):
    return self.identifier == Platform.MachineIdentifier
      
  @property
  def host(self):
    return '127.0.0.1' if self.local else self.address

  def update(self, sections=True, channels=True):
    # Update the section dictionary
    if sections:
      self.update_dict(
        'sections',
        'http://%s:%s/library/sections' % (self.host, self.port),
        dict(
          title = 'title',
          section_type = 'type',
          art = 'art',
          thumb = 'thumb',
          agent = 'agent',
          scanner = 'scanner',
          language = 'language',
          updated_at = 'updatedAt',
          uuid = 'uuid',
        )
      )

    # Update the channel dictionary
    if channels:
      self.update_dict(
        'channels',
        'http://%s:%s/channels/all' % (self.host, self.port),
        dict(
          title = 'title',
          art = 'art',
          thumb = 'thumb',
          store_services = 'hasStoreServices',
          prefs = 'hasPrefs',
          identifier = 'identifier',
          platforms = 'platforms',
          protocols = 'protocols'
        )
      )
    

class PeerService(SystemService):
  def __init__(self, system):
      SystemService.__init__(self, system)
      Log.Debug("Starting the peer service")
    
      self.servers = dict()
      self.refreshed = Thread.Event()

      Route.Connect('/system/search', self.get_search_providers)
      Route.Connect('/system/notify/serverUpdated', self.handle_notify)

      if not Core.config.daemonized:
        Route.Connect("/system/library/sections", self.get_sections)
        for name in ('channels', 'plugins'):
          Route.Connect("/system/%s/all" % name, self.get_channels)
          for prefix in ['music', 'video', 'photos', 'applications']:
            Route.Connect("/system/%s/%s" % (name, prefix), self.get_channels, prefix='/' + prefix)
          
        Thread.Create(self.refresh_servers)
        Thread.CreateTimer(120, self.refresh_servers)

  @property    
  def environment_secure(self):
    return len(os.environ.get('PLEXTOKEN', '')) > 0
      
  def get_servers_el(self):
    Log.Debug("Fetching the current list of servers")
    return XML.ElementFromURL('http://127.0.0.1:32400/servers', cacheTime = 0)
    

  def refresh_servers(self):
    servers_el = self.get_servers_el()

    Log.Debug("Refreshing local server")
    self.update_servers(servers_el, local_only=True)

    @spawn
    def background_refresh():
      Log.Debug("Background-refreshing remote servers")
      self.update_servers(servers_el, remote_only=True)

      # Flag that we've refreshed at least once
      self.refreshed.set()


  def update_servers(self, servers_el=None, local_only=False, remote_only=False, info=True, sections=True, channels=True, required_attribs={}):
    # If we have a token, only include the local server in the list.
    if self.environment_secure:
        local_only = True
        remote_only = False
        
    Log.Debug("Updating servers (%s/%s %s/%s/%s)", str(local_only), str(remote_only), str(info), str(sections), str(channels))
    requires_update = lambda identifier: (local_only == False and remote_only == False) or (local_only == True and identifier == Platform.MachineIdentifier) or (remote_only == True and identifier != Platform.MachineIdentifier)
    
    # Get the list of servers from this PMS if we weren't given one
    if info:
      if servers_el == None:
        servers_el = self.get_servers_el()

      server_el_list = servers_el.xpath('//Server')
      Log.Debug("Found %d servers", len(server_el_list))

      # Copy the current server dictionary and create an empty list to store identifiers
      servers = dict(self.servers)
      identifiers = list()

      for el in server_el_list:
        attrs = None
        try:
          attrs = el.attrib
          identifier = attrs['machineIdentifier']

          # Check whether we need to do an update for this identifier
          attribs_match = True
          for key in required_attribs:
            if key not in attrs or attrs[key] != required_attribs[key]:
              Log.Debug("Ignoring %s because %s != %s (%s)", identifier, key, required_attribs[key], attrs[key])
              attribs_match = False
              break

          if attribs_match and requires_update(identifier):
            name = attrs.get('name')
            version = attrs.get('version')
            address = attrs.get('address')
            port = attrs.get('port')

            # Fix for Bonjour-only hosts on Windows
            if address == None:
              address = Core.networking.resolve_hostname_if_required(attrs['host'])
              port = 32400

            if identifier not in servers:
              Log.Debug("Creating new server %s (%s)", name, identifier)
              server = servers[identifier] = Server(identifier, name, version, address, port)
            else:
              Log.Debug("Updating existing server %s (%s)", name, identifier)
              server = servers[identifier]
              @lock(server)
              def modify_server():
                server.name = name
                server.version = version
                server.address = address
                server.port = port

          # Add this to the list of "seen" identifiers
          identifiers.append(identifier)
        except:
          Log.Exception("Exception adding server with attributes %s (XML: %s)", str(attrs), XML.StringFromElement(el))

      # Remove servers that have disappeared
      for identifier in servers.keys():
        if identifier not in identifiers:
          Log.Debug("Removing server with identifier '%s'", identifier)
          del servers[identifier]

      @lock(self)
      def modify_self():
        self.servers = servers

    # Update any servers that meet the given requirements
    servers = dict(self.servers)
    if channels or sections:
      for identifier in servers:
        if requires_update(identifier):
          servers[identifier].update(sections, channels) 



  def get_directory(self, attr, attr_map={}, literals={}, key_format='%s', prefix=None, update_kwargs={}):
    # Wait for the initial refresh to complete
    self.refreshed.wait()

    self.update_servers(local_only=True, **dict(update_kwargs))
    items = list()
    title_counts = dict()

    client_platform = Client.Platform
    client_protocols = Client.Protocols
    cap_check = 'X-Plex-Disable-Capability-Checking' not in Request.Headers

    @lock(self)
    def modify_self():
      servers = dict(self.servers)
      for identifier in servers:
        server = servers[identifier]
        @lock(server)
        def modify_server():
          if VERBOSE:
            Log.Debug("Iterating over %s from server %s", attr, identifier)
          dct = getattr(server, attr)
          for key in dct:
            instance = dct[key]
            @lock(instance)
            def modify_instance():
              if prefix == None or key.startswith(prefix):
                platforms_str = instance.get('platforms')
                platforms = platforms_str.split(',') if platforms_str else []
                protocols_str = instance.get('protocols')
                protocols = protocols_str.split(',') if protocols_str else []
                
                # Check capabilities if required
                if cap_check:
                  if len(platforms) > 0 and '*' not in platforms and client_platform != None and client_platform not in platforms:
                    if VERBOSE:
                      Log.Debug("Excluding %s (client platform %s not in compatible platforms %s)", key, client_platform, str(platforms))
                    return
                  
                  if len(protocols) > 0:
                    for protocol in protocols:
                      if protocol not in client_protocols:
                        if VERBOSE:
                          Log.Debug("Excluding %s (required capability %s not in client caps %s)", key, protocol, str(client_protocols))
                        return

                if VERBOSE:
                  Log.Debug("Adding %s", key)

                item = DirectoryItem(
                  key = String.Encode("http://%s:%s%s" % (server.address, server.port, (key_format % key))),
                  title = instance['title'],
                  host = server.address,
                  port = server.port,
                  serverName = server.name,
                  serverVersion = server.version,
                  machineIdentifier = identifier,
                  path = key_format % key,
                  art = instance['art'],
                  thumb = instance['thumb'],
                  local = server.local,

                )
                # Map items from instance properties to XML attributes
                for k, v in attr_map.items():
                  setattr(item, v, instance[k])
                
                # Set literal XML attributes
                for k, v in literals.items():
                  setattr(item, k, v)
                
                # Append the item and track the title count
                items.append(item)
                title_counts[instance['title']] = title_counts.get(instance['title'], 0) + 1
    
    items.sort(key = lambda x: x.title.lower())

    mc = MediaContainer()
    for item in items:
      item.unique = title_counts[item.title] == 1
      mc.Append(item)
    return mc

  
  def get_channels(self, prefix=None):
    return self.get_directory(
      attr = 'channels',
      attr_map = dict(
        identifier = 'identifier',
        prefs = 'hasPrefs',
        store_services = 'hasStoreServices',
      ),
      literals = dict(
        share = True,
        type = 'plugin',
      ),
      update_kwargs = dict(
        sections = False,
        info = False
      ),
      prefix = prefix
    )


  def get_sections(self):
    return self.get_directory(
      attr = 'sections',
      key_format = '/library/sections/%s',
      attr_map = dict(
        section_type = 'type',
        uuid = 'uuid',
      ),
      update_kwargs = dict(
        channels = False,
        info = False
      )
    )


  def handle_notify(self, host=None):
    Log.Debug("Handling server update notification (%s)", str(host))
    attrs = {}
    if host != None:
      attrs['host'] = host
    self.update_servers(required_attribs=attrs)
    return ''



  def get_search_providers(self, query=None):
    c = ObjectContainer()
    
    if not Core.config.daemonized:
      self.update_servers(local_only=True, channels=False, sections=False)

      servers = dict(self.servers)  
    
      for identifier in self.servers:
        if identifier != Platform.MachineIdentifier:
          server = self.servers[identifier]
          @lock(server)
          def modify_server():
            c.add(ProviderObject(
              title = server.name,
              key = 'http://%s:%s/search?local=1' % (server.address, server.port),
              type = 'mixed',
              machine_identifier = identifier
            ))
    
    if query != None and len(query) > 2:
      for identifier in Core.services.search_services:
        for name in Core.services.search_services[identifier]:
          c.add(ProviderObject(
            title = name,
            key = '/system/:/services/search?identifier=%s&name=%s' % (identifier, String.Quote(name)),
            type = 'mixed'
          ))
      
    return c
