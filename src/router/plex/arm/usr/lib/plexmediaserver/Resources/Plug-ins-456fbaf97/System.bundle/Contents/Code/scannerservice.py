#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService
import cgi
import os

class ScannerService(SystemService):
  """
    The scanner service provides information about scanners available to the
    media server.
  """

  def __init__(self, system):
    SystemService.__init__(self, system)

    Route.Connect('/system/scanners/{mediaType}', self.list)

    self.scanner_types = {
      1: 'Movies',
      2: 'Series',
      8: 'Music',
      9: 'Music',
     13: 'Photos',
	 # Game type has been removed
	 # 19: 'Games'
    }

  def list(self, mediaType):
    """
      Lists scanners that handle the specified media type.
    """

    # Convert the mediaType from PMS into a class name
    if int(mediaType) not in self.scanner_types: return
    scanner_type = self.scanner_types[int(mediaType)]

    d = MediaContainer()

    bundle_path = Core.path_for_bundle('Scanners', 'com.plexapp.system.scanners')
    scanner_paths = [Core.storage.join_path(bundle_path, 'Contents', 'Resources', scanner_type), # System path
                     Core.storage.join_path(Core.app_support_path, 'Scanners', scanner_type)]    # User path

    scanners = []
    for path in scanner_paths:
      if os.path.exists(path):
        scanners += os.listdir(path)

    for scanner in set([os.path.splitext(s)[0] for s in scanners if os.path.splitext(s)[1] in ('.py', '.pyc')]):
      con = XMLContainer(name=scanner)
      con.tagName = 'Scanner'
      d.Append(con)

    return d
