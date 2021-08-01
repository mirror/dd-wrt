
import os, shutil, sys, errno, stat

# relpath path import (available in Python 2.6 and above)
try:
    import posixpath
    relpath = posixpath.relpath
except (NameError, AttributeError):
    from posixpath import curdir, sep, pardir, join
    def relpath(path, start=curdir):
        """Return a relative version of a path"""
        if not path:
            raise ValueError("no path specified")
        start_list = posixpath.abspath(start).split(sep)
        path_list = posixpath.abspath(path).split(sep)
        # Work out how much of the filepath is shared by start and path.
        i = len(posixpath.commonprefix([start_list, path_list]))
        rel_list = [pardir] * (len(start_list)-i) + path_list[i:]
        if not rel_list:
            return curdir
        return join(*rel_list)
except (ImportError):
    # We are in the wrong platform
    def relpath(path, start=curdir):
        raise NotImplementedError(
            'The relpath() function is only implemented on posix platforms'
        )
    
if not hasattr(os.path, 'relpath'):
  os.path.relpath = relpath
  

# Replace standard builtin, os and shutil functions with ones that convert strings to unicode so Windows doesn't get upset

if sys.platform == 'win32':
  def longpathify(s):
  
    # Specify the long path prefix, to avoid 260 character limit nonsense.
    if s[1:3] == ':\\':
      s = "\\\\?\\" + s

    # Make sure we are using Windows separators.
    s = s.replace('/', '\\')
    
    return s

  def uni(s):   
    # Try to find a codec willing to handle the string
    if isinstance(s, unicode):
      return s      
    return unicode(s.decode('utf-8'))
  
  if isinstance(__builtins__, dict):
    __builtins__['uni'] = uni
    __builtins__['longpathify'] = longpathify
  else:
    __builtins__.uni = uni
    __builtins__.longpathify = longpathify
  
  # open
  # open's mode: default='r'
  # open's buffering: default=-1
  #     1 means line buffered, 
  #     any other positive value means use a buffer of (approximately) that size (in bytes).
  #     A negative buffering means to use the system default, which is usually line buffered for 
  #     tty devices and fully buffered for other files. If omitted, the system default is used.
  if isinstance(__builtins__, dict):
    __builtins__['_open'] = __builtins__['open']
    def builtins_open(filename, mode='r', buffering=-1):
      return __builtins__['_open'](longpathify(uni(filename)), mode, buffering)
    __builtins__['open'] = builtins_open
  else:
    __builtins__._open = __builtins__.open
    def builtins_open(filename, mode='r', buffering=-1):
      return __builtins__._open(longpathify(uni(filename)), mode, buffering)
    __builtins__.open = builtins_open

  # shutil.copy
  shutil._copy = shutil.copy
  def shutil_copy(src, dst):
    return shutil._copy(longpathify(uni(src)), longpathify(uni(dst)))
  shutil.copy = shutil_copy

  # In some circumstances (during bundle updates, e.g.) files on Windows were getting marked
  # read-only, which was causing issues like (repeated) failed updates. This fix checks for
  # and unsets that bit before deletion.
  #
  def remove_read_only(func, path, exc):
    excvalue = exc[1]
    if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
      os.chmod(path, stat.S_IWRITE)
      func(path)
    else:
      raise

  # shutil.rmtree
  shutil._rmtree = shutil.rmtree
  def shutil_rmtree(path, ignore_errors=False, onerror=remove_read_only):
    return shutil._rmtree(longpathify(uni(path)), ignore_errors, onerror)
  shutil.rmtree = shutil_rmtree

  # os.makedirs
  os._makedirs = os.makedirs
  def os_makedirs(path, mode=0777):
    return os._makedirs(longpathify(uni(path)), mode)
  os.makedirs = os_makedirs
  
  # os.listdir
  os._listdir = os.listdir
  def os_listdir(path):
    return os._listdir(longpathify(uni(path)))
  os.listdir = os_listdir

  # os.remove
  os._remove = os.remove
  def os_remove(path):
    return os._remove(longpathify(uni(path)))
  os.remove = os_remove

  # os.unlink
  os._unlink = os.unlink
  def os_unlink(path):
    return os._unlink(longpathify(uni(path)))
  os.unlink = os_unlink

  # os.symlink
  if hasattr(os, 'symlink'):
    os._symlink = os.symlink
    def os_symlink(source, link_name):
      return os._symlink(longpathify(uni(source)), longpathify(uni(link_name)))
    os.symlink = os_symlink

  # os.stat
  os._stat = os.stat
  def os_stat(path):
    return os._stat(longpathify(uni(path)))
  os.stat = os_stat
  
  # os.utime
  os._utime = os.utime
  def os_utime(path, times):
    return os._utime(longpathify(uni(path)), times)
  os.utime = os_utime

  # os.rename
  os._rename = os.rename
  def os_rename(src, dst):
    if os_path_isfile(dst):
      os_remove(dst)
    elif os_path_isdir(dst):
      shutil_rmtree(dst)
    return os._rename(longpathify(uni(src)), longpathify(uni(dst)))
  os.rename = os_rename

  # os.chmod
  os._chmod = os.chmod
  def os_chmod(path, mode):
    return os._chmod(longpathify(uni(path)), mode)
  os.chmod = os_chmod

  # os.chdir
  os._chdir = os.chdir
  def os_chdir(path):
    return os._chdir(longpathify(uni(path)))
  os.chdir = os_chdir

  # os.rmdir
  os._rmdir = os.rmdir
  def os_rmdir(path):
    return os._rmdir(longpathify(uni(path)))
  os.rmdir = os_rmdir

  # os.system
  os._system = os.system
  def os_system(command):
    return os._system(longpathify(uni(command)))
  os.system = os_system

  # os.path.exists
  os.path._exists = os.path.exists
  def os_path_exists(path):
    return os.path._exists(longpathify(uni(path)))
  os.path.exists = os_path_exists

  # os.path.abspath
  os.path._abspath = os.path.abspath
  def os_path_abspath(path):
    return os.path._abspath(uni(path))
  os.path.abspath = os_path_abspath

  # os.path.dirname
  os.path._dirname = os.path.dirname
  def os_path_dirname(path):
    return os.path._dirname(uni(path))
  os.path.dirname = os_path_dirname
  
  # os.path.isdir
  os.path._isdir = os.path.isdir
  def os_path_isdir(path):
    return os.path._isdir(longpathify(uni(path)))
  os.path.isdir = os_path_isdir
  
  # os.path.islink
  os.path._islink = os.path.islink
  def os_path_islink(path):
    return os.path._islink(longpathify(uni(path)))
  os.path.islink = os_path_islink
  
  # os.path.relpath
  os.path._relpath = os.path.relpath
  def os_path_relpath(path, start=None):
    if start == None:
      start = os.curdir
    return os.path._relpath(uni(path), uni(start))
  os.path.relpath = os_path_relpath
  
  # os.path.normpath
  os.path._normpath = os.path.normpath
  def os_path_normpath(path):
    return os.path._normpath(uni(path))
  os.path.normpath = os_path_normpath
  
  # os.path.getmtime
  os.path._getmtime = os.path.getmtime
  def os_path_getmtime(path):
    return os.path._getmtime(longpathify(uni(path)))
  os.path.getmtime = os_path_getmtime
  
  # os.path.getatime
  os.path._getatime = os.path.getatime
  def os_path_getatime(path):
    return os.path._getatime(longpathify(uni(path)))
  os.path.getatime = os_path_getatime
  
  # os.path.isfile
  os.path._isfile = os.path.isfile
  def os_path_isfile(path):
    return os.path._isfile(longpathify(uni(path)))
  os.path.isfile = os_path_isfile
  
  # os.path.splitext
  os.path._splitext = os.path.splitext
  def os_path_splitext(path):
    return os.path._splitext(uni(path))
  os.path.splitext = os_path_splitext
  
  # os.path.basename
  os.path._basename = os.path.basename
  def os_path_basename(path):
    return os.path._basename(uni(path))
  os.path.basename = os_path_basename
  
  # os.path.expanduser
  os.path._expanduser = os.path.expanduser
  def os_path_expanduser(path):
    return os.path._expanduser(uni(path))
  os.path.expanduser = os_path_expanduser
  
  # os.path.isabs
  os.path._isabs = os.path.isabs
  def os_path_isabs(path):
    return os.path._isabs(uni(path))
  os.path.isabs = os_path_isabs
  
  # os.path.join
  os.path._join = os.path.join
  def os_path_join(*args):
    return os.path._join(*[uni(arg) for arg in args])
  os.path.join = os_path_join
  
  # os.walk
  os._walk = os.walk
  def os_walk(top, topdown=True, onerror=None, followlinks=False):
    return os._walk(longpathify(uni(top)), topdown, onerror, followlinks)
  os.walk = os_walk
  
else:

  if isinstance(__builtins__, dict):
    __builtins__['uni'] = unicode
  else:
    __builtins__.uni = unicode
    
    
