#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import simplejson, demjson, zipfile, hashlib, binascii
from lxml import etree, html, objectify
from lxml.html import soupparser, HTMLParser
import cStringIO as StringIO
from BeautifulSoup import UnicodeDammit
import gzip

from base import BaseComponent, SubComponent

pickle = Framework.LazyModule('pickle')
html_parser = HTMLParser(encoding='utf-8')
xml_parser = etree.XMLParser(remove_blank_text=True)

class Hashing(BaseComponent):
  def _generateHash(self, data, obj, digest=False):
    obj.update(data)
    if digest:
      return obj.digest()
    else:
      return obj.hexdigest()

  def md5(self, data, digest=False):
    return self._generateHash(data, hashlib.md5(), digest)

  def sha1(self, data, digest=False):
    return self._generateHash(data, hashlib.sha1(), digest)

  def sha224(self, data, digest=False):
    return self._generateHash(data, hashlib.sha224(), digest)

  def sha256(self, data, digest=False):
    return self._generateHash(data, hashlib.sha256(), digest)

  def sha384(self, data, digest=False):
    return self._generateHash(data, hashlib.sha384(), digest)

  def sha512(self, data, digest=False):
    return self._generateHash(data, hashlib.sha512(), digest)
  
  def crc32(self, data):
    crc = binascii.crc32(data)
    return '%08X' % (crc & 2**32L - 1)


class ZipArchive(object):
  def __init__(self, data=None):
    self._closed = False
    if data:
      self._io = StringIO.StringIO(str(data))
    else:
      self._io = StringIO.StringIO()
    self._zip = zipfile.ZipFile(self._io, mode='a')
    
  @property
  def Names(self):
    return self._zip.namelist
    
  def __iter__(self):
    names = self._zip.namelist()
    return names.__iter__()
    
  def __getitem__(self, name):
    return self._zip.read(name)
    
  def __setitem__(self, name, data):
    self._zip.writestr(name, data)
    
  @property
  def Data(self):
    self._io.seek(0)
    return self._io.read()
    
  def Close(self):
    if not self._closed:
      self._zip.close()
    self._closed = True
    
  def Test(self):
    return self._zip.testzip()
    
  def __str__(self):
    return self.Data
    
  def __del__(self):
    self.Close()
    self._io.close()

class Archiving(SubComponent):
  
  zip_archive = ZipArchive
  
  def gzip_decompress(self, data):
    stream = StringIO.StringIO(data)
    gzipper = gzip.GzipFile(mode='rb', fileobj=stream)
    ret = gzipper.read()
    gzipper.close()
    stream.close()
    del gzipper
    del stream
    return ret
    
  def gzip_compress(self, data):
    stream = StringIO.StringIO()
    gzipper = gzip.GzipFile(mode='wb', fileobj=stream)
    gzipper.write(data)
    gzipper.close()
    ret = stream.getvalue()
    stream.close()
    del gzipper
    del stream
    return ret
    

class XML(SubComponent):
  
  def _construct_el(self, el, text, kwargs):
    if text:
      el.text = text
    for key in kwargs:
      el.set(key, kwargs[key])
    return el
    
  def element(self, name, text=None, **kwargs):
    return self._construct_el(etree.Element(name), text, kwargs)
    
  def html_element(self, name, text=None, **kwargs):
    return self._construct_el(html.Element(name), text, kwargs)
    
  def to_string(self, el, encoding='utf-8', method=None):
    if method == None:
      if isinstance(el, html.HtmlElement):
        method = 'html'
      else:
        method = 'xml'
    if method == 'xml':
      return etree.tostring(el, pretty_print=True, encoding=encoding, xml_declaration=True)
    elif method == 'html':
      return html.tostring(el, method=method, encoding=encoding)
      
  def from_string(self, string, isHTML=False, encoding=None, remove_blank_text=False):
    if string is None: return None
    
    if encoding == None:
      ud = UnicodeDammit(str(string), isHTML=isHTML)
      markup = ud.markup.encode('utf-8')
    else:
      markup = str(string).encode(encoding)

    if isHTML:
      try:
        return html.fromstring(markup, parser=html_parser)
      except:
        self._core.log_exception('Error parsing with lxml, falling back to soupparser')
        return soupparser.fromstring(string)
    else:
      return etree.fromstring(markup, parser=(xml_parser if remove_blank_text else None))
      
  def object_from_string(self, string):
    return objectify.fromstring(string)
    
  def object_to_string(self, obj, encoding='utf-8'):
    return etree.tostring(obj, pretty_print=True, encoding=encoding)
      
