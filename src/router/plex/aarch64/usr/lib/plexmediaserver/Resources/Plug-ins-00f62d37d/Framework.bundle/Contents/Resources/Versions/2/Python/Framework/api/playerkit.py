#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import socket, struct, base64
import urllib
from socket import AF_INET, SOCK_DGRAM
import uuid, weakref

from base import BaseKit

#TODO: Push client list down from PMS when changes detected via bonjour, and only allow valid clients to be used
#TODO: Screenshots, key chars

class PlayerCommand(object):
  def __init__(self, command, **kwargs):
    self._command = command
    self._kwargs = kwargs
    
  def _function_arg_str(self, **kwargs):
    function_args = dict(self._kwargs)
    function_args.update(kwargs)
    formatted_args = self._format_args(**function_args)
    if len(formatted_args) > 0:
      return '(%s)' % formatted_args
    else:
      return ""
    
  def _format_args(self, **kwargs):
    return ""
    
  def _format_response(self, response):
    return response
    
  def __call__(self, **kwargs):
    pass


class UDPCommand(PlayerCommand):  

  def __call__(self, **kwargs):
    cmd = self._command + self._function_arg_str(**kwargs)
    self._player._send_udp_command(cmd)
    
    
class HTTPCommand(PlayerCommand):  

  def __call__(self, **kwargs):
    cmd = self._command + self._function_arg_str(**kwargs)
    return self._format_response(self._player._send_http_command(cmd))

    
class PlayFileCommand(HTTPCommand):  
  def __init__(self, **kwargs):
    HTTPCommand.__init__(self, 'PlayFile', **kwargs)
    
  def _format_args(self, path, userAgent=None, httpCookies=None):
    ret = path
    if userAgent or httpCookies:
      ret += ';'
      if userAgent:
        ret += userAgent
      ret += ';'
      if httpCookies:
        ret += httpCookies
    return ret


class PlayMediaCommand(HTTPCommand):  
  def __init__(self, **kwargs):
    HTTPCommand.__init__(self, 'PlayMedia', **kwargs)

  def _format_args(self, path, key, userAgent=' ', httpCookies=' ', viewOffset=' '):
    ret = path + ';' + key
    if userAgent or httpCookies or viewOffset:
      ret += ';'
      if userAgent:
        ret += userAgent.replace(' ','+')
      ret += ';'
      if httpCookies:
        ret += httpCookies
      ret += ';'
      if viewOffset:
        ret += viewOffset
    return ret
    
class SetVolumeCommand(HTTPCommand):
  def __init__(self, **kwargs):
    HTTPCommand.__init__(self, 'setvolume', **kwargs)
    
  def _format_args(self, level):
    return str(level)
  
    
class ScreenshotCommand(HTTPCommand):
  def __init__(self, **kwargs):
    HTTPCommand.__init__(self, 'takescreenshot', **kwargs)
  
  def _format_args(self, width=480, height=270, quality=75):
    return ";false;0;%s;%s;%s;true" % (str(width), str(height), str(quality))
  
  def _format_response(self, response):
    return base64.b64decode(response[7:-8])
  

class SendKeyCommand(PlayerCommand):
  def __init__(self, virtual=False):
    PlayerCommand.__init__(self, 'SendKey')
    if virtual:
      self._base_code = 0xF100
    else:
      self._base_code = 0xF000
      
  def __call__(self, code):
    cmd = 'SendKey(%s)' % str(hex(self._base_code + int(code)))
    self._player._send_http_command(cmd)

  
class SendStringCommand(SendKeyCommand):
  def __init__(self):
    SendKeyCommand.__init__(self, virtual=True)
    
  def __call__(self, text):
    for char in text:
      SendKeyCommand.__call__(self, ord(char))
      
      
class PlayerController(object):
  def __init__(self, player, **commands):
    self._player = player
    for name in commands:
      commands[name]._player = player
    self.command_dict = commands

  def __getattr__(self, name):
    if name in self.command_dict:
      return self.command_dict[name]


