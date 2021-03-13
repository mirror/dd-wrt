import Framework
import weakref
import attributes
import templates

# Convert a Python name with underscores to an XML camelcased name
def convert_name(name):
  while '_' in name:
    pos = name.find('_')
    name = name[:pos] + name[pos+1].upper() + name[pos+2:]
  return name

class ObjectMetaclass(type):

  def __eq__(cls, other):
    # Allow comparison of object classes based on their name, since we synthesise different
    # classes for each sandbox that should be considered equivalent.
    #
    return cls != Object and other != Object and hasattr(other, '__mro__') and Object in other.__mro__ and (cls.__name__ == other.__name__)

  def __getattr__(cls, name):
    return cls._class_attributes[name]
    
  def __setattr__(cls, name, value):
    if name in cls.__dict__ or name[0] == '_':
      super(ObjectMetaclass, cls).__setattr__(cls, name, value)
    else:
      if cls._attribute_list != None and name not in cls._attribute_list:
        raise Framework.exceptions.FrameworkException("Object of type '%s' has no attribute named '%s'" % (str(cls), name))
      cls._class_attributes[name] = value


class Object(object):
  __metaclass__ = ObjectMetaclass
  
  _attribute_list = list()
  _class_attributes = dict()
  _unique = False
  
  xml_tag = 'Object'
  
  def __init__(self, **kwargs):
    self._response_headers = {'Content-Type': 'application/xml'}
    self._attributes = {}
    for name in kwargs:
      setattr(self, name, kwargs[name])
    
  @property
  def _core(self):
    """
      Easy access to the core
    """
    return type(self)._sandbox._core

  @property
  def _context(self):
    """
      Easy access to the context.
    """
    return type(self)._sandbox.context
      
  def _set_attribute(self, el, name, value):
    try:
      if isinstance(value, bool):
        value = '1' if value else '0'
      elif isinstance(value, (Framework.components.localization.LocalString, Framework.components.localization.LocalStringPair, Framework.components.runtime.HostedResource, int, long)):
        value = str(value)
      elif isinstance(value, dict):
        value = None
      if value != None:
        el.set(convert_name(name), value)
    except:
      self._core.log_exception("Exception setting attribute '%s' of object %s to %s (type: %s)", name, str(self), str(value), str(type(value)))
            
  def _to_xml(self, excluded_attrs=[]):
    root = self._core.data.xml.element(type(self).xml_tag)
    
    for name in type(self)._class_attributes:
      if name not in excluded_attrs:
        self._set_attribute(root, name, type(self)._class_attributes[name])
    
    for name in self._attributes:
      if name not in excluded_attrs:
        self._set_attribute(root, name, self._attributes[name])
    
    return root
    
  def _get_response_status(self):
    return 200
    
  def _get_response_headers(self):
    return self._response_headers
    
  @property
  def headers(self):
    return self._response_headers
    
  def __hasattr__(self, name):
    return name in self._attributes or name in self.__dict__
    
  def __getattr__(self, name):
    if name[0] != '_':
      if name in self._attributes:
        return self._attributes[name]
      elif name in self._attribute_list:
        return None
    
    return object.__getattribute__(self, name)
    
  def __setattr__(self, name, value):
    if name[0] != '_':
      if self._attribute_list != None and name not in self._attribute_list:
          raise Framework.exceptions.FrameworkException("Object of type '%s' has no attribute named '%s'" % (str(type(self)), name))
      self._attributes[name] = value
      return
    object.__setattr__(self, name, value)
    
    
