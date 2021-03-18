#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import sys, string, httplib, time

from base import BaseInterface

class BasicRequest(object):
  def __init__(self, uri, headers):
    self.uri = uri
    self.headers = headers
    self.method = 'GET'
    self.body = ""

class PipeInterface(BaseInterface):
  
  def _init(self):
    # Get a reference to the real stdout
    self.out = sys.__stdout__
    
  def listen(self, daemonized):
    self._core.log.info("Entering run loop")
    while True:
      try:
        # Read the input
        path = raw_input()
        path = path.lstrip("GET ").strip()
        LastPrefix = None
        
        # Read headers
        headers = {}
        stop = False
        while stop == False:
          line = raw_input()
          if len(line) <= 1:
            stop = True
          else:
            split = string.split(line.strip(), ":", maxsplit=1)
            if len(split) == 2:
              headers[split[0].strip()] = split[1].strip()
        
        # Pass the request info to the runtime & get the result
        status, headers, body = self._core.runtime.handle_request(BasicRequest(path, headers))
        
        response = '%d %s\r\n' % (status, httplib.responses[status])
        response += 'Content-Length: %d\r\n' % len(body)
        for name in headers:
          response += '%s: %s\r\n'% (name, headers[name])

        response += '\r\n'
        
        if len(body) > 0:
          response += '%s\r\n' % body
        
        self.out.write(response)
        self.out.flush()
        
        # After this first response, we're not going to get any more input,
        # so we're going to go into idle mode and sleep. We can't read, because
        # on XP there is a bug where if a thread is blocked in read() and a DLL
        # load occurs, which results in GetFileType being called, which blocks
        # waiting for the read to complete. http://support.microsoft.com/kb/2009703
        #
        if sys.platform == 'win32':
          while True:
            time.sleep(10)
              
      except KeyboardInterrupt:
        self._keyboard_interrupt()
      
      except EOFError:
        self._eof_error()
        
      except:
        self._core.log_exception("Exception in pipe interface")
        
  def _keyboard_interrupt(self):
    self._stop()
    
  def _eof_error(self):
    self._stop()
      
  def _stop(self):
    self._core.log.info("Stopping plug-in")
    sys.exit(0)