class PlayerInstance(object):
  def __init__(self, core, host, udp_port=9777, http_port=3000):
    self._cmd_fmt = "\x58\x42\x4D\x43\x02\x00\x00\x0A\x00\x00\x00\x01\x00\x00\x00\x01%s\x49\x9D\xFA\x3A\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02%s\x00"
    self._socket = socket.socket(AF_INET, SOCK_DGRAM)
    self._core = core
    self._host = self._core.networking.resolve_hostname_if_required(host)
    self._udp_port = udp_port
    self._http_port = http_port
    
    self.Navigation   = PlayerController(self,
      MoveUp          = UDPCommand("Up"),
      MoveDown        = UDPCommand("Down"),
      MoveLeft        = UDPCommand("Left"),
      MoveRight       = UDPCommand("Right"),
      PageUp          = UDPCommand("PageUp"),
      PageDown        = UDPCommand("PageDown"),
      NextLetter      = UDPCommand("NextLetter"),
      PreviousLetter  = UDPCommand("PrevLetter"),
      Select          = UDPCommand("Select"),
      Back            = UDPCommand("ParentDir"),
      ContextMenu     = UDPCommand("ContextMenu"),
      ToggleOSD       = UDPCommand("OSD")
    )
    
    self.Playback     = PlayerController(self,
      Play            = UDPCommand("Play"),
      Pause           = UDPCommand("Pause"),
      Stop            = UDPCommand("Stop"),
      Rewind          = UDPCommand("Rewind"),
      FastForward     = UDPCommand("FastForward"),
      StepForward     = UDPCommand("StepForward"),
      BigStepForward  = UDPCommand("BigStepForward"),
      StepBack        = UDPCommand("StepBack"),
      BigStepBack     = UDPCommand("BigStepBack"),
      SkipNext        = UDPCommand("SkipNext"),
      SkipPrevious    = UDPCommand("SkipPrevious")
    )
    
    self.Application  = PlayerController(self,
      PlayFile        = PlayFileCommand(),
      PlayMedia       = PlayMediaCommand(),
      SetVolume       = SetVolumeCommand(),
      Screenshot      = ScreenshotCommand(),
      SendString      = SendStringCommand(),
      SendKey         = SendKeyCommand(virtual=False),
      SendVirtualKey  = SendKeyCommand(virtual=True),
    )
  
  def _send_udp_command(self, cmd):
    self._socket.sendto(self._cmd_fmt % (struct.pack(">H", len(cmd)+2), cmd), (self._host, self._udp_port))
    
  def _send_http_command(self, cmd):
    response = self._core.networking.http_request(("http://%s:%s/xbmcCmds/xbmcHttp?command=" % (self._host, self._http_port)) + urllib.quote(cmd), cacheTime=0, immediate=True)
    return response.content

class JSONCommand(PlayerCommand):
  def __call__(self, **kwargs):
    function_args = dict(kwargs)
    function_args.update(kwargs)
    params = self._format_params(**function_args)

    defaultdata = {"jsonrpc":"2.0", "id":str(uuid.uuid4().int)[:6]}
    defaultdata.update(self._format_json(self._command))
    if len(params) > 0:
      if "params" in defaultdata:
        defaultdata["params"].update(params)
      else:
        defaultdata["params"]=params

    return self._format_response(self._player._send_http_command(defaultdata))

  def _format_params(self, **kwargs):
    return {}

  def _format_json(self, cmd):
    return {"method":cmd}

class JSONPlayerCommand(JSONCommand):
  def _format_params(self, **kwargs):
    # FIXME: might be "wrong" to hardcode playerid to 1 here, but I have never seen another id.
    return {"playerid":1}

class JSONSetSpeedCommand(JSONCommand):
  def _format_json(self, cmd):
    return {"method":"Player.SetSpeed", "params": {"playerid": 1, "speed": int(cmd)}}

class JSONPlayerGoToCommand(JSONCommand):
  def _format_json(self, cmd):
    return {"method":"Player.GoTo", "params": {"playerid": 1, "to": cmd}}

class JSONSetVolumeCommand(JSONCommand):
  def _format_params(self, level):
    return {"volume":int(level)}

class JSONSendStringCommand(JSONCommand):
  def _format_params(self, text):
    return {"text":text}

class JSONPlayMediaCommand(JSONCommand):
  def _format_params(self, path, key, userAgent=None, httpCookies=None, viewOffset=None):
    data = {"path":path, "key":key}
    if userAgent: data["userAgent"] = userAgent
    if httpCookies: data["httpCookies"] = httpCookies
    if viewOffset: data["viewOffset"] = viewOffset
    return data

