#
# Copyright (c) 2010-2011 Plex Development Team. All rights reserved.
#

import AudioFiles

def Scan(path, files, mediaList, subdirs, language=None, root=None):
  # Scan for audio files.
  AudioFiles.Scan(path, files, mediaList, subdirs, root)

  # Read tags, etc. and build up the mediaList
  AudioFiles.Process(path, files, mediaList, subdirs, language, root)
