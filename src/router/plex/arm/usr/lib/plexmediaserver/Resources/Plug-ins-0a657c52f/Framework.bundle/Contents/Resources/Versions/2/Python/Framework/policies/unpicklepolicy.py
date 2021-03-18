#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import pickle

from base import BasePolicy


class UnpicklePolicy(BasePolicy):

  environment = dict(
    unpickle = pickle.loads
  )
