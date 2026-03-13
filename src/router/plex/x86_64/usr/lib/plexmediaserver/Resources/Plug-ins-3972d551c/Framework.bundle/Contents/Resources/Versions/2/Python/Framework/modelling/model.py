#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os, urllib, weakref, shutil
import templates
from serialization import Serializable
from attributes import AttributeSet


class ModelMetaclass(type):
  def __init__(cls, name, bases, dct):
    template = cls._template
    # Copy the value attributes from the template
    cls._attributes = dict()
    cls._synthetic_attributes = dict()
    for name in dir(template):
      obj = getattr(template, name)
      if isinstance(obj, templates.AttributeTemplate):
        if obj._synthetic:
          cls._synthetic_attributes[name] = weakref.proxy(obj)
        else:
          cls._attributes[name] = weakref.proxy(obj)
        
    return super(ModelMetaclass, cls).__init__(name, bases, dct)

  def __getitem__(cls, name):
    obj = cls()
    obj._uid = name
    obj._core.log.debug("Loading model with GUID %s", name)
    obj._read()
    return obj
    
  @property
  def _classpath(cls):
    return os.path.join(cls._path, cls._pluralized_name)
    
  @property
  def _pluralized_name(cls):
    return Framework.utils.plural(cls._model_name.replace('_', ' '))

  @property
  def _model_name(cls):
    try:
      return cls._template.model_name
    except:
      return cls.__name__
    
  @property
  def _core(cls):
    return cls._access_point._core
    
  def guid_exists(cls, uid):
    path = cls._make_path(uid)
    return os.path.exists(path)
    
  def exists(cls, id, **kwargs):
    return cls.guid_exists(cls.make_guid(id, **kwargs))
    
  def _make_path(cls, uid):
    uid_hash = cls._access_point._core.data.hashing.sha1(uid)
    return os.path.join(cls._classpath, uid_hash[0], uid_hash[1:] + '.bundle', 'Contents')

  def _make_relpath(cls, uid):
    uid_hash = cls._access_point._core.data.hashing.sha1(uid)
    return os.path.join(cls._pluralized_name, uid_hash[0], uid_hash[1:] + '.bundle')

  def make_guid(cls, id, **kwargs):
    for key in kwargs:
      if key not in cls._template.guid_args:
        raise Framework.exceptions.FrameworkException("Invalid GUID argument '%s' for model %s" % (key, cls.__name__))
    for key in cls._template.guid_args:
      if key not in kwargs:
        raise Framework.exceptions.FrameworkException("Missing GUID argument '%s' for model %s" % (key, cls.__name__))
    
    if len(kwargs) > 0:
      qs = '?' + urllib.urlencode(kwargs)
    else:
      qs = ''
    return '%s://%s%s' % (cls._access_point._identifier, id, qs)
    
  def erase(cls, uid):
    path = os.path.join(cls._make_path(uid), cls._access_point._identifier)
    if os.path.exists(path):
      shutil.rmtree(path)
  

class Model(AttributeSet, Serializable):
  
  __metaclass__ = ModelMetaclass
  
  _template     = None
  _access_point = None
  _path         = None
  
  def __init__(self, id=None, lang=None):
    self._id = id
    
    if id and lang:
      self._uid = '%s://%s?lang=%s' % (self.provider, id, lang)
    else:
      self._uid = None
        
    attrs = dict()
    synthetic_attrs = dict()
    cls = type(self)
    
    for name in cls._attributes:
      template = cls._attributes[name]
      if isinstance(template, templates.AttributeTemplate):
        attrs[name] = template._new(self, name)

    for name in cls._synthetic_attributes:
      template = cls._synthetic_attributes[name]
      if isinstance(template, templates.AttributeTemplate):
        # Override the attribute name with a synthetic name if defined
        if template.synthetic_name:
          name = template.synthetic_name
        synthetic_attrs[name] = template._new(self, name)
        
    self._attributes = attrs
    self._synthetic_attributes = synthetic_attrs
    
    if self._uid != None:
      self._read()
      
  @property
  def _core(self): return type(self)._access_point._core
  
  @property
  def _storage_path(self):
    return type(self)._make_path(self._uid)
  
  @property
  def _access_point(self):
    return type(self)._access_point
    
  @property
  def _model(self):
    return self
    
  @property
  def provider(self):
    return self._access_point._identifier
    
  @property
  def contributors(self):
    try:
      return os.listdir(self._model._storage_path)
    except:
      return []

  @property
  def attrs(self):
    return self._attributes

  def contribution(self, identifier):
    #TODO: Check for errors, throw exceptions
    name = type(self).__name__
    accessor = self._access_point._accessor
    contributor_access_point = accessor.get_access_point(identifier, read_only=True)
    contributor_class = getattr(contributor_access_point, name)
    return contributor_class[self._uid]
  
  @property
  def id(self):
    return self._id
    
  @property
  def guid(self):
    return self._uid
    
  def _save(self):
    pass
    
  def _write(self, subdir=None):
    if self._uid == None:
      type(self)._core.log.error("Can't write unidentified "+type(self).__name__)

    if self._access_point._read_only:
      raise Framework.exceptions.FrameworkException("Data for %s instance %s provided by %s can't be modified" %  (type(self).__name__, self._uid, self.provider))

    if subdir == None:
      subdir = self._access_point._identifier
    
    try:
      self._core.runtime.acquire_lock('_Model:%s' % self._uid)
      self._serialize(os.path.join(self._storage_path, subdir))
    except:
      self._core.log_exception("Exception serializing %s with guid '%s'", type(self).__name__, self._uid)
    finally:
      self._core.runtime.release_lock('_Model:%s' % self._uid)
      
      
  def _read(self, subdir=None):
    if subdir == None:
      subdir = type(self)._access_point._identifier
    path = os.path.join(self._storage_path, subdir)
    if os.path.exists(path):
      try: self._deserialize(path)
      except: self._core.log_exception("Exception deserializing %s with guid '%s' (%s)", type(self).__name__, self._uid, path)
    else:
      self._core.log.error("Cannot read model from %s", path)
  
  def _serialize(self, path):
    if not self._uid:
      self._core.log.error("Cannot serialize at %s - no GUID", path)
      return
    
    cls = type(self)
    Framework.utils.makedirs(path)
    el = self._core.data.xml.element(cls._model_name)
    el.set('id', self._id)

    for name in cls._attributes:
      attr = self._attributes[name]
      if isinstance(attr, Serializable):
        attr_el = attr._serialize(os.path.join(path, name))
        if attr_el is not None:
          el.append(attr_el)
      else:
        self._core.log.error("Attribute '%s' is not serializable.", name) 

    xml_str = self._core.data.xml.to_string(el)
    
    root_path = os.path.join(path, 'Info.xml')
    self._core.log.debug("Serializing to "+root_path)
    self._core.storage.save(root_path, xml_str)
    
  def _deserialize(self, path, el=None):
    root_path = os.path.join(path, 'Info.xml')
    self._core.log.debug("Deserializing from "+root_path)
    if not os.path.exists(root_path):
      self._core.log.error("Unable to deserialize object at %s" % path)
      return
      
    #TODO: Move this to _read
    xml_str = self._core.storage.load(root_path)
    el = self._core.data.xml.from_string(xml_str)
    
    self._id = el.get('id')
    
    for attr_el in el.getchildren():
      if attr_el.tag in self._attributes:
        attr = self._attributes[attr_el.tag]
        attr._deserialize(os.path.join(path, attr_el.tag), attr_el)
        
    