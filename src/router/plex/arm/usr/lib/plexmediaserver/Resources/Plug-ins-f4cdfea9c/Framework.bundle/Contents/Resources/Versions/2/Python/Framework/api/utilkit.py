import Framework
from operator import itemgetter, attrgetter
import random
import base64
import urllib
import string
import uuid
import unicodedata
import os
import re
import datetime
import dateutil.parser
import email.utils
import time
import math
import textwrap
import urlparse
import cgi
import sys

import pyamf
from pyamf.remoting.client import RemotingService
from pyamf import sol

from BeautifulSoup import BeautifulSoup

from base import BaseKit
from networkkit import BaseHTTPKit


class ArchiveKit(BaseHTTPKit):
  
  def Zip(self, data=None):
    return self._core.data.archiving.zip_archive(data)
    

  def ZipFromURL(self, url, values=None, headers={}, cacheTime=None, encoding=None, errors=None, sleep=0):
    # Update the cache time
    self._context.cache_time = cacheTime if values == None else 0
    
    return self.Zip(self._http_request(
      url = url,
      values = values,
      headers = headers,
      cacheTime = cacheTime,
      encoding = encoding,
      errors = errors,
      immediate = True,
      sleep = sleep,
      opener = self._opener,
    ).content)


  def GzipCompress(self, data):
    return self._core.data.archiving.gzip_compress(data)

    
  def GzipDecompress(self, data):
    return self._core.data.archiving.gzip_decompress(data)



class SOL(object):
  def __init__(self, *args):
    if len(args) < 2:
      raise Framework.exceptions.FrameworkException('AMF.SOL() requires at least two arguments')
      
    #TODO: Paths for Linux & Windows
    sol_path = os.path.expanduser("~/Library/Preferences/Macromedia/Flash Player/#SharedObjects")
    subdir = [d for d in os.listdir(sol_path) if re.match("^[A-Z0-9]{8}$", d)][0]  # hopefully there's only one...
    self._path = os.path.join(sol_path, subdir, *args) + '.sol'
    if os.path.exists(self._path):
      self._sol = sol.load(self._path)
    else:
      self._sol = sol.SOL(args[-1])
      
  def save(self, encoding=0):
    sol_dir = os.path.dirname(self._path)
    if not os.path.exists(sol_dir):
      os.makedirs(sol_dir)
    self._sol.save(self._path, encoding)
      
  def __cmp__(self, other):                   return self._sol.__cmp__(other)                   
  def __contains__(self, item):               return self._sol.__contains__(item)               
  def __hash__(self):                         return self._sol.__hash__()                       
  def __getitem__(self, name):                return self._sol.__getitem__(name)                
  def __setitem__(self, name, value):         return self._sol.__setitem__(name, value)         
  def __delitem__(self, name):                return self._sol.__delitem__(name)                
                                                                                                
  def __lt__(self, other):                    return self._sol.__lt__(other)                    
  def __le__(self, other):                    return self._sol.__le__(other)                    
  def __eq__(self, other):                    return self._sol.__eq__(other)                    
  def __ne__(self, other):                    return self._sol.__ne__(other)                    
  def __gt__(self, other):                    return self._sol.__gt__(other)                    
  def __ge__(self, other):                    return self._sol.__ge__(other)                    
                                                                                                
  def clear(self):                            return self._sol.clear()                          
  def get(self, key, default=None):           return self._sol.get(key, default)                
  def has_key(self, key):                     return self._sol.has_key(key)                     
  def items(self):                            return self._sol.items()                          
  def iteritems(self):                        return self._sol.iteritems()                      
  def iterkeys(self):                         return self._sol.iterkeys()                       
  def keys(self):                             return self._sol.keys()                           
  def pop(self, key, default=None):           return self._sol.pop(key, default)                
  def popitem(self):                          return self._sol.popitem()                        
  def setdefault(self, key, default=None):    return self._sol.setdefault(key, default)
  def update(self, *args, **kwargs):          return self._sol.update(*args, **kwargs)
  def values(self):                           return self._sol.values()                           
  
  

