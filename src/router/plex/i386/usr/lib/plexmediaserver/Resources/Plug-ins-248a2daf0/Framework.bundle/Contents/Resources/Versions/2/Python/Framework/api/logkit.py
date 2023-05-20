#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BaseKit

class LogKit(BaseKit):
  
  def Debug(self, fmt, *args, **kwargs):
    self._core.log.debug(fmt, *args, **kwargs)
  
  def Info(self, fmt, *args, **kwargs):
    self._core.log.info(fmt, *args, **kwargs)
  
  def Warn(self, fmt, *args, **kwargs):
    self._core.log.warn(fmt, *args, **kwargs)
      
  def Error(self, fmt, *args, **kwargs):
    self._core.log.error(fmt, *args, **kwargs)
  
  def Critical(self, fmt, *args, **kwargs):
    self._core.log.critical(fmt, *args, **kwargs)
    
  def Exception(self, fmt, *args, **kwargs):
    """
      The same as the *Critical* method above, but appends the current stack trace to the log message.
      
      .. note:: This method should only be called when handling an exception.
    """
    self._core.log_exception(fmt, *args, **kwargs)

  def Stack(self):
    self._core.log_stack()
    
  def __call__(self, fmt, *args, **kwargs):
    self.Info(fmt, *args, **kwargs)
