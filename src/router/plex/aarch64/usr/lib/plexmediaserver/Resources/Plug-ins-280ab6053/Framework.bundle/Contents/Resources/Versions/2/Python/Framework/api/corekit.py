#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BaseKit


class CoreKit(BaseKit):
  
  _root_object = False
  _included_policies = [
    Framework.policies.ElevatedPolicy,
  ]
  
  def _init(self):
    self._publish(self._core, name='Core')
    self._publish(Framework)
    self._publish(self._factory, name='Factory')
    self._publish(self._factory_class, name='FactoryClass')

    
  def _factory(self, cls):
    return Framework.objects.ObjectFactory(self._core, cls)

    
  def _factory_class(self, fac):
    return fac._object_class

