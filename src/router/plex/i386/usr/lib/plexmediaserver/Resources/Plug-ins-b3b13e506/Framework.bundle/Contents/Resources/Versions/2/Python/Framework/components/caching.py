#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from __future__ import with_statement
import Framework
import os, datetime, weakref
import time
from operator import itemgetter

from base import BaseComponent


class CachedItem(object):
  def __init__(self, name, manager):
    self._name = name
    self._hash = manager._core.data.hashing.sha1(name)
    self._manager = weakref.proxy(manager)
    self._attributes = {}
    self._item_sizes = {}
    self._expire = 0
    self._update_times()
    self._saved = True
    
  def __getattr__(self, name):
    if name[0] != '_' and name in self._attributes:
      self._update_times()
      self._save()
      return self._attributes[name]
    
  def __setattr__(self, name, value):
    if name[0] != '_':
      self._attributes[name] = value
      self._update_times(modified=True)
      self._save()
    object.__setattr__(self, name, value)
    
  def _item_path(self, name):
    self._manager._core.storage.ensure_dirs(os.path.join(self._manager._path, self._hash[0:2]))
    return os.path.join(self._manager._path, self._hash[0:2], self._hash[2:] + '.' + name)
    
  def __getitem__(self, name):
    self._update_times()
    item_path = self._item_path(name)
    self._save()
    if os.path.exists(item_path):
      fdata = self._manager._core.storage.load(item_path)
      return fdata
    else:
      return None
      
  def __setitem__(self, name, data):
    self._update_times(modified=True)
    self._item_sizes[name] = len(data)
    self._manager._core.storage.save(self._item_path(name), data)
    self._save()
    
  def _update_times(self, modified=False):
    now = datetime.datetime.now()
    self._accessed = now
    if modified:
      self._modified = now
    self._saved = False
    
  def _save(self):
    self._manager._notify_updated(self)
    
  @property
  def modified_at(self): return self._modified
  
  @property
  def accessed_at(self): return self._accessed
  
  @property
  def expired(self):
    return self._expire == 0 or self._modified < (datetime.datetime.now() - datetime.timedelta(seconds=self._expire))
    
  def set_expiry_interval(self, interval): self._expire = interval
  
  @property
  def _attributes_path(self):
    self._manager._core.storage.ensure_dirs(os.path.join(self._manager._path, self._hash[0:2]))
    return os.path.join(self._manager._path, self._hash[0:2], self._hash[2:] + '_attributes')
    
