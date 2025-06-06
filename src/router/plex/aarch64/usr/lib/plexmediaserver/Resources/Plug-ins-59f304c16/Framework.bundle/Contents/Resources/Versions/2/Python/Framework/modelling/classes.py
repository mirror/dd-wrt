#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

def generate_class(access_point, namespace_name, template, storage_path):
  import namespace
  namespace.__name__ = 'Framework.models.'+namespace_name+'.'+access_point._identifier.replace('.','_')
  if Framework.constants.flags.log_model_class_generation in access_point._core.flags:
	  access_point._core.log.debug("Generating class for '%s' model in namespace '%s' with access point '%s'", template.__name__, namespace_name, access_point._identifier)
  cls = namespace.generate_class(access_point, template, storage_path)
  del namespace
  return cls