class Container(Object):
  _children_attr_name = 'objects'
  _child_types = []
  
  def __init__(self, **kwargs):
    setattr(self, '_objects', [])
    
    if self._children_attr_name in kwargs:
      for obj in kwargs[self._children_attr_name]:
        self.add(obj)
        
      del kwargs[self._children_attr_name]
      
    Object.__init__(self, **kwargs)
      
  def _to_xml(self, excluded_attrs=[]):
    ea = list(excluded_attrs)
    if type(self)._children_attr_name not in ea:
      ea.append(type(self)._children_attr_name)
    root = Object._to_xml(self, ea)
    self._append_children(root, self._objects)
    return root
    
  def _append_children(self, root, children):
    if children:
      for obj in children:
        el = obj._to_xml()
        if el != None:
          root.append(el)
        
  def add(self, obj):
    for obj_type in type(self)._child_types:
      if isinstance(obj, obj_type):
        cls = type(obj)
        # Check whether the object should be unique within the container.
        if cls._unique:
          for other in self._objects:
            if isinstance(other, cls):
              raise Framework.exceptions.FrameworkException("Only one object of type '%s' can be added to a container." % cls.__name__)

        self._objects.append(obj)
        return
        
    raise Framework.exceptions.FrameworkException("Object of type '%s' cannot be added to this container." % str(type(obj)))

  def extend(self, obj_list):
      # If passed a container, extract the object list.
      if isinstance(obj_list, Container):
          obj_list = obj_list._objects
      for obj in obj_list:
          self.add(obj)
    
  def __len__(self):
    return len(self._objects)
    
  def __nonzero__(self):
    return True
    
  def __hasattr__(self, name):
    if name == self._children_attr_name:
      return True
    return Object.__hasattr__(self, name)
    
  def __getattr__(self, name):
    if name == self._children_attr_name:
      return self._objects
    return Object.__getattr__(self, name)
      
  def __setattr__(self, name, value):
    if name == self._children_attr_name:
      self._objects = value
      return
    Object.__setattr__(self, name, value)
    

class ModelInterfaceObjectMetaclass(ObjectMetaclass):
  def __init__(cls, name, bases, dct):

    return super(ModelInterfaceObjectMetaclass, cls).__init__(name, bases, dct)

  def __setattr__(cls, name, value):
    if name in cls.__dict__ or name[0] == '_':
      super(ObjectMetaclass, cls).__setattr__(name, value)
    else:
      if name not in cls._model_class._attributes and name not in cls._model_class._synthetic_attributes:
        raise Framework.exceptions.FrameworkException("Class '%s' has no attribute named %s" % (cls.__name__, name))
      cls._class_attributes[name] = value


class ModelInterfaceObject(Container):
  __metaclass__ = ModelInterfaceObjectMetaclass
  _model_class = None
  
  def __init__(self, **kwargs):
    # Create a new model instance
    self._model = self._model_class()
    
    model_attrs = {}
    for name in kwargs.keys():
      if hasattr(self._model, name) or self._model._has_synthetic_attr(name):
        model_attrs[name] = kwargs[name]
        del kwargs[name]
      
    Container.__init__(self, **kwargs)
    
    for name in model_attrs:
      setattr(self, name, model_attrs[name])
      
    
  @property
  def _model_class(self):
    """
      Easy access to the model class
    """
    return type(self)._model_class
    
  @property
  def _template(self):
    """
      Easy access to the model template
    """
    return self._model_class._template
    
  def _get_response_status(self):
    return self._model_class._template.http_response_status
    
  def _get_response_headers(self):
    d = dict()
    d.update(self._model_class._template.http_response_headers)
    d.update(self._response_headers)
    return d
    
  def _check_attr_permissions(self, name):
    """
      If the model template excludes the named attribute from interfaces, raise an exception.
    """
    if name[0] != '_' and name not in self._model._synthetic_attributes and (name not in self._model._attributes or self._model._attributes[name]._template._exclude_from_interface) and name not in type(self)._attribute_list and not Container.__hasattr__(self, name):
      raise Framework.exceptions.FrameworkException("The '%s' attribute is inaccessible from model interfaces." % name)
    
  def __hasattr__(self, name):
    if name in self._model._attributes or self._model._has_synthetic_attr(name) or name in self._attributes:
      return True
    return Container.__hasattr__(self, name)
    
  def __getattr__(self, name):
    self._check_attr_permissions(name)
    
    # If this attribute is synthetic, use the special getter
    if self._model._has_synthetic_attr(name):
      return self._model._get_synthetic_attr(name)
    
    # Otherwise, return the regular attribute
    if name in self._model._attributes:
      return getattr(self._model, name)
    
    return Container.__getattr__(self, name)
    
  def __setattr__(self, name, value):
    try:
      if name[0] != '_':
        self._check_attr_permissions(name)
        if hasattr(self._model, name) or self._model._has_synthetic_attr(name):
          # If this attribute is synthetic, use the special setter
          if self._model._has_synthetic_attr(name):
            self._model._set_synthetic_attr(name, value)
            return
          else:
            # If the attribute is a set, and a list has been provided, clear the set and add each list item to it
            attr = getattr(self._model, name)
            if isinstance(attr, attributes.SetObject) and isinstance(value, list):
              attr.clear()
              # If the item uses a record template, handle it a bit differently so we can set multiple attributes. Assume a dict is being provided.
              if isinstance(attr._template._item_template, templates.RecordTemplate):
                for item in value:
                  record = attr.new()
                  for key in item:
                    setattr(record, key, item[key])
              
              else:            
                for item in value:
                  attr.add(item)
              return
              
            # Otherwise, call the default setter
            else:
              setattr(self._model, name, value)
              return

      Container.__setattr__(self, name, value)
    except Framework.exceptions.AttributeTypeMismatch, e:
      raise Framework.exceptions.AttributeTypeMismatch("Error setting attribute '%s': %s" % (name, e.status))
      

  def _to_xml(self):
    """
      Convert the model object to the Alexandria XML format
    """
    # Create a root element for the object
    root = Container._to_xml(self)
    root.tag = self._template.xml_tag
    
    # Set an empty key attribute - this may be overwritten later, but is required to prevent Plex for OS X crashing
    if root.get('key') == None:
      root.set('key', '')
      
    # Set XML attributes defined in the model template
    for name in self._template.xml_attributes:
      self._set_attribute(root, name, self._template.xml_attributes[name])
    
    # For each model attribute, convert it to the proper XML
    def set_xml_attr(el, name, attr):
      
      # If the attribute is a RecordObject, set all the record attributes on the XML element
      if isinstance(attr, attributes.RecordObject):
        for sub_attr_name in attr._attributes:
          sub_attr = attr._attributes[sub_attr_name]
          sub_attr_string = sub_attr._to_string()
          
          # If the attribute name is the same as the record class name, check whether we should override it.
          if sub_attr_name.lower() == type(attr._template).__name__.lower() and attr._template.xml_attr_name:
            sub_attr_name = attr._template.xml_attr_name
            
          if sub_attr_string:
            el.set(convert_name(sub_attr_name), sub_attr_string)
      
      # Otherwise, set a single attribute directly 
      else:
        attr_string = attr._to_string()
        if attr._template.xml_attr_name:
          name = attr._template.xml_attr_name
          
        if attr_string:
          el.set(convert_name(name), attr_string)

    def set_xml_attrs(attr_dict):
      for name in attr_dict:
        attr = attr_dict[name]
        
        # Set ValueObject attributes directly
        if isinstance(attr, attributes.ValueObject):
          set_xml_attr(root, name, attr)
            
        # Add SetObject attributes as child elements
        elif isinstance(attr, attributes.SetObject):
          for i in range(len(attr)):
            set_attr = attr._items[i]
            set_attr_el = self._core.data.xml.element(attr._template.xml_tag, None)
            set_xml_attr(set_attr_el, attr._template.xml_attr_name, set_attr)
            root.append(set_attr_el)
          
            
    set_xml_attrs(self._model._attributes)
    set_xml_attrs(self._model._synthetic_attributes)
    
    return root
    
    
