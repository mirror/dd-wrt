#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework


class BaseInterface(Framework.CoreObject):
    
  def listen(self, daemonized):
    pass
