#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import feedparser
import HTTP

####################################################################################################

def FeedFromString(feed):
  return feedparser.parse(feed)
  
####################################################################################################

def FeedFromURL(url, values=None, headers={}, cacheTime=None, autoUpdate=False):
  return FeedFromString(HTTP.Request(url, values=values, headers=headers, cacheTime=cacheTime, autoUpdate=autoUpdate))
  
####################################################################################################
