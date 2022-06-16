#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService

class LocalFileStreamInfo(object):
  def __init__(self, path, size=None):
    self.path = path
    self.size = size
    self.access_time = Datetime.Now()


class StreamService(SystemService):
  def __init__(self, system):
    SystemService.__init__(self, system)
    Log.Debug("Starting the stream service")
    self.streams = dict()
    Core.messaging.expose_function(self.register_local_file, '_StreamService:RegisterLocalFile')
    Core.messaging.expose_function(self.is_stream_valid, '_StreamService:IsStreamValid')
    Core.runtime.add_private_request_handler(self.token_handler)
    Thread.CreateTimer(3600, self.invalidation_timer)
    
  def token_handler(self, pathNouns, kwargs, context=None):
    if len(pathNouns) == 6 and pathNouns[2] == ':':
      pathNouns = pathNouns[3:]

    if len(pathNouns) == 3 and pathNouns[0] == 'stream' and pathNouns[1] == 'token':
      uuid = pathNouns[2]
      if '.' in uuid:
        uuid = uuid[:uuid.find('.')]
      Log("Returning info for stream '%s'", uuid)
      return self.stream_info(uuid)
    
  def invalidation_timer(self):
    for uuid, stream in self.streams.items():
      if stream.access_time < Datetime.Now() - Datetime.Delta(hours=24):
        Log("Removing invalidated stream '%s'", uuid)
        del self.streams[uuid]
    
  def register_local_file(self, path, size=None):
    uuid = String.UUID()
    self.streams[uuid] = LocalFileStreamInfo(path, size)
    Log("Registered local file stream: %s -> %s", uuid, path)
    return uuid
    
  def is_stream_valid(self, uuid):
    return uuid in self.streams
    
  def stream_info(self, uuid):
    if uuid not in self.streams:
      return
    
    stream = self.streams[uuid]
    stream.access_time = Datetime.Now()
    
    xml = XML.Element('File')
    xml.set('path', stream.path)
    if stream.size:
      size = stream.size
    else:
      size = Core.storage.file_size(stream.path)
    xml.set('size', str(size))
    
    return xml
