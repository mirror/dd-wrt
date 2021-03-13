import Framework
import mimetypes, os

from base import BaseKit

class StreamInitiator(Framework.CoreObject):
  def __init__(self, core):
    Framework.CoreObject.__init__(self, core)
    self.uuid = None
    
  @property
  def valid(self):
    return True
    
  def _ext(self):
    return ''
    
  def _refresh_uuid(self):
    pass
    
  def __str__(self):
    if not self.uuid or not self.valid:
      self._refresh_uuid()
    return 'http://%s:32400/:/stream/%s' % (self._core.networking.address, self.uuid + self._ext())
  
  
class LocalFileStreamInitiator(StreamInitiator):
  def __init__(self, core, path, size):
    StreamInitiator.__init__(self, core)
    self._path = path
    self._size = size
    self._refresh_uuid()
    
  def _refresh_uuid(self):
    self.uuid = self._core.messaging.call_external_function(
      '..system',
      "_StreamService:RegisterLocalFile",
      kwargs = dict(
        path = self._path,
        size = self._size
      )
    )

  def _ext(self):
    return os.path.splitext(self._path)[1]
    
  @property
  def valid(self):
    return self._core.messaging.call_external_function(
      '..system',
      "_StreamService:IsStreamValid",
      kwargs = dict(
        uuid = self.uuid
      )
    )

class StreamKit(BaseKit):

  _included_policies = [
    Framework.policies.ElevatedPolicy,
  ]

  def LocalFile(self, path, size=None):
    return LocalFileStreamInitiator(self._core, path, size)