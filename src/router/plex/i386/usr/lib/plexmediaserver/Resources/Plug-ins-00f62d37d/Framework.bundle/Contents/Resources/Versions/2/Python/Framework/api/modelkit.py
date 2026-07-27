#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os

from base import BaseKit

def ProxyObjectGenerator(proxy_name):
  def ProxyFunction(data, sort_order=None, ext=None, index=None, **kwargs):
    return Framework.modelling.attributes.ProxyObject(proxy_name, proxy_name.lower(), data, sort_order, ext, index, **kwargs)
  return ProxyFunction

class ProxyKit(object):
  def __init__(self):
    self.Preview     = ProxyObjectGenerator('Preview')
    self.Media       = ProxyObjectGenerator('Media')
    self.LocalFile   = ProxyObjectGenerator('LocalFile')
    self.Remote     = ProxyObjectGenerator('Remote')
    
    
class ModelKit(BaseKit):
  
  _root_object = False

  _included_policies = [
    Framework.policies.BundlePolicy,
  ]

  _excluded_policies = [
    Framework.policies.ModernPolicy,
  ]
  
  def _init(self):
    template_file = os.path.join(self._core.bundle_path, 'Contents', 'Models', '__init__.pym')
    
    if os.path.exists(template_file):
      self._accessor = Framework.modelling.ModelAccessor(
        self._core,
        'usermodels',
        template_file,
        os.path.join(self._core.storage.data_path, 'ModelData')
      )
      self._publish(self._accessor.get_access_point(self._core.identifier), name='Model')
      
    self._publish(ProxyKit(), 'Proxy')
