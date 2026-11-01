import os, re, string, unicodedata, sys
from urllib import urlopen, urlencode
import UnicodeHelper

RE_UNICODE_CONTROL =  u'([\u0000-\u0008\u000b-\u000c\u000e-\u001f\ufffe-\uffff])' + \
                      u'|' + \
                      u'([%s-%s][^%s-%s])|([^%s-%s][%s-%s])|([%s-%s]$)|(^[%s-%s])' % \
                      (
                        unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                        unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                        unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff)
                      )

# Platform-safe function to split a path into a list of path elements.
def SplitPath(path, maxdepth=20):
    (head, tail) = os.path.split(path)
    if maxdepth and head and head != path:
        return SplitPath(head, maxdepth - 1) + [tail]
    else:
        return [head or tail]

# Check for a given filename in a list of full paths.
def ContainsFile(files, file):
  for i in files:
    if os.path.basename(i).lower() == file.lower():
      return i
  return None

# Log to PMS log.
def Log(message, level=3, source='Scanners.bundle'):
  try:
    try: message = unicode(message, errors='replace').encode('utf-8', errors='replace')
    except Exception, decode_ex:
      try: message = message.encode('utf-8', errors='replace')
      except Exception, encode_ex:
        message = 'Log message suppressed due to exceptions: %s, %s' % (decode_ex, encode_ex)
    args = urlencode({'message' : UnicodeHelper.toBytes(message), 'level' : level, 'source' : source, 'X-Plex-Token' : os.environ['X_PLEX_TOKEN']})
    res = urlopen('http://127.0.0.1:32400/log?%s' % args)
    res.close()
  except: pass
  
# Safely return Unicode.
def Unicodize(s, lang):

  # Precompose.
  try: s = unicodedata.normalize('NFKD', s.decode('utf-8'))
  except:
    try: s = unicodedata.normalize('NFKD', s.decode(sys.getdefaultencoding()))
    except:
      try: s = unicodedata.normalize('NFKD', s.decode(sys.getfilesystemencoding()))
      except:
        try: s = unicodedata.normalize('NFKD', s.decode('ISO-8859-1'))
        except:
          try: s = unicodedata.normalize('NFKD', s)
          except Exception, e:
            Log(type(e).__name__ + ' exception precomposing: ' + str(e))

  # Strip control characters.
  s = re.sub(RE_UNICODE_CONTROL, '', s)

  return s
  
# Cleanup string.
def CleanUpString(s):

  # Precompose.
  try:
    s = unicodedata.normalize('NFKD', s.decode('utf-8'))
  except (UnicodeError, UnicodeDecodeError):
    try: s = unicodedata.normalize('NFKD', s)
    except: pass

  # Ands.
  s = s.replace('&', 'and')

  # Rearrange trailing ", the"
  if s.lower().endswith(', the'):
    s = 'the ' + s[:-5]

  # Strip diacritics and punctuation.
  s = u''.join([c for c in s if not unicodedata.combining(c) and not unicodedata.category(c).startswith('P')])
  
  # Lowercase it.
  s = s.lower()
  
  # Strip leading "the/a"
  s = re.sub('^(the|a) ', '', s)

  # Symbols.
  s = re.sub('[\'";,-]+', '', s)
  
  # Spaces.
  s = re.sub('[ ]+', ' ', s).strip()

  return s
  
# Compute Levenshtein distance.
def LevenshteinDistance(first, second):
  first = CleanUpString(first)
  second = CleanUpString(second)
  
  if len(first) > len(second):
    first, second = second, first
  if len(second) == 0:
    return len(first)
  first_length = len(first) + 1
  second_length = len(second) + 1
  distance_matrix = [[0] * second_length for x in range(first_length)]
  for i in range(first_length):
    distance_matrix[i][0] = i
  for j in range(second_length):
    distance_matrix[0][j]=j
  for i in xrange(1, first_length):
    for j in range(1, second_length):
      deletion = distance_matrix[i-1][j] + 1
      insertion = distance_matrix[i][j-1] + 1
      substitution = distance_matrix[i-1][j-1]
      if first[i-1] != second[j-1]:
        substitution = substitution + 1
      distance_matrix[i][j] = min(insertion, deletion, substitution)
  return distance_matrix[first_length-1][second_length-1]

# Levenshtein ratio.
def LevenshteinRatio(first, second):
  if len(first) == 0 or len(second) == 0:
    return 0
  else:
    return 1 - (LevenshteinDistance(first, second) / float(max(len(first), len(second))))
