#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import os
import sys
import time
import mimetypes
import hashlib
import urllib
import base64
import cerealizer
import types

_urllib_quote = urllib.quote
_urllib_quote_plus = urllib.quote_plus

def version_at_least(version, *components):
  if version == None:
    return False
  string_parts = version.split('-')[0].split('.')
  int_parts = []
  for part in string_parts:
    try:
      int_parts.append(int(part))
    except:
      int_parts.append(0)
  parts = tuple(int_parts)
  return parts >= tuple(components)

def makedirs(path):
  if not os.path.exists(path):
    os.makedirs(path)
    
def timestamp_from_datetime(dt):
  return time.mktime(dt.timetuple())
  
def guess_mime_type(filename):
  mtype = mimetypes.guess_type(filename)[0]
  if mtype: return mtype
  else: return 'application/octet-stream'
  
def urlencode(string):
  encoded = urllib.urlencode({'v':string})
  return encoded[2:]

def unistr(s):
  if isinstance(s, unicode):
    s = s.encode('raw_unicode_escape', 'strict')
  return s
  
def quote(s, *args, **kwargs):
  return _urllib_quote(unistr(s), *args, **kwargs)

def quote_plus(s, *args, **kwargs):
  return _urllib_quote_plus(unistr(s), *args, **kwargs)
  
def safe_encode(string):
  return base64.b64encode(string).replace('/','@').replace('+','*').replace('=','_')

def safe_decode(string):
  return base64.b64decode(string.replace('@','/').replace('*','+').replace('_','=') + '=' * (4 - len(string) % 4))

def pack(obj):
  serialized_obj = cerealizer.dumps(obj)
  encoded_string = safe_encode(serialized_obj)
  return urllib.quote(encoded_string)
  
def unpack(string):
  unquoted_string = urllib.unquote(string)
  decoded_string = safe_decode(unquoted_string)
  return cerealizer.loads(decoded_string)
  
# Checks whether a function accepts a named argument
def function_accepts_arg(f, argname):
  if isinstance(f, types.MethodType):
    f = f.im_func
  if isinstance(f, types.FunctionType):
    return argname in f.func_code.co_varnames[:f.func_code.co_argcount]
  return False

# Test for common truth values
def is_true(value):
  return value in (True, 1, 'true', 'True')


# using module re to pluralize most common english words
# (rule_tuple used as function default, so establish it first)

import re, string

# (pattern, search, replace) regex english plural rules tuple
rule_tuple = (
('[ml]ouse$', '([ml])ouse$', '\\1ice'), 
('child$', 'child$', 'children'), 
('booth$', 'booth$', 'booths'), 
('foot$', 'foot$', 'feet'), 
('ooth$', 'ooth$', 'eeth'), 
('l[eo]af$', 'l([eo])af$', 'l\\1aves'), 
('sis$', 'sis$', 'ses'), 
('man$', 'man$', 'men'), 
('ife$', 'ife$', 'ives'), 
('eau$', 'eau$', 'eaux'), 
('lf$', 'lf$', 'lves'), 
('[sxz]$', '$', 'es'), 
('[^aeioudgkprt]h$', '$', 'es'), 
('(qu|[^aeiou])y$', 'y$', 'ies'), 
('$', '$', 's')
)

def regex_rules(rules=rule_tuple):
    for line in rules:
        pattern, search, replace = line
        yield lambda word: re.search(pattern, word) and re.sub(search, replace, word)

def plural(noun):
    for rule in regex_rules():
        result = rule(noun)
        if result: 
            return result
            

def clean_up_string(s):
  s = unicode(s)

  # Ands.
  s = s.replace('&', 'and')

  # Pre-process the string a bit to remove punctuation.
  s = re.sub('[' + string.punctuation + ']', '', s)
  
  # Lowercase it.
  s = s.lower()
  
  # Strip leading "the/a"
  s = re.sub('^(the|a) ', '', s)
  
  # Spaces.
  s = re.sub('[ ]+', ' ', s).strip()
    
  return s


# Parse client information from the headers.
def parse_codec(value):
  # e.g. h264{profile:high&resolution:1080&level:51}
  
  # Check whether the value has args
  if value[-1] == '}':
    # Split the args from the codec name
    codec, arg_string = value[:-1].split('{')
    
    # Convert the arg string into a dictionary
    args = {}
    for arg in arg_string.split('&'):
      k, v = arg.split(':')
      args[k] = v
      
  # No args - return an empty dict
  else:
    codec = value
    args = {}
  return codec, args


# From http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Longest_common_substring#Python
def longest_common_substring(first, second):
  S = clean_up_string(first)
  T = clean_up_string(second)
  
  m = len(S); n = len(T)
  L = [[0] * (n+1) for i in xrange(m+1)]
  LCS = set()
  longest = 0
  for i in xrange(m):
    for j in xrange(n):
      if S[i] == T[j]:
        v = L[i][j] + 1
        L[i+1][j+1] = v
        if v > longest:
          longest = v
          LCS = set()
        if v == longest:
          LCS.add(S[i-v+1:i+1])
  if len(LCS) > 0:
    return LCS.pop()
  return ''
            
# TODO: Attribution http://www.korokithakis.net/node/87
def levenshtein_distance(first, second):
  first = clean_up_string(first)
  second = clean_up_string(second)
  
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

urllib.quote = quote
urllib.quote_plus = quote_plus


def ps_import(name):
  """
    Platform-specific import function - imports bundled "name_new" modules when targeting
    Python on OS X or Linux.
  """
  try:
    if sys.platform != 'win32':
      mod = __import__(name + '_new')
      return mod
  except:
    pass
  return __import__(name)
