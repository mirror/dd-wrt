#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import CodePolicy, ModernPolicy


class ServicePolicy(CodePolicy, ModernPolicy):
  
  environment = dict(
    __name__    = '__service__'
  )
  
  ext = 'pys'
  