#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import PMS, socket

####################################################################################################

def Hostname():
  return socket.gethostname()

####################################################################################################
  
def Address():
  try:
    return socket.gethostbyaddr(socket.gethostname())[2][0]
  except:
    return None
    
####################################################################################################