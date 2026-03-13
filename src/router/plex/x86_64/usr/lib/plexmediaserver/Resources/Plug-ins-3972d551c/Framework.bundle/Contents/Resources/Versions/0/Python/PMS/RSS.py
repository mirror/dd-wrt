#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  WIP: Functions for handling RSS content.
"""

# TODO: Add more functionality

import feedparser

####################################################################################################

def Parse(feed):
  """
    Parse the given RSS feed.
    
    @param feed: The feed to parse (can be a file path, URL or string)
    @type feed: string
  """
  return feedparser.parse(feed)
  
####################################################################################################
