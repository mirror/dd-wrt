#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import errno
import os
import shutil
import stat
import sys
import tempfile

if sys.platform == 'win32':
  import ctypes
  kdll = ctypes.windll.LoadLibrary("kernel32.dll")

from base import BaseComponent

class Storage(BaseComponent):
  
  def _init(self):
    self.data_path = os.path.join(self._core.plugin_support_path, 'Data', self._core.identifier)
    self.walk = os.walk
    self.copy = shutil.copy
    self.rename = shutil.move
    self.remove = os.remove
    self.utime = os.utime
    self.dir_name = os.path.dirname
    self.last_accessed = os.path.getatime
    self.last_modified = os.path.getmtime
    self.path_sep = os.path.sep
    self.base_name = os.path.basename
    self.abs_path = os.path.abspath
    self.make_temp_dir = tempfile.mkdtemp
    self.change_dir = os.chdir
    self.current_dir = os.getcwd

    # Create a dictionary for storing file modification times.
    self._mtimes = {}

    Framework.utils.makedirs(os.path.join(self.data_path, 'DataItems'))
    
    if not (hasattr(self._core.config, 'do_chdir') and getattr(self._core.config, 'do_chdir') == False):
      os.chdir(self.data_path)
    
  def load(self, filename, binary=True, mtime_key=None):
    filename = os.path.abspath(filename)
    has_runtime = hasattr(self._core, 'runtime')
    if has_runtime: self._core.runtime.acquire_lock('_Storage:'+filename)
    data = None
    try:
      if binary: mode = 'rb'
      else: mode = 'r'
      f = open(filename, mode)
      data = f.read()
      f.close()
      self._update_mtime(filename, mtime_key)
    except:
      self._core.log_exception("Exception reading file %s", filename)
      data = None
      raise
    finally:
      if has_runtime: self._core.runtime.release_lock('_Storage:'+filename)
      return data
    
  def save(self, filename, data, binary=True, mtime_key=None):
    # Don't attempt to save if no data was passed
    if data == None:
      self._core.log.error("Attempted to save no data to '%s' - aborting. Nothing has been saved to disk.", filename)
      return
      
    filename = os.path.abspath(filename)
    self._core.runtime.acquire_lock('_Storage:'+filename)
    tempfile = '%s/._%s' % (os.path.dirname(filename), os.path.basename(filename))
    try:
      if os.path.exists(tempfile):
        os.remove(tempfile)
      if binary: mode = 'wb'
      else: mode = 'w'
      f = open(tempfile, mode)
      f.write(str(data))
      f.close()
      if os.path.exists(filename):
        os.remove(filename)
      shutil.move(tempfile, filename)
      self._update_mtime(filename, mtime_key)
    except:
      self._core.log_exception("Exception writing to %s", filename)
      if os.path.exists(tempfile):
        os.remove(tempfile)
      raise
    finally:
      self._core.runtime.release_lock('_Storage:'+filename)

  def _update_mtime(self, path, mtime_key):
    mtime_key_id = id(mtime_key)
    if mtime_key_id not in self._mtimes:
      self._mtimes[mtime_key_id] = {}
    self._mtimes[mtime_key_id][path] = self.last_modified(path)

  def has_changed(self, path, mtime_key):
    # If the file doesn't exist, it hasn't changed.
    if not self.file_exists(path):
      return False

    mtime_key_id = id(mtime_key)
    return mtime_key_id not in self._mtimes or path in self._mtimes[mtime_key_id] and self.last_modified(path) != self._mtimes[mtime_key_id][path]

      
  def list_dir(self, path):
    return os.listdir(path)
    
  def join_path(self, *args):
    return os.path.join(*args)
    
  def file_exists(self, path):
    return os.path.exists(path)
    
  def dir_exists(self, path):
    return os.path.exists(path) and os.path.isdir(path)
    
  def link_exists(self, path):
    return os.path.exists(path) and os.path.islink(path)
    
  def make_dirs(self, path):
    if not os.path.exists(path):
      os.makedirs(path)
  
  def ensure_dirs(self, path):
    try:
      self.make_dirs(path)
    except:
      if not os.path.exists(path):
        raise

  def remove_tree(self, path):
    if self.dir_exists(path):
      shutil.rmtree(path, ignore_errors=False, onerror=self.remove_read_only)

  # In some circumstances (during bundle updates, e.g.) files on Windows were getting marked
  # read-only, which was causing issues like (repeated) failed updates. This fix checks for
  # and unsets that bit before deletion.
  #
  def remove_read_only(self, func, path, exc):
    excvalue = exc[1]
    if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
      os.chmod(path, stat.S_IWRITE)
      func(path)
    else:
      raise
      
  def copy_tree(self, src, target, symlinks=False):
    if self.dir_exists(src):
      if self.dir_exists(target):
        self.remove_tree(target)
      shutil.copytree(src, target, symlinks)

  def resource_paths_for_sandbox(self, sandbox):
    # Return resource paths for the provided sandbox, or the core sandbox if none was given.
    paths = sandbox.resource_paths if sandbox else self._core.sandbox.resource_paths
    return paths
    
  def path_for_resource_in_sandbox(self, itemname, main_sandbox):
    def look_up_resource(sandbox):
      # Get paths for the given sandbox, and try to find a matching resource.
      for resource_path in self.resource_paths_for_sandbox(sandbox):
        path = os.path.join(resource_path, itemname)
        if os.path.exists(path):
          return path
    
    result = look_up_resource(main_sandbox)
    
    if result == None and main_sandbox:
        
      # If no resource could be found in the default sandbox, check all services with
      # a matching identifier.     
      all_services = self._core.services.get_all_services(main_sandbox.identifier)
      
      for service in all_services:
        result = look_up_resource(service.sandbox)
        if result:
          break
    
    return result
    
      
  def load_resource(self, itemname, binary=True, sandbox=None):
    path = self.path_for_resource_in_sandbox(itemname, sandbox)
    if path:
      return self.load(path, binary)
    
  def resource_exists(self, itemname, sandbox=None):
    path = self.path_for_resource_in_sandbox(itemname, sandbox)
    return path != None
    
  def resource_mtime(self, itemname, sandbox=None):
    path = self.path_for_resource_in_sandbox(itemname, sandbox)
    return self.last_modified(path) if path else 0
    
  def data_item_path(self, itemname):
    return os.path.join(self.data_path, 'DataItems', itemname)  
  
  def data_item_exists(self, itemname):
    return os.path.exists(self.data_item_path(itemname))
    
  def remove_data_item(self, itemname):
    if self.data_item_exists(itemname):
      return os.unlink(self.data_item_path(itemname))
    else:
      return False
      
  def load_data_item(self, itemname, is_object=False):
    if self.data_item_exists(itemname):
      data = self.load(self.data_item_path(itemname))
      if is_object:
        return self._core.data.pickle.load(data)
      else:
        return data
    else:
      return None
    
  def save_data_item(self, itemname, data, is_object=False):
    if is_object:
      data = self._core.data.pickle.dump(data)
    self.save(self.data_item_path(itemname), data)
    
  def file_size(self, path):
    stat = os.stat(path)
    return stat.st_size

  def symlink(self, src, dst):

    # If we are disabling symlinks, do nothing. The lookups are handled inside PMS
    if len(os.environ.get('DISABLESYMLINKS', '')) > 0:
      return

    # Remove old link if it exists.
    if os.path.exists(dst):
      try: os.unlink(dst)
      except: pass

    try:
      # Platform dependent way of creating a new link.
      if sys.platform == 'win32':
        is_dir = 1 if os.path.isdir(src) else 0

        # Turn the source into an absolute path and create a hard link.
        full_src = os.path.normpath(os.path.join(os.path.dirname(dst), src))
        res = kdll.CreateHardLinkW(unicode(longpathify(dst)), unicode(longpathify(full_src)), 0)
        if res == 0:
          self._core.log.debug("Error creating hard link from [%s] to [%s]", src, dst)
      else:
        os.symlink(src, dst)

    except:
      self._core.log_exception("Error creating symbolic link from [%s] to %s]", src, dst)
