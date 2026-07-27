#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import attributes, Framework

class AbstractTemplate(object):
  pass

class XMLGeneratingObjectTemplate(object):
  xml_tag = 'Object'
  xml_attributes = {}
  http_response_status = 200
  http_response_headers = {}

class ModelTemplate(XMLGeneratingObjectTemplate):
  guid_args = []
  use_hashed_map_paths = False

class AttributeTemplate(object):
  
  _object_type = None
  _data_ext = None
  _always_external = False
  xml_attr_name = None
  
  # Specifies whether this type should always be excluded from model interfaces
  _always_exclude_from_interface = False
  
  def __init__(self):
    self._external = type(self)._always_external
    self._exclude_from_interface = type(self)._always_exclude_from_interface
    
    # Specifies whether the attribute should only be available via model interfaces
    self._synthetic = False
    
    # Specifies whether the attribute should be serialised even if it has a null value
    self._allows_null = False
    
    # Specifies whether the synthetic attribute should be renamed (workaround for synthetic & regular attributes with the same name, e.g. 'art')
    self.synthetic_name = None
      
  def _new(self, owner, name):
    return type(self)._object_type(owner, self, name)

  @property
  def save_externally(self):
    self._external = True
    
  @property
  def exclude_from_interface(self):
    self._exclude_from_interface = True
  
  @property
  def is_synthetic(self):
    self._synthetic = True
    
  @property
  def allows_null(self):
    self._allows_null = True

class ReferenceTemplate(AttributeTemplate):
  def __init__(self, item_template):
    AttributeTemplate.__init__(self)
    self._item_template = item_template

class ContainerTemplate(ReferenceTemplate):
  xml_tag = 'Container'
  xml_attr_name = 'tag'
  @property
  def save_items_externally(self):
    self._item_template.save_externally
    
  @property
  def exclude_from_interface(self):
    self._item_template.exclude_from_interface
  
class ValueTemplate(AttributeTemplate):
  pass
  
class StringTemplate(ValueTemplate):
  _object_type = attributes.StringObject
  _data_ext = 'txt'
  
class IntegerTemplate(ValueTemplate):
  _object_type = attributes.IntegerObject
  _data_ext = 'int'
  
class BooleanTemplate(ValueTemplate):
  _object_type = attributes.BooleanObject
  _data_ext = 'bool'
  
class FloatTemplate(ValueTemplate):
  _object_type = attributes.FloatObject
  _data_ext = 'float'
  
class DateTemplate(ValueTemplate):
  _object_type = attributes.DateObject
  _data_ext = 'date'
  
class TimeTemplate(ValueTemplate):
  _object_type = attributes.TimeObject
  _data_ext = 'time'
  
class DatetimeTemplate(ValueTemplate):
  _object_type = attributes.DatetimeObject
  _data_ext = 'datetime'
  
class DataTemplate(AttributeTemplate):
  _object_type = attributes.DataObject
  _data_ext = None
  _always_external = True

class ObjectContainerTemplate(AttributeTemplate):
  def __init__(self, *classes):
    super(ObjectContainerTemplate, self).__init__()
    self._classes = classes

  _object_type = attributes.ObjectContainerObject
  _data_ext = "xml"
  _always_external = True
  
class RecordTemplate(AttributeTemplate, XMLGeneratingObjectTemplate):
  _object_type = attributes.RecordObject
  _data_ext = 'xml'

class MapTemplate(ContainerTemplate):
  _object_type = attributes.MapObject
  _data_ext = 'xml'
  
class DirectoryTemplate(MapTemplate):
  _object_type = attributes.DirectoryObject
  _data_ext = 'xml'
  _always_exclude_from_interface = True
  
  def __init__(self):
    MapTemplate.__init__(self, DataTemplate())
    
class ProxyTemplate(object):
  def __init__(self, name):
    self._name = name
    
class ProxiedDataTemplate(DataTemplate):
  _object_type = attributes.ProxiedDataObject
    
class ProxyContainerTemplate(DirectoryTemplate):
  _object_type = attributes.ProxyContainerObject
  
  def __init__(self, *proxies):
    self._accepted_proxies = []
    for proxy in proxies:
      if not isinstance(proxy, ProxyTemplate):
        raise Framework.exceptions.FrameworkException("Proxy containers must only contain proxy objects, not %s" % repr(proxy)) 
      self._accepted_proxies.append(proxy._name)
    MapTemplate.__init__(self, ProxiedDataTemplate())

class SetTemplate(ContainerTemplate):
  _object_type = attributes.SetObject
  _data_ext = 'xml'
  
class LinkTemplate(ReferenceTemplate):
  _object_type = attributes.LinkObject
  _data_ext = 'xml'
  
  def __init__(self, item_template):
    # Only allow links to model classes
    if not hasattr(item_template, '__mro__') or (hasattr(item_template, '__mro__') and not ModelTemplate in item_template.__mro__):
      raise Framework.exceptions.FrameworkException("Links must connect to a model class, not %s" % str(item_template))
    ReferenceTemplate.__init__(self, item_template)
    
    