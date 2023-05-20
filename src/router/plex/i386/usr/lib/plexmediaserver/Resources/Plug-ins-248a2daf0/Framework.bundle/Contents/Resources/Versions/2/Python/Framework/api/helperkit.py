#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os
import errno
import time
import select
import sys
import subprocess_new as subprocess

from base import BaseKit

if sys.platform != 'win32':
  import fcntl

PIPE = subprocess.PIPE


class Popen(subprocess.Popen):
  def recv(self, maxsize=None):
    return self._recv('stdout', maxsize)
    
  def recv_err(self, maxsize=None):
    return self._recv('stderr', maxsize)
    
  def recv_wait(self):
    return self._recv_wait('stdout')
    
  def recv_wait_err(self):
    return self._recv_wait('stderr')

  def send_recv(self, input='', maxsize=None):
    return self.send(input), self.recv(maxsize), self.recv_err(maxsize)

  def get_conn_maxsize(self, which, maxsize):
    if maxsize is None:
      maxsize = 1024
    elif maxsize < 1:
      maxsize = 1
    return getattr(self, which), maxsize
    
  def _close(self, which):
    getattr(self, which).close()
    setattr(self, which, None)
    
  def send(self, input):
    if not self.stdin:
      return None

    if not select.select([], [self.stdin], [], 0)[1]:
      return 0

    try:
      written = os.write(self.stdin.fileno(), input)
    except OSError, why:
      if why[0] == errno.EPIPE: #broken pipe
        return self._close('stdin')
      raise

    return written

  def _recv_wait(self, which):
    conn = getattr(self, which)
    try:
      select.select([conn], [], [], 1)
    except:
      pass
    
  def _recv(self, which, maxsize):
    conn, maxsize = self.get_conn_maxsize(which, maxsize)
    if conn is None:
      return None
          
    flags = fcntl.fcntl(conn, fcntl.F_GETFL)
    if not conn.closed:
      fcntl.fcntl(conn, fcntl.F_SETFL, flags| os.O_NONBLOCK)
          
    try:
      if not select.select([conn], [], [], 0)[0]:
        return ''
              
      r = conn.read(maxsize)
      if not r:
        return self._close(which)
  
      if self.universal_newlines:
        r = self._translate_newlines(r)
      return r
    finally:
      if not conn.closed:
        fcntl.fcntl(conn, fcntl.F_SETFL, flags)
                    
                    


class HelperKit(BaseKit):
  """
    Executes and interacts with external binaries.
  """

  _included_policies = [
    Framework.policies.ElevatedPolicy,
  ]

  def _helper_path(self, helper):
    helperPath = os.path.join(
      self._core.bundle_path, 
      'Contents', 
      'Helpers',
      self._core.runtime.os,
      self._core.runtime.cpu,
      helper
    )
    
    if os.path.exists(helperPath):
      return helperPath
      
    else:
      
      # Check for binaries in the root helper directory if we are on OSX/i386
      if self._core.runtime.os == 'MacOSX' and self._core.runtime.cpu == 'i386':
        helperPath = os.path.join(
          self._core.bundle_path, 
          'Contents', 
          'Helpers',
          helper
        )

        if os.path.exists(helperPath):
          self._core.log.warning("Helper named '%s' is not at the correct platform-specific path ('/Contents/Helpers/MacOSX/i386')", helper)
          return helperPath

      # If we're on Windows, check for a helper with the .exe extension
      elif self._core.runtime.os == 'Windows':
        helperPath += '.exe'
        if os.path.exists(helperPath):
          return helperPath

    self._core.log.error("Helper named '%s' is not available on this platform", helper)
    return None
        

  def Run(self, helper, *args):
    """
      Runs a helper and returns any output.
    """  
    helper_path = self._helper_path(helper)
    if not helper_path: return
     
    os.chmod(helper_path, 0755)
    if self._core.runtime.os == 'Windows':
      execString = '"%s"' % helper_path
    else:
      execString = 'nice -n 20 "%s"' % helper_path
    for arg in args:
      execString += ' "%s"' % arg
    return os.popen(execString).read().strip()

  def Process(self, helper, *args, **kwargs):
    """
      Starts a subprocess for the given helper, allowing interaction via pipes
    """
    
    stderr_pipe = subprocess.STDOUT
    
    for kwarg in kwargs:
      if kwarg == 'stderr':
        if kwargs['stderr'] == True:
          stderr_pipe = PIPE
      else:
        raise Framework.exceptions.FrameworkException('Unknown keyword argument \'%s\'' % kwarg)
      
    helper_path = self._helper_path(helper)
    if not helper_path: return
    
    os.chmod(helper_path, 0755)
    if self._core.runtime.os == 'Windows':
      proc_args = [helper_path]
    else:
      proc_args = ['nice', '-n', '20', helper_path]
    
    proc_args.extend(args)
    p = Popen(proc_args, stdin=PIPE, stdout=PIPE, stderr=stderr_pipe)
    return p
    
