#!/usr/bin/python2.4
import Filter
import os.path, re, datetime, titlecase, unicodedata, sys

video_exts = ['3g2', '3gp', 'asf', 'asx', 'avc', 'avi', 'avs', 'bivx', 'bup', 'divx', 'dv', 'dvr-ms', 'evo', 'fli', 'flv',
              'm2t', 'm2ts', 'm2v', 'm4v', 'mkv', 'mov', 'mp4', 'mpeg', 'mpg', 'mts', 'nsv', 'nuv', 'ogm', 'ogv', 'tp',
              'pva', 'qt', 'rm', 'rmvb', 'sdp', 'svq3', 'strm', 'ts', 'ty', 'vdr', 'viv', 'vob', 'vp3', 'wmv', 'wtv', 'xsp', 'xvid', 'webm']

ignore_samples = ['[-\._]sample', 'sample[-\._]']
ignore_trailers = ['-trailer\.']
ignore_extras = ['^trailer.?$','-deleted\.', '-behindthescenes\.', '-interview\.', '-scene\.', '-featurette\.', '-short\.', '-other\.']
ignore_extras_startswith = ['^movie-trailer.*']
ignore_dirs =  ['\\bextras?\\b', '!?samples?', 'bonus', '.*bonus disc.*', 'bdmv', 'video_ts', '^interview.?$', '^scene.?$', '^trailer.?$', '^deleted.?(scene.?)?$', '^behind.?the.?scenes$', '^featurette.?$', '^short.?$', '^other.?$']
ignore_suffixes = ['.dvdmedia']

source_dict = {'bluray':['bdrc','bdrip','bluray','bd','brrip','hdrip','hddvd','hddvdrip'],'cam':['cam'],'dvd':['ddc','dvdrip','dvd','r1','r3','r5'],'retail':['retail'],
               'dtv':['dsr','dsrip','hdtv','pdtv','ppv'],'stv':['stv','tvrip'],'screener':['bdscr','dvdscr','dvdscreener','scr','screener'],
               'svcd':['svcd'],'vcd':['vcd'],'telecine':['tc','telecine'],'telesync':['ts','telesync'],'web':['webrip','web-dl'],'workprint':['wp','workprint']}
source = []
for d in source_dict:
  for s in source_dict[d]:
    if source != '':
      source.append(s)

audio = ['([^0-9])5\.1[ ]*ch(.)','([^0-9])5\.1([^0-9]?)','([^0-9])7\.1[ ]*ch(.)','([^0-9])7\.1([^0-9])']
subs = ['multi','multisubs']
misc = ['cd1','cd2','1cd','2cd','custom','internal','repack','read.nfo','readnfo','nfofix','proper','rerip','dubbed','subbed','extended','unrated','xxx','nfo','dvxa', 'web']
format = ['ac3','divx','fragment','limited','ogg','ogm','ntsc','pal','ps3avchd','r1','r3','r5','720i','720p','1080i','1080p','remux','x264','xvid','vorbis','aac','dts','fs','ws','1920x1080','1280x720','h264','h','264','prores','uhd','2160p','truehd','atmos','hevc']

reversed_tokens = set()
# use the values of 'format' and 'source' to generate the reversed tokens
for f in format + source:
  if len(f) > 3:
    # only add token if length > 3, otherwise the reversed token can easily exists in a normal (not reversed) name
    reversed_tokens.add(f[::-1].lower())

edition = ['se'] # se = special edition
yearRx = '([\(\[\.\-])([1-2][0-9]{3})([\.\-\)\]_,+])'

