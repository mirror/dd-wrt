#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BasePolicy


class ModelPolicy(BasePolicy):

  environment = dict(
    __name__ = '__model__',
  )

  ext = 'pym'