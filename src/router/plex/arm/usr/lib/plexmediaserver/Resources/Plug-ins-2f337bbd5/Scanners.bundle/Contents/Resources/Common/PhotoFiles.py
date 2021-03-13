#!/usr/bin/env python

photo_exts = ['png','jpg','jpeg','bmp','gif','ico','tif','tiff','tga','pcx','dng','nef','cr2','crw','orf','arw','erf','3fr','dcr','x3f','mef','raf','mrw','pef','sr2', 'mpo', 'jps', 'rw2', 'jp2', 'j2k']

import os, os.path, time
import Filter, Media, VideoFiles

def Scan(path, files, mediaList, subdirs, language=None, root=None, **kwargs):
  # Filter out bad stuff.
  Filter.Scan(path, files, mediaList, subdirs, photo_exts + VideoFiles.video_exts, root)

  # Add all the photos to the list.
  for path in files:
    file = os.path.basename(path)
    title,ext = os.path.splitext(file)
    ext = ext[1:].lower()

    if ext in photo_exts:
      photo = Media.Photo(title)

      # Creation date, year.
      try:
        created_at = time.gmtime(os.path.getmtime(path))
        photo.released_at = time.strftime('%Y-%m-%d %H:%M:%S', created_at)
        photo.year = int(time.strftime('%Y', created_at))
      except:
        print 'Unable to get mtime for photo'

      # Add the photo.
      photo.parts.append(path)
      mediaList.append(photo)
    else:
      # Add the video.
      video = Media.Video(title)
      video.parts.append(path)
      mediaList.append(video)