class AMFKit(BaseKit):
    
  def RemotingService(self, *args, **kwargs):
    # Fix for code that interacted with the older PyAMF.
    if 'client_type' in kwargs:
      self._core.log.warn("The 'client_type' argument of AMF.RemotingService is deprecated. Please use 'amf_version' instead.")
      if 'amf_version' not in kwargs:
        kwargs['amf_version'] = kwargs['client_type']
      del kwargs['client_type']

    # If no AMF version is specified, default to 3, since this will work for more cases than 0.
    if 'amf_version' not in kwargs:
      self._core.log.warn("No 'amf_version' argument provided to AMF.RemotingService - defaulting to 3.")
      kwargs['amf_version'] = 3
      
    return RemotingService(*args, **kwargs)
    

  @property
  def RegisterClass(self):
    return pyamf.register_class
    

  def SOL(self, *args):
    return SOL(*args)
    


class HashKit(BaseKit):

  def MD5(self, data, digest=False):
    return self._core.data.hashing.md5(data, digest)


  def SHA1(self, data, digest=False):
    return self._core.data.hashing.sha1(data, digest)


  def SHA224(self, data, digest=False):
    return self._core.data.hashing.sha224(data, digest)


  def SHA256(self, data, digest=False):
    return self._core.data.hashing.sha256(data, digest)


  def SHA384(self, data, digest=False):
    return self._core.data.hashing.sha384(data, digest)


  def SHA512(self, data, digest=False):
    return self._core.data.hashing.sha512(data, digest)


  def CRC32(self, data):
    return self._core.data.hashing.crc32(data)



class RegexKit(BaseKit):

  IGNORECASE = re.IGNORECASE
  MULTILINE = re.MULTILINE
  DOTALL = re.DOTALL

  def __call__(self, pattern, flags=0):
    return re.compile(pattern, flags)



