
# Import standard Python exceptions
import Framework
import sys

from httplib import IncompleteRead
from socket import timeout as SocketTimeout
from socket import error as SocketError
from urllib2 import URLError
from urllib2 import HTTPError


class FrameworkException(Exception): pass
class UnauthorizedException(Exception): pass
class BadRequestException(Exception): pass
class AttributeException(Exception): pass

class RedirectError(Exception):
  def __init__(self, code, headers):
    self.code = code
    self.headers = headers
  
  @property
  def location(self):
    return self.headers['Location']

class PlexError(Exception):
  def __init__(self, code, status, traceback=None):
    Exception.__init__(self, code, status)
    self.code = code
    self.status = status
    self.traceback = traceback

class PlexNonCriticalError(PlexError): pass

class MediaNotAvailable(PlexNonCriticalError):
  def __init__(self):
  	PlexError.__init__(self, 2001, "This media is not currently available.")

class MediaExpired(PlexNonCriticalError):
  def __init__(self):
  	PlexError.__init__(self, 2002, "This media has expired.")

class LiveMediaNotStarted(PlexNonCriticalError):
  def __init__(self):
  	PlexError.__init__(self, 2003, "This live media has not yet started.")

class MediaNotAuthorized(PlexNonCriticalError):
  def __init__(self):
  	PlexError.__init__(self, 2004, "You are not authorized to access this media.")

class MediaGeoblocked(PlexNonCriticalError):
  def __init__(self):
  	PlexError.__init__(self, 2005, "This media is geoblocked and can't be accessed from your current location.")
  	
class StreamLimitExceeded(PlexNonCriticalError):
  def __init__(self):
    PlexError.__init__(self, 2006, "You have reached the limit of streams from this service. Please close one and try again.")

class AttributeTypeMismatch(PlexError):
  def __init__(self, status):
    PlexError.__init__(self, 2101, status)

class ContextException(PlexError):
  def __init__(self, status):
    PlexError.__init__(self, 2102, status)

class APIException(PlexError):
  def __init__(self, status):
    PlexError.__init__(self, 2103, status)

class NonCriticalArgumentException(PlexNonCriticalError):
  def __init__(self, status):
    PlexNonCriticalError.__init__(self, 2104, status)

