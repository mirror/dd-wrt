#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

@handler('/system', "System")
def Main():
  con = MediaContainer()
  con.Append(DirectoryItem('library', 'Library Sections'))
  con.Append(DirectoryItem('plugins', 'Plug-ins'))
  return con
  
def ServiceMain():
  pass

def PlayerMain():
  return ''

import messageservice
import agentservice
import scannerservice
import bundleservice
import playerservice
import flagservice
import peerservice
import streamservice
import codeservice
import proxyservice

class System(Object):
  def __init__(self):
    # Read and store the revision
    try:
      version_data = Core.storage.load(Core.storage.join_path(Core.bundle_path, 'Contents', 'VERSION'))
      Log("Starting System %s", version_data.strip())
      self.revision = version_data.split(' ')[0]
    except:
      self.revision = None
      Log.Exception('Error reading bundle revision')
    
    # Attempts to start the given service, catching and logging any exceptions
    def start_service(service_name, service_class, should_start=True):
      if should_start:
        try:
          setattr(self, service_name + 'service', service_class(self))
        except:
          Core.log_exception("Error starting %s service", service_name)
    
    node = hasattr(Core.config, 'daemonized') and Core.config.daemonized
    
    # If we're on the node, add a prefix for /services too
    if node:
      Plugin.AddPrefixHandler('/services', ServiceMain, 'Services')
    else:
      Plugin.AddPrefixHandler('/player', PlayerMain, 'Player')
    
    start_service('message',    messageservice.MessageService,    True)
    start_service('agent',      agentservice.AgentService,        True)
    start_service('scanner',    scannerservice.ScannerService,    not node)
    start_service('bundle',     bundleservice.BundleService,      not node)
    start_service('player',     playerservice.PlayerService,      not node)
    start_service('flag',       flagservice.FlagService,          True)
    start_service('peer',       peerservice.PeerService,          True)
    start_service('stream',     streamservice.StreamService,      not node)
    start_service('code',       codeservice.CodeService,          True)
    start_service('proxy',      proxyservice.ProxyService,        not node)

    if not node:
      self.agentservice.update_attribution_flags()
    
system_instance = None

def Start():
  global system_instance
  system_instance = System()
  Plugin.Nice(15)

def ValidatePrefs():
  pass