class StringKit(BaseKit):

  def _init(self):
    self.LETTERS = string.ascii_letters
    self.LOWERCASE = string.ascii_lowercase
    self.UPPERCASE = string.ascii_uppercase
    self.DIGITS = string.digits
    self.HEX_DIGITS = string.hexdigits
    self.LETTERS = string.letters
    self.OCT_DIGITS = string.octdigits
    self.PUNCTUATION = string.punctuation
    self.PRINTABLE = string.printable
    self.WHITESPACE = string.whitespace


  def Encode(self, s):
    """
      Encodes the given string using the framework's standard encoding (a slight variant on
      Base64 which ensures that the string can be safely used as part of a URL).
    """
    return Framework.utils.safe_encode(s)


  def Decode(self, s):
    """
      Decodes a string previously encoded using the above function.
    """
    return Framework.utils.safe_decode(s)


  def Base64Encode(self, s, with_newlines=False):
    """
      Encodes the given string using Base64 encoding.
    """
    if with_newlines:
      return base64.encodestring(s)
    else:
      return base64.b64encode(s)


  def Base64Decode(self, s):
    """
      Decodes the given Base64-encoded string.
    """
    return base64.decodestring(s)


  def Quote(self, s, usePlus=False):
    """
      Replaces special characters in *s* using the ``%xx`` escape. Letters,
      digits, and the characters ``'_.-'`` are never quoted. If *usePlus* is ``True``,
      spaces are escaped as a plus character instead of ``%20``.
    """
    if usePlus:
      return urllib.quote_plus(s)
    else:
      return urllib.quote(s)


  def URLEncode(self, s):
    return Framework.utils.urlencode(s)


  def Unquote(self, s, usePlus=False):
    """
      Replace ``%xx`` escapes by their single-character equivalent. If *usePlus* is ``True``,
      plus characters are replaced with spaces.
    """
    if usePlus:
      return urllib.unquote_plus(s)
    else:
      return urllib.unquote(s)


  def Join(self, words, sep=None):
    return string.join(words, sep)


  def JoinURL(self, base, url, allow_fragments=True):
    return urlparse.urljoin(base, url, allow_fragments)


  def StripTags(self, s):
    """
      Removes HTML tags from a given string.
    """
    return re.sub(r'<[^<>]+>', '', s)


  def DecodeHTMLEntities(self, s):
    """
      Converts HTML entities into regular characters (e.g. "&amp;"" => "&")
    """
    soup = BeautifulSoup(s, convertEntities=BeautifulSoup.HTML_ENTITIES)
    return unicode(soup)


  def UUID(self):
    """
      Generates a universally unique identifier (UUID) string. This string is guaranteed to be unique.
    """
    return str(uuid.uuid4())


  def StripDiacritics(self, s):
    """
      Removes diacritics from a given string.
    """
    u = unicode(s).replace(u"\u00df", u"ss").replace(u"\u1e9e", u"SS")
    nkfd_form = unicodedata.normalize('NFKD', u)
    only_ascii = nkfd_form.encode('ASCII', 'ignore')
    return only_ascii


  def Pluralize(self, s):
    """
      Attempts to return a pluralized version of the given string (e.g. converts ``boot`` to ``boots``).
    """
    return Framework.utils.plural(s)
    

  def LevenshteinDistance(self, first, second):
    """
      Computes the `Levenshtein distance <http://en.wikipedia.org/wiki/Levenshtein_distance>`_ between two given strings.
    """
    return Framework.utils.levenshtein_distance(first, second)


  def LevenshteinRatio(self, first, second):
    """
      Computes the `Levenshtein ratio (0-1) <http://en.wikipedia.org/wiki/Levenshtein_distance>`_ between two given strings.
    """
    if len(first) == 0 or len(second) == 0:
      return 0.0
    else:
      return 1 - (Framework.utils.levenshtein_distance(first, second) / float(max(len(first), len(second))))


  def LongestCommonSubstring(self, first, second):
    """
      Returns the longest substring contained within both strings.
    """
    return Framework.utils.longest_common_substring(first, second)


  def CapitalizeWords(self, s):
    return string.capwords(s)


  def ParseQueryString(self, s, keep_blank_vaues=True, strict_parsing=False):
    return cgi.parse_qs(s, keep_blank_vaues, strict_parsing)


  def ParseQueryStringAsList(self, s, keep_blank_vaues=True, strict_parsing=False):
    return cgi.parse_qsl(s, keep_blank_vaues, strict_parsing)


  def SplitExtension(self, s):
    return os.path.splitext(s)


  def Dedent(self, s):
    return textwrap.dedent(s)

  def Clean(self, s, form='NFKD', lang=None, strip_diacritics=False, strip_punctuation=False):

    # Guess at a language-specific encoding, should we need one.
    encoding_map = {'ko' : 'cp949'}

    # Precompose.
    try: s = unicodedata.normalize(form, s.decode('utf-8'))
    except:
      try: s = unicodedata.normalize(form, s.decode(sys.getdefaultencoding()))
      except:
        try: s = unicodedata.normalize(form, s.decode(sys.getfilesystemencoding()))
        except:
          try: s = unicodedata.normalize(form, s.decode('utf-16'))
          except:
            try: s = unicodedata.normalize(form, s.decode(encoding_map.get(lang, 'ISO-8859-1')))
            except:
              try: s = unicodedata.normalize(form, s)
              except Exception, e:
                Log(type(e).__name__ + ' exception precomposing: ' + str(e))

    # Strip control characters. No good can come of these.
    s = u''.join([c for c in s if not unicodedata.category(c).startswith('C')])

    # Strip punctuation if we were asked to.
    if strip_punctuation:
      s = u''.join([c for c in s if not unicodedata.category(c).startswith('P')])

    # Strip diacritics if we were asked to.
    if strip_diacritics:
      s = u''.join([c for c in s if not unicodedata.combining(c)])

    return s