class JSON(SubComponent):
  def _init(self):
    self._lock = self._core.runtime.lock()
    
  def from_string(self, jsonstring, encoding=None):
    try:
      self._lock.acquire()
      return simplejson.loads(jsonstring, encoding)
    except simplejson.decoder.JSONDecodeError, e:
      self._core.log.warn("Error decoding with simplejson, using demjson instead (this will cause a performance hit) - %s" % str(e.args[0]))
      return demjson.decode(jsonstring, encoding)
    finally:
      self._lock.release()
      
  def to_string(self, obj):
    try:
      self._lock.acquire()
      return simplejson.dumps(obj)
    except:
      self._core.log.warn("Error encoding with simplejson, trying demjson instead. This will cause a performance hit.")
      self._core.log_exception('JSON encoding error')
      return demjson.encode(obj)
    finally:
      self._lock.release()
    
class Pickle(SubComponent):

  def load(self, string):
    try:
      # Temporarily insert the unpickle function into the sandboxed environment
      self._core.sandbox.environment['PlexFramework_core_data_unpickle_'] = pickle.loads
      
      # Compile & execute the unpickling code
      code = self._core.loader.compile('PlexFramework_core_data_unpickled_object_=PlexFramework_core_data_unpickle_(%s)' % repr(string), '__unpickle__')
      self._core.sandbox.execute(code)
      del code
      
      # Grab the unpickled object and return it
      obj = self._core.sandbox.environment['PlexFramework_core_data_unpickled_object_']
      return obj
    
    finally:
      # Delete things we don't want lying around any more
      if 'PlexFramework_core_data_unpickle_' in self._core.sandbox.environment:
        del self._core.sandbox.environment['PlexFramework_core_data_unpickle_']
      if 'PlexFramework_core_data_unpickled_object_' in self._core.sandbox.environment:
        del self._core.sandbox.environment['PlexFramework_core_data_unpickled_object_']
      
  
  def dump(self, obj):
    try:
      # Temporarily insert the pickle function and the object to pickle into the sandboxed environment
      self._core.sandbox.environment['PlexFramework_core_data_pickle_'] = pickle.dumps
      self._core.sandbox.environment['PlexFramework_core_data_object_to_pickle_'] = obj
    
      # Compile & execute the pickling code
      code = self._core.loader.compile('PlexFramework_core_data_pickled_object_string_=PlexFramework_core_data_pickle_(PlexFramework_core_data_object_to_pickle_)', '__pickle__')
      self._core.sandbox.execute(code)
      
      # Grab the pickled string and return it
      pickled_string = self._core.sandbox.environment['PlexFramework_core_data_pickled_object_string_']
      return pickled_string
    
    finally:
      # Delete things we don't want lying around any more
      if 'PlexFramework_core_data_pickle_' in self._core.sandbox.environment:
        del self._core.sandbox.environment['PlexFramework_core_data_pickle_']
      if 'PlexFramework_core_data_object_to_pickle_' in self._core.sandbox.environment:
        del self._core.sandbox.environment['PlexFramework_core_data_object_to_pickle_']
      if 'PlexFramework_core_data_pickled_object_string_' in self._core.sandbox.environment:
        del self._core.sandbox.environment['PlexFramework_core_data_pickled_object_string_']
    
    return pickle.dumps(obj)
    
class Data(BaseComponent):
  subcomponents = dict(
    json      = JSON,
    xml       = XML,
    pickle    = Pickle,
    archiving = Archiving,
    hashing   = Hashing,
  )