# Cleanup folder / filenames
def CleanName(name):
  name_tokens_lowercase = set()
  for t in re.split('([^ \-_\.\(\)+]+)', name):
    t = t.strip()
    if not re.match('[\.\-_\(\)+]+', t) and len(t) > 0:
      name_tokens_lowercase.add(t.lower())

  if len(set.intersection(name_tokens_lowercase, reversed_tokens)) > 2:
    # if name contains more than two reversed tokens, mirror the name
    name = name[::-1]
  
  orig = name

  # Make sure we pre-compose.  Try to decode with reported filesystem encoding, then with UTF-8 since some filesystems lie.
  try:
    name = unicodedata.normalize('NFKC', name.decode(sys.getfilesystemencoding()))
  except:
    try:
      name = unicodedata.normalize('NFKC', name.decode('utf-8'))
    except:
      pass

  name = name.lower()

  # grab the year, if there is one. set ourselves up to ignore everything after the year later on.
  year = None
  yearMatch = re.search(yearRx, name)
  if yearMatch:
    yearStr = yearMatch.group(2)
    yearInt = int(yearStr)
    if yearInt > 1900 and yearInt < (datetime.date.today().year + 1):
      year = int(yearStr)
      name = name.replace(yearMatch.group(1) + yearStr + yearMatch.group(3), ' *yearBreak* ')
    
  # Take out things in brackets. (sub acts weird here, so we have to do it a few times)
  done = False
  while done == False:
    (name, count) = re.subn(r'\[[^\]]+\]', '', name, re.IGNORECASE)
    if count == 0:
      done = True
  
  # Take out bogus suffixes.
  for suffix in ignore_suffixes:
    rx = re.compile(suffix + '$', re.IGNORECASE)
    name = rx.sub('', name)
  
  # Take out audio specs, after suffixing with space to simplify rx.
  name = name + ' '
  for s in audio:
    rx = re.compile(s, re.IGNORECASE)
    name = rx.sub(' ', name)
  
  # Now tokenize.
  tokens = re.split('([^ \-_\.\(\)+]+)', name)
  
  # Process tokens.
  newTokens = []
  for t in tokens:
    t = t.strip()
    if not re.match('[\.\-_\(\)+]+', t) and len(t) > 0:
    #if t not in ('.', '-', '_', '(', ')') and len(t) > 0:
      newTokens.append(t)

  # Now build a bitmap of good and bad tokens.
  tokenBitmap = []

  garbage = subs
  garbage.extend(misc)
  garbage.extend(format)
  garbage.extend(edition)
  garbage.extend(source)
  garbage.extend(video_exts)
  garbage = set(garbage)
  
  # Keep track of whether we've encountered a garbage token since they shouldn't appear more than once.
  seenTokens = {}

  # Go through the tokens backwards since the garbage most likely appears at the end of the file name.
  # If we've seen a token already, don't consider it garbage the second time.  Helps cases like "Internal.Affairs.1990-INTERNAL.mkv"
  #
  for t in reversed(newTokens):
    if t.lower() in garbage and t.lower() not in seenTokens:
      tokenBitmap.insert(0, False)
      seenTokens[t.lower()] = True
    else:
      tokenBitmap.insert(0, True)

  
  # Now strip out the garbage, with one heuristic; if we encounter 2+ BADs after encountering
  # a GOOD, take out the rest (even if they aren't BAD). Special case for director's cut.
  numGood = 0
  numBad  = 0
  
  finalTokens = []
  
  for i in range(len(tokenBitmap)):
    good = tokenBitmap[i]
    
    # If we've only got one or two tokens, don't whack any, they might be part of
    # the actual name (e.g. "Internal Affairs" "XXX 2")
    #
    if len(tokenBitmap) <= 2:
      good = True
    
    if good and numBad < 2:
      if newTokens[i] == '*yearBreak*':
        # If the year token is first just skip it and keep reading,
        # otherwise we can ignore everything after it.
        #
        if i == 0:
          continue
        else:
          break
      else:
        finalTokens.append(newTokens[i])
    elif not good and newTokens[i].lower() == 'web':
      # We're checking here for the 'web-dl' case (have to do it in two pieces b/c it gets split).
      if i+1 < len(newTokens) and newTokens[i+1].lower() in ['dl']:
        i += 1
    
    if good == True:
      numGood += 1
    else:
      numBad += 1

  # If we took *all* the tokens out, use the first one, otherwise we'll end up with no name at all.
  if len(finalTokens) == 0 and len(newTokens) > 0:
    finalTokens.append(newTokens[0])
    
  #print "CLEANED [%s] => [%s]" % (orig, u' '.join(finalTokens))
  #print "TOKENS: ", newTokens
  #print "BITMAP: ", tokenBitmap
  #print "FINAL:  ", finalTokens
  
  cleanedName = ' '.join(finalTokens)
  
  # If we failed to decode/encode above, we may still be dealing with a non-ASCII string here,
  # which will raise if we try to encode it, so let's just handle it and hope for the best!
  #
  try:
    cleanedName = cleanedName.encode('utf-8')
  except:
    pass
  
  return (titlecase.titlecase(cleanedName), year)

# Remove files that aren't videos.
def Scan(path, files, mediaList, subdirs, root=None):
  
  # Filter out bad stuff.
  Filter.Scan(path, files, mediaList, subdirs, video_exts, root)
  
  filesToRemove = []
  for i in files:
    # Remove files that aren't video.
    (file, ext) = os.path.splitext(i)
      
    # Remove sample files if they're smaller than 300MB.
    for rx in ignore_samples:
      if re.search(rx, i, re.IGNORECASE) and os.path.getsize(i) < 300 * 1024 * 1024:
        filesToRemove.append(i)

    # Remove trailer files.
    for rx in ignore_trailers:
      if re.search(rx, i, re.IGNORECASE):
        filesToRemove.append(i)

    # Remove things that look like extras.
    for rx in ignore_extras:
      if re.search(rx, i, re.IGNORECASE):
        filesToRemove.append(i)

    # Remove things that start with certain extras patterns.
    fn = os.path.basename(i)
    for rx in ignore_extras_startswith:
      if re.search(rx, fn, re.IGNORECASE):
        filesToRemove.append(i)
        
  # Uniquify and remove.
  filesToRemove = list(set(filesToRemove))
  for i in filesToRemove:
    files.remove(i)
      
  # Check directories, but not at the top-level.
  ignore_dirs_total = []
  if len(path) > 0:
    ignore_dirs_total += ignore_dirs
  
  whack = []
  for dir in subdirs:
    baseDir = os.path.basename(dir)
    for rx in ignore_dirs_total:
      if re.match(rx, baseDir, re.IGNORECASE):
        whack.append(dir)
        break
  
  for w in whack:
    subdirs.remove(w)
      
def RetrieveSource(name):
  name = os.path.basename(name)
  wordSplitter = re.compile(r"[^a-zA-Z0-9']+", re.IGNORECASE)
  words = wordSplitter.split(name.upper())
  sources = source
  foundSources = []
  
  # Return the first one we find.
  for d in source_dict:
    for s in source_dict[d]:
      if s.upper() in words:
        return d

# Find the first occurance of a year.
def FindYear(words):
  yearRx = '^[1-2][0-9]{3}$'
  i = 0
  for w in words:
    if re.match(yearRx, w):
      year = int(w)
      if year > 1900 and year < datetime.date.today().year + 1:
        return i
    i += 1
    
  return None