class DatetimeKit(BaseKit):

  def Now(self):
    """
      Returns the current date and time.
      
      :rtype: `datetime <http://docs.python.org/library/datetime.html#datetime-objects>`_
    """
    return datetime.datetime.now()


  def UTCNow(self):
    """
      Returns the current UTC date and time.

      :rtype: `datetime <http://docs.python.org/library/datetime.html#datetime-objects>`_
    """
    return datetime.datetime.utcnow()


  def ParseDate(self, date, fmt=None):
    """
      Attempts to convert the given string into a datetime object.
      
      :type date: str
      
      :rtype: `datetime <http://docs.python.org/library/datetime.html#datetime-objects>`_
    """
    if date == None or len(date) == 0:
      return None #TODO: Should we return None or throw an exception here?
    try:
      year_only = re.compile(r'[0-9]{4}')
      year_month_date = re.compile(r'[0-9]{4}-[0-9]{2}-[0-9]{2}')
      if fmt != None:
        result = datetime.datetime.strptime(date, fmt)
      elif year_month_date.match(date):
        result = datetime.datetime.strptime(date, "%Y-%m-%d")
      elif year_only.match(date):
        result = datetime.datetime.strptime(date + '-01-01', "%Y-%m-%d")
      else:
        result = datetime.datetime.fromtimestamp(time.mktime(email.utils.parsedate(date)))
    except:
      result = dateutil.parser.parse(date)
    return result


  def Delta(self, **kwargs):
    """
      Creates a *timedelta* object representing the duration given as keyword arguments. Valid
      argument names are `days`, `seconds`, `microseconds`, `milliseconds`, `minutes`, `hours`
      and `weeks`. This object can then be added to or subtracted from an existing *datetime*
      object.
      
      :rtype: `timedelta <http://docs.python.org/library/datetime.html#timedelta-objects>`_
    """
    return datetime.timedelta(**kwargs)


  def TimestampFromDatetime(self, dt):
    """
      Converts the given *datetime* object to a UNIX timestamp.
      
      :rtype: int
    """
    return Framework.utils.timestamp_from_datetime(dt)
    

  def FromTimestamp(self, ts):
    """
      Converts the given UNIX timestamp to a *datetime* object.
      
      :rtype: `datetime <http://docs.python.org/library/datetime.html#datetime-objects>`_
    """
    return datetime.datetime.fromtimestamp(ts)


  def MillisecondsFromString(self, s):
    seconds  = 0
    duration = s.split(':')
    duration.reverse()

    if len(duration) > 3:
      raise Framework.exceptions.FrameworkException("Too many components in time string '%s'" % s)

    for i in range(0, len(duration)):
      seconds += int(duration[i]) * (60**i)

    return seconds * 1000





class UtilKit(BaseKit):

  def _init(self):
    self._publish(AMFKit)
    self._publish(HashKit)
    self._publish(StringKit)
    self._publish(DatetimeKit)
    self._publish(ArchiveKit)
    self._publish(RegexKit)
    self._publish(StringKit.Encode, name='E')
    self._publish(StringKit.Decode, name='D')


  def Floor(self, x):
    return math.floor(x)


  def Ceiling(self, x):
    return math.ceil(x)


  def VersionAtLeast(self, version_string, *components):
    return Framework.utils.version_at_least(version_string, *components)

  
  def ListSortedByKey(self, l, key):
    return sorted(l, key=itemgetter(key))

  
  def ListSortedByAttr(self, l, attr):
    return sorted(l, key=attrgetter(attr))


  def SortListByKey(self, l, key):
    l.sort(key=itemgetter(key))


  def SortListByAttr(self, l, attr):
    l.sort(key=attrgetter(attr))

    
  def LevenshteinDistance(self, first, second):
    return Framework.utils.levenshtein_distance(first, second)

    
  def LongestCommonSubstring(self, first, second):
    return Framework.utils.longest_common_substring(first, second)

    
  def Random(self):
    """
      Returns a random number between 0 and 1.
      
      :rtype: float
    """
    return random.random()

    
  def RandomInt(self, a, b):
    """
      Returns a random integer *N* such that ``a <= N <= b``.
    """
    return random.randint(a, b)
    

  def RandomItemFromList(self, l):
    """
      Returns a random item selected from the given list.
    """
    return l[random.randint(0,len(l)-1)]


  def RandomChoice(self, l):
    return random.choice(l)


  def RandomSample(self, l, count):
    return random.samle(l, count)