class XBMCJSONPlayerInstance(PlayerInstance):
  def __init__(self, core, host, udp_port=9778, http_port=3000):
    PlayerInstance.__init__(self, core, host, udp_port, http_port)

    self.Navigation   = PlayerController(self,
      MoveUp          = JSONCommand("Input.Up"),
      MoveDown        = JSONCommand("Input.Down"),
      MoveLeft        = JSONCommand("Input.Left"),
      MoveRight       = JSONCommand("Input.Right"),
      PageUp          = JSONCommand("Input.PageUp"),
      PageDown        = JSONCommand("Input.PageDown"),
      NextLetter      = JSONCommand("Input.NextLetter"),
      PreviousLetter  = JSONCommand("Input.PrevLetter"),
      Select          = JSONCommand("Input.Select"),
      Back            = JSONCommand("Input.Back"),
      ContextMenu     = JSONCommand("Input.ContextMenu"),
      ToggleOSD       = JSONCommand("Input.ShowOSD")
    )

    self.Playback     = PlayerController(self,
      Play            = JSONPlayerCommand("Player.PlayPause"),
      Pause           = JSONPlayerCommand("Player.PlayPause"),
      Stop            = JSONPlayerCommand("Player.Stop"),
      Rewind          = JSONSetSpeedCommand("-2"),
      FastForward     = JSONSetSpeedCommand("2"),
      StepForward     = JSONPlayerCommand("Input.StepForward"),
      BigStepForward  = JSONPlayerCommand("Input.BigStepForward"),
      StepBack        = JSONPlayerCommand("Input.StepBackward"),
      BigStepBack     = JSONPlayerCommand("Input.BigStepBackward"),
      SkipNext        = JSONPlayerGoToCommand("next"),
      SkipPrevious    = JSONPlayerGoToCommand("previous")
    )

    self.Application  = PlayerController(self,
      PlayMedia       = JSONPlayMediaCommand("Player.PlexPlayMedia"),
      SetVolume       = JSONSetVolumeCommand("Application.SetVolume"),
      SendString      = JSONSendStringCommand("Input.SendText"),
#      SendKey         = SendKeyCommand(virtual=False),
#      SendVirtualKey  = SendKeyCommand(virtual=True),
    )

  def _send_http_command(self, cmd):
    jsondata = self._core.data.json.to_string(cmd)
    response = self._core.networking.http_request(("http://%s:%s/jsonrpc" %(self._host, self._http_port)), cacheTime=0, immediate=True, data=jsondata, headers={"Content-type":"application/json"})

class RokuEcpCommand(PlayerCommand):
  def __call__(self, **kwargs):
    cmd = self._command
    self._core = self._player._core

    return self._format_response(self._player._send_ecp_command(cmd))


class RokuHttpCommand(PlayerCommand):
  def __call__(self, **kwargs):
    cmd = self._command
    headers = self._format_headers(**kwargs)
    self._core = self._player._core

    return self._format_response(self._player._send_http_command(cmd, headers))

  # Using headers instead of the query string is pretty lame, but at the moment
  # the web server on the Roku has no facility for interpreting the query string
  # and processes headers by default. Since we're building a custom request for
  # the Roku anyway, we might as well make it easy for the Roku to process.

  def _format_headers(self, **kwargs):
    return {}


class RokuPlayMediaCommand(RokuHttpCommand):
  def __init__(self, **kwargs):
    RokuHttpCommand.__init__(self, 'PlayMedia', **kwargs)

  def _format_headers(self, **kwargs):
    headers = {}
    args = dict(self._kwargs)
    args.update(kwargs)

    for name in ('key', 'path', 'userAgent', 'httpCookies', 'viewOffset'):
      if name in args:
        headers['X-Plex-Arg-' + name] = urllib.quote(args[name])

    return headers


