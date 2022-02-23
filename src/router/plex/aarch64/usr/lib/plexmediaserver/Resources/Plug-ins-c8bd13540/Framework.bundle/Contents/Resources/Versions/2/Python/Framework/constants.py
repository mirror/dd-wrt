#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework


class interface(Framework.ConstantGroup):

  pipe = 'pipe'
  socket = 'socket'


  
class context(Framework.ConstantGroup):

  media = 'media'
  agent = 'agent'


  
class header(Framework.ConstantGroup):

  client_capabilities = 'X-Plex-Client-Capabilities'
  client_platform = 'X-Plex-Platform'
  client_platform_old = 'X-Plex-Client-Platform'
  client_version = 'X-Plex-Version'
  transaction_id = 'X-Plex-Transaction-Id'
  language = 'X-Plex-Language'
  preferences = 'X-Plex-Preferences'
  proxy_cookies = 'X-Plex-Proxy-Cookies'
  product = 'X-Plex-Product'
  token = 'X-Plex-Token'
  container_start = 'X-Plex-Container-Start'
  container_size = 'X-Plex-Container-Size'
  container_total_size = 'X-Plex-Container-Total-Size'



class arguments(Framework.ConstantGroup):
  token = 'auth_token'



class flags(Framework.ConstantGroup):

  use_xpython = 'UseExtendedPython'
  use_myplex_dev_server = 'UserMyPlexDevServer'

  enable_debugging = 'EnableDebugging'

  log_route_connections = 'LogRouteConnections'
  log_all_route_connections = 'LogAllRouteConnections'
  log_service_loads = 'LogServiceLoads'
  log_model_class_generation = 'LogModelClassGeneration'
  log_metadata_combination = 'LogMetadataCombination'

  syslog_peer_service = 'SystemVerboseLogPeerService'
  syslog_store_service = 'SystemVerboseLogStoreService'



class plugin_class(Framework.ConstantGroup):

  content = 'Content'
  agent = 'Agent'
  channel = 'Channel'
  resource = 'Resource'
  system = 'System'
  