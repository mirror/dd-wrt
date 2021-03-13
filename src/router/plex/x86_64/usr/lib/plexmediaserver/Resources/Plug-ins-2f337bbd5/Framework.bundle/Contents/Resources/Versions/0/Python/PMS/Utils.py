#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  Utility functions.
"""

from datetime import datetime
import base64

####################################################################################################
def WeekdayToString(day):
  """
    Gets the textual representation of a weekday
    
    @param day: The day of the week
    @type day: int
    @return: string
  """
  if   day == 0: return "Monday"
  elif day == 1: return "Tuesday"
  elif day == 2: return "Wednesday"
  elif day == 3: return "Thursday"
  elif day == 4: return "Friday"
  elif day == 5: return "Saturday"
  elif day == 6: return "Sunday"
  else: return None
  
####################################################################################################
def Today():
  """
    Return today's date.
  """
  return datetime.today()
  
####################################################################################################
def EncodeStringToUrlPath(str):
    return base64.b64encode(str).replace('/','@').replace('+','#')

####################################################################################################
def DecodeUrlPathToString(str):
    return base64.b64decode(str.replace('@','/').replace('#','+'))

####################################################################################################

def BuildPath(pathNouns):
  """
    Builds a path from a given array of nouns.
    
    @param pathNouns: The nouns to use when constructing the path.
    @type pathNouns: array
    @return: string
  """
  if len(pathNouns) > 0:
    query = pathNouns[0]
    if len(pathNouns) > 1:
      for i in range(1, len(pathNouns)): query += "/%s" % pathNouns[i]  
    return query

####################################################################################################