class ModelInterfaceObjectContainer(Container):

  _child_types = [ModelInterfaceObject]

  def _to_xml(self):
    # Same as Container, but set the 'size' attribute
    root = Container._to_xml(self)
    root.set('size', str(len(self._objects)))
    return root

def generate_class(cls, sandbox, child_types=None):
  """
    Generates a new object class linked to the given core
  """
  if child_types is None:
    child_types = []
  if hasattr(cls, '_child_types'):
    child_types.extend(cls._child_types)
  generated_class = type(cls.__name__, (cls, ), dict(_sandbox = weakref.proxy(sandbox), _class_attributes = {}, _child_types=child_types))
  return generated_class
  
    
def generate_model_interface_class(sandbox, model_class, superclass=ModelInterfaceObject, attribute_list=[], child_types=[], children_attr_name = 'objects'):
  """
    Generates a new ModelInterfaceObject subclass for the given model
  """
  return type(model_class.__name__, (superclass,), dict(_model_class = model_class, _sandbox = weakref.proxy(sandbox), _attribute_list = attribute_list, _child_types = child_types, _children_attr_name = children_attr_name, _class_attributes = {}))
  
  
def generate_model_interface_container_class(sandbox, name, superclass=ModelInterfaceObjectContainer, child_types=[ModelInterfaceObject], **kwargs):
  """
    Generates a new ModelInterfaceObjectContainer class with the given name and default class attributes
  """
  if 'xml_tag' in kwargs:
    xml_tag = kwargs['xml_tag']
  else:
    xml_tag = name
  return type(name, (superclass,), dict(_sandbox = weakref.proxy(sandbox), xml_tag = xml_tag, _child_types = child_types, _class_attributes = kwargs))
