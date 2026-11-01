#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from model import Model

def generate_class(access_point, template, storage_path):
  return type(template.__name__, (Model,), dict(_template = template, _access_point = access_point, _path = storage_path, _instances = dict()))
  