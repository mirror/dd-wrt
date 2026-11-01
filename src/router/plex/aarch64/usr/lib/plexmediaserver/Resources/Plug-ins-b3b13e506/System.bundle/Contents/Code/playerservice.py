#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService
from urllib import urlencode

class PlayerService(SystemService):
  def __init__(self, system):
    SystemService.__init__(self, system)
    Log.Debug("Starting the player service")
    
    Route.Connect("/player/{player_controller}/{command}", self.process_remote_command)
    Route.Connect("/system/players/{player}/{player_controller}/{command}", self.process_legacy_remote_command)


  def process_remote_command(self, player_controller, command, **kwargs):
    identifier = Request.Headers['X-Plex-Target-Client-Identifier']
    el = XML.ElementFromURL('http://127.0.0.1:32400/clients')
    for child_el in el:
      if child_el.get('machineIdentifier') == identifier:
        host = child_el.get('host')
        port = child_el.get('port')
        url = 'http://%s:%s/player/%s/%s' % (host,
                                             port,
                                             player_controller,
                                             command)
        if (len(kwargs) > 0):
          query = urlencode(kwargs)
          url += '?' + query

        Log.Debug('Final remote command URL: %s', url)

        player_response = HTTP.Request(url,
                                       immediate=True,
                                       timeout=600,
                                       headers=Request.Headers)

        header = 'X-Plex-Client-Identifier'
        if header in player_response.headers:
          Response.Headers[header] = player_response.headers[header]
        else:
          Response.Headers[header] = identifier
                                                  
        Response.Headers['Access-Control-Expose-Headers'] = header

        return player_response.content

    raise FrameworkException('Unable to find player with identifier %s' % identifier)


  def process_legacy_remote_command(self, player, player_controller, command, **kwargs):
    try:
      # Capitalise the first letter of the args so they match the attribute names
      player_controller = player_controller[0].upper()+player_controller[1:]
      command = command[0].upper()+command[1:]

      # Get a player object
      player_obj = Player[player]

      # Find & call the requested function
      if hasattr(player_obj, player_controller):
        controller = getattr(player_obj, player_controller)
        if hasattr(controller, command):
          cmd = getattr(controller, command)
          result = cmd(**kwargs)

          if player_controller.lower() == 'application' and command.lower() == 'screenshot':
            return DataObject(result, 'image/jpeg')

          else:
            return ''

        else:
          Log("%s has no attribute named '%s'" % (repr(controller), command))
      else:
        Log("%s has no controller named '%s'" % (repr(player_obj), player_controller))

    except FrameworkException:
      Log("Unable to access player '%s'" % player)
