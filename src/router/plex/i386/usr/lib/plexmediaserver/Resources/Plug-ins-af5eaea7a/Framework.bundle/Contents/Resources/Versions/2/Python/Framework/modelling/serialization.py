#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os


class Serializable(object):
  
  def _serialize(self, path):
    raise Framework.exceptions.FrameworkException("_serialize not implemented in subclass %s" % type(self).__name__)
    
  def _deserialize(self, path, el):
    raise Framework.exceptions.FrameworkException("_deserialize not implemented in subclass %s" % type(self).__name__)
    
  def _writedata(self, path, data):
    Framework.utils.makedirs(os.path.dirname(path))
    el = self._core.data.xml.element(self._name)
    el.set('external', str(True))
    if self._data_ext is not None:
      el.set('extension', str(self._data_ext))
      path += '.' + self._data_ext
    self._core.storage.save(path, data)
    return el

  def _readdata(self, path):
    if self._data_ext is not None:
      path += '.' + self._data_ext
    if os.path.exists(path):
      return self._core.storage.load(path)

  def _deletedata(self, path):
    if self._data_ext is not None:
      path += '.' + self._data_ext
    if os.path.exists(path):
      os.unlink(path)
