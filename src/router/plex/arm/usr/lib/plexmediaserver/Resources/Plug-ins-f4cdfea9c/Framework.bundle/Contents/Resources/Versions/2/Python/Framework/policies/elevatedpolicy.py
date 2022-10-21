#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BundlePolicy


def _super(*args):
  return super(*args)

class ElevatedPolicy(BundlePolicy):

  environment = dict(
    hasattr               = hasattr,
    getattr               = getattr,
    setattr               = setattr,
    dir                   = dir,
    super                 = _super,
    type                  = type,
  )
  
  allow_whitelist_extension = True
  allow_bundled_libraries = True
  elevated_execution = True