class RokuPlayerInstance(object):
  def __init__(self, core, host, http_port=8324, ecp_port=8060):
    self._core = core
    self._host = self._core.networking.resolve_hostname_if_required(host)
    self._http_port = http_port
    self._ecp_port = ecp_port

    self.Navigation   = PlayerController(self,
      MoveUp          = RokuEcpCommand("Up"),
      MoveDown        = RokuEcpCommand("Down"),
      MoveLeft        = RokuEcpCommand("Left"),
      MoveRight       = RokuEcpCommand("Right"),
      Select          = RokuEcpCommand("Select"),
      Back            = RokuEcpCommand("Back"),
      ContextMenu     = RokuEcpCommand("Info"),
      ToggleOSD       = RokuEcpCommand("Info")
    )

    self.Playback     = PlayerController(self,
      Play            = RokuEcpCommand("Play"),
      Pause           = RokuEcpCommand("Play"),
      Stop            = RokuHttpCommand("Stop"),
      Rewind          = RokuEcpCommand("Rev"),
      SkipPrevious    = RokuEcpCommand("Rev"),
      FastForward     = RokuEcpCommand("Fwd"),
      SkipNext        = RokuEcpCommand("Fwd"),
      StepBack        = RokuEcpCommand("InstantReplay")
    )

    self.Application  = PlayerController(self,
      PlayMedia       = RokuPlayMediaCommand()
    )

  def _send_ecp_command(self, cmd):
    response = self._core.networking.http_request("http://%s:%s/keypress/%s" % (self._host, self._ecp_port, cmd), method='POST', cacheTime=0)
    return response.content

  def _send_http_command(self, cmd, headers):
    response = self._core.networking.http_request("http://%s:%s/application/%s" % (self._host, self._http_port, cmd), method='POST', cacheTime=0, headers=headers)
    return response.content

class PassthroughPlayerController(object):
  def __init__(self, player, name, actions):
    self._player = weakref.proxy(player)
    self._name = name
    self._actions = actions
    
  def __hasattr__(self, name):
    return name in self._actions
    
  def __getattr__(self, name):
    def f(**kwargs):
      # Construct the URL.
      url = "http://%s:%s/player/%s/%s" % (self._player._host, self._player._port, self._name, name[0].lower() + name[1:])
      
      # Build and append a query string if required.
      if len(kwargs) > 0:
        url += '?' + '&'.join(['%s=%s' % (k, urllib.quote(v)) for k, v in kwargs.items()])
        
      # Make the request and return the response.
      req = self._player._core.networking.http_request(url = url, cacheTime=0)
      return req.content
    return f

class PassthroughPlayerInstance(object):
  def __init__(self, core, host, port):
    self._core = core
    self._host = host
    self._port = port
    
    self.Application = PassthroughPlayerController(self, 'application', [
      'PlayFile',
      'PlayMedia',
      'SetVolume',
      'Screenshot',
      'SendString',
      'SendKey',
      'SendVirtualKey'
    ])
    
    self.Navigation = PassthroughPlayerController(self, 'navigation', [
      'MoveUp',
      'MoveDown',
      'MoveLeft',
      'MoveRight',
      'PageUp',
      'PageDown',
      'NextLetter',
      'PreviousLetter',
      'Select',
      'Back',
      'ContextMenu',
      'ToggleOSD'
    ])
    
    self.Playback = PassthroughPlayerController(self, 'playback', [
      'Play',
      'Pause',
      'Stop',
      'Rewind',
      'FastForward',
      'StepForward',
      'BigStepForward',
      'StepBack',
      'BigStepBack',
      'SkipNext',
      'SkipPrevious'
    ])
  

class PlayerKit(BaseKit):

  _included_policies = [
    Framework.policies.BundlePolicy,
  ]

  _excluded_policies = [
    Framework.policies.ModernPolicy,
  ]

  def _init(self):
    self._instances = dict()
    self._client_info = dict()

  def _refresh_client_info(self):
    try:
      clients = self._core.data.xml.from_string(self._core.networking.http_request("http://127.0.0.1:32400/clients", cacheTime=0))
      for client_el in clients.xpath('//Server'):
        self._client_info[client_el.get('host')] = {
          'name': client_el.get('name'),
          'port': int(client_el.get('port')),
          'protocol': client_el.get('protocol', '')
        }
    except:
      pass

  def __getitem__(self, name):
    if name not in self._instances:
      if name not in self._client_info:
        self._refresh_client_info()

      if name in self._client_info and self._client_info[name]['protocol'] == "roku":
        self._instances[name] = RokuPlayerInstance(self._core, name, http_port=self._client_info[name]['port'])
      elif name in self._client_info and self._client_info[name]['protocol'] == "xbmcjson":
        self._instances[name] = XBMCJSONPlayerInstance(self._core, name, http_port=self._client_info[name]['port'])
      elif name in self._client_info and self._client_info[name]['protocol'] in ("passthrough", "plex"):
        self._instances[name] = PassthroughPlayerInstance(self._core, name, self._client_info[name]['port'])
      else:
        self._instances[name] = PlayerInstance(self._core, name)
    return self._instances[name]
