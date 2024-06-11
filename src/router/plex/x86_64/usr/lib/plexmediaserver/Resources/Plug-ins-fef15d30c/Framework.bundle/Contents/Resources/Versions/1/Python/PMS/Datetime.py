#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import datetime, dateutil.parser, email.utils, time

####################################################################################################

def Now():
  return datetime.datetime.now()

####################################################################################################

def ParseDate(date):
  try:
    result = datetime.datetime.fromtimestamp(time.mktime(email.utils.parsedate(date)))
  except:
    result = dateutil.parser.parse(date)
  return result
  
####################################################################################################

def Delta(**kwargs):
  return datetime.timedelta(**kwargs)
  
####################################################################################################