class CacheManager(dict):
  def __init__(self, core, path):
    self._core = core
    self._path = path
    self._makedirs()
    self._lock = self._core.runtime.lock()
    self._info = {}
    
    # Load the cache info file
    self._info_path = self._core.storage.join_path(path, 'CacheInfo')
    if self._core.storage.file_exists(self._info_path):
      try:
        self._info = self._core.data.json.from_string(self._core.storage.load(self._info_path))
        
        # If the cache version is unset, set it now
        if self._core.get_value(self._name + '.CacheVersion', 0) < 1:
          self._core.set_value(self._name + '.CacheVersion', 1)
          
      except:
        self._core.log_exception("Exception loading the cache info file from '%s'", self._info_path)
        self.clear()
    elif len(self._core.storage.list_dir(self._path)) > 0:
      self._core.log.debug("No info file found, trashing the cache folder")
      self.clear()
    else:
      self._core.set_value(self._name + '.CacheVersion', 1)
      
  def _validate_files(self, file_list, keys):
    self._core.log.debug("Starting cache validation for %s", self._name)
    
    # Convert the list of URLs to hashes
    hashes = [self._core.data.hashing.sha1(key) for key in keys]
    
    for filename in file_list:
      # Compute the original URL hash from the file path
      name_hash = filename[0].split(self._core.storage.path_sep)[-1] + filename[1]
      if '.' in name_hash:
        name_hash = name_hash[:name_hash.rfind('.')]
      elif len(name_hash) >= 12:
        name_hash = name_hash[:-11]
        
      # If the file's URL hash isn't in the list, remove it
      filename_str = self._core.storage.join_path(*filename)
      if name_hash not in hashes:
        self._core.log.debug("Cache file at '%s' is invalid - removing", filename_str)
        self._core.storage.remove(filename_str)
      else:
        #self._core.log.debug("Cache file at '%s' is valid", filename_str)
        pass
        
    self._core.set_value(self._name + '.CacheVersion', 1)
    self._core.log.debug("Cache validation for %s complete", self._name)
  
      
  @property
  def _name(self):
    return os.path.split(self._path)[-1]
    
  def _makedirs(self):
    Framework.utils.makedirs(self._path)
    
  def _save_info(self, acquire_lock=True):
    if acquire_lock:
      self._lock.acquire()
    try:
      self._core.storage.save(self._info_path, self._core.data.json.to_string(self._info))
    finally:
      if acquire_lock:
        self._lock.release()
    
  def _get(self, name):
    if name in self:
      try:
        ref = dict.__getitem__(self, name)()
        if ref:
          return ref
      except:
        pass

    item = CachedItem(name, self)
    self._load_info(item)
    dict.__setitem__(self, name, weakref.ref(item))
    return item
    
  def __getitem__(self, name):
    with self._lock:
      return self._get(name)
    
  def __setitem__(self, name, value):
    raise Framework.exceptions.FrameworkException("Cache manager items can't be set directly.")
    
  def __delitem__(self, key):
    # Load the item from the cache
    item = self._get(key)
  
    # Remove all data files stored by the cache item
    for name in item._item_sizes:
      path = item._item_path(name)
      if self._core.storage.file_exists(path):
        self._core.storage.remove(path)
  
    # Remove the attributes file
    if self._core.storage.file_exists(item._attributes_path):
      self._core.storage.remove(item._attributes_path)
  
    del item
    if key in self._info:
      del self._info[key]
    
  def _load_info(self, item):
    try:
      if self._core.storage.file_exists(item._attributes_path):
        json = self._core.storage.load(item._attributes_path)
        dct = self._core.data.json.from_string(json)
        item._accessed = datetime.datetime.fromtimestamp(dct['accessed_at'])
        item._modified = datetime.datetime.fromtimestamp(dct['modified_at'])
        item._expire = dct['expiry_interval']
        item._attributes = dct['attributes']
        item._item_sizes = dct['item_sizes']
    except:
      item._update_times(True)
      item._expire = 0
      item._attributes = {}
    
  def _notify_updated(self, item):
    with self._lock:
      accessed_at = int(Framework.utils.timestamp_from_datetime(item.accessed_at))
      modified_at = None
      if item.modified_at:
        modified_at = int(Framework.utils.timestamp_from_datetime(item.modified_at))
      dct = dict(
        accessed_at = accessed_at,
        modified_at = modified_at,
        expiry_interval = int(item._expire),
        attributes = dict(item._attributes),
        item_sizes = dict(item._item_sizes)
      )
      json = self._core.data.json.to_string(dct)
      self._core.storage.save(item._attributes_path, json)
      object.__setattr__(item, 'saved', True)
      
      # Update the cache info dict
      total_size = 0
      for item_name in item._item_sizes:
        total_size += item._item_sizes[item_name]
      self._info[item._name] = [accessed_at, total_size]
    self._save_info()
      
  def trim(self, max_size, max_items):
    with self._lock:
      try:
        # Sort the items - the least recently accessed will be first in the list
        info = sorted(self._info.items(), key=itemgetter(1))
        
        # Remove items if the count is above the maximum limit
        info_len = len(info)
        if info_len > max_items:
          remove_count = info_len - max_items
          for item in info[:remove_count]:
            del self[item[0]]
          info = info[remove_count:]

        # Compute the total size of the cache
        size = 0
        for item in info:
          size += item[1][1]
        
        # While the size is greater than the maxiumum allowed size, whack the least recently accessed cache item
        while size > max_size:
          key, attrs = info[0]
          item_size = attrs[1]
          
          del self[key]
          
          # Decrement the size
          size -= item_size
        
          del info[0]
          del self._info[key]
        
        del info
        
        self._core.log.debug("%s cache trimmed - new size is %d KB (%d items)", os.path.split(self._path)[1], int(size / 1024), len(self._info))
      
      except:
        self._core.log_exception("Error trimming cache")
        
    self._save_info()
    
  def clear(self):
    with self._lock:
      # Move to trash on OS X, or delete immediately on other platforms
      # TODO: Use faster methods on other platforms (e.g. Recycle Bin on Windows)
      try:
        if self._core.runtime.os == 'MacOSX':
          trash_path = self._core.storage.join_path(os.getenv("HOME"), '.Trash', '%s (%s) %d' % (os.path.split(self._path)[1], self._core.identifier, int(time.time())))
          self._core.storage.rename(self._path, trash_path)
        else:
          self._core.storage.remove_tree(self._path)
      except:
        self._core.log_exception('Exception trashing the cache folder')
      self._makedirs()
      dict.clear(self)
      self._info = {}
      self._core.set_value(self._name + '.CacheVersion', 1)
      self._save_info(acquire_lock=False)
      
  @property
  def item_count(self):
    return len(self._info)
      
    
class Caching(BaseComponent):
  
  def _init(self):
    self.caches_path = os.path.join(self._core.plugin_support_path, 'Caches', self._core.identifier)
    self.managers = {}
    
  def get_cache_manager(self, name, system=False):
    if system: name += '.system'
    else: name += '.user'
    if name in self.managers:
      return self.managers[name]

    mgr = CacheManager(self._core, os.path.join(self.caches_path, name))
    self.managers[name] = mgr
    return mgr