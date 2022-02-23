#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework


class BaseComponent(Framework.CoreObject):

  def __init__(self, core):
    Framework.CoreObject.__init__(self, core)

    component_class = type(self)
    if hasattr(component_class, 'subcomponents'):
      for subcomponent in component_class.subcomponents:
        setattr(self, subcomponent, component_class.subcomponents[subcomponent](self))
        


class SubComponent(object):

  def __init__(self, base):
    self._base = base
    self._init()
    

  def _init(self):
    pass
    
    
  @property
  def _core(self):
    return self._base._core
