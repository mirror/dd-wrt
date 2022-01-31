# opensubtitles.org
# Subtitles service allowed by www.OpenSubtitles.org
# Language codes: http://www.opensubtitles.org/addons/export_languages.php

import difflib, os

OS_API = 'http://plexapp.api.opensubtitles.org/xml-rpc'
OS_PLEX_USERAGENT = 'plexapp.com v9.0'
SUBTITLE_EXT = ['utf','utf8','utf-8','sub','srt','smi','rt','ssa','aqt','jss','ass','idx']

####################################################################################################
def Start():

  HTTP.CacheTime = CACHE_1DAY
  HTTP.Headers['User-Agent'] = OS_PLEX_USERAGENT

  if 'quotaReached' not in Dict:

    Dict['quotaReached'] = int(Datetime.TimestampFromDatetime(Datetime.Now())) - (24*60*60)
    Dict.Save()

####################################################################################################
def opensubtitlesProxy():

  proxy = XMLRPC.Proxy(OS_API)
  username = Prefs['username'] if Prefs['username'] else ''
  password = Prefs['password'] if Prefs['password'] else ''

  # Check for missing token
  if 'proxyToken' not in Dict:
    # Perform login
    Log('No valid token in Dict.')
    (success, token) = proxyLogin(proxy, username, password)

    if success:
      Dict['proxyToken'] = token
      return (proxy, token)
    else:
      Dict['proxyToken'] = ''
      return (proxy, '')

  else:
    # Token already exists, check if it's still valid
    Log('Existing token found. Revalidating.')
    if Dict['proxyToken'] != '' and checkToken(proxy, Dict['proxyToken']):
      return (proxy, Dict['proxyToken'])
    else:
      # Invalid token. Re-authenticate.
      (success, token) = proxyLogin(proxy, username, password)

      if success:
        Dict['proxyToken'] = token
        return (proxy, token)
      else:
        return (proxy, '')

####################################################################################################
def proxyLogin(proxy, username, password):

  token = proxy.LogIn(username, password, 'en', OS_PLEX_USERAGENT)['token']

  if checkToken(proxy, token):
    Log('Successful login.')
    return (True, token)
  else:
    Log('Unsuccessful login.')
    return (False, '')

####################################################################################################
def checkToken(proxy, token):

  try:
    proxyCheck = proxy.NoOperation(token)

    if proxyCheck['status'] == '200 OK':
      Log('Valid token.')
      return True
    else:
      Log('Invalid Token.')
      return False

  except:
    Log('Error occured when checking token.')
    return False

####################################################################################################
def quotaReached():

  if Dict['quotaReached'] > int(Datetime.TimestampFromDatetime(Datetime.Now())) - (24*60*60):

    Log('24 hour download quota has been reached')
    return True

  return False

####################################################################################################
def fetchSubtitles(proxy, token, part, imdbID=None, filename=None, season=None, episode=None):

  langList = list(set([Prefs['langPref1'], Prefs['langPref2'], Prefs['langPref3']]))
  if 'None' in langList:
    langList.remove('None')

  # Remove all subs from languages no longer set in the agent's prefs
  langListAlt = [Locale.Language.Match(l) for l in langList] # ISO 639-2 (from agent's prefs) --> ISO 639-1 (used to store subs in PMS)

  for l in part.subtitles:
    if l not in langListAlt:
      part.subtitles[l].validate_keys([])

  for l in langList:

    subtitleResponse = False

    if part.openSubtitleHash != '':

      Log('Looking for match for GUID %s and size %d' % (part.openSubtitleHash, part.size))
      try:
        subtitleResponse = proxy.SearchSubtitles(token,[{'sublanguageid':l, 'moviehash':part.openSubtitleHash, 'moviebytesize':str(part.size)}])['data']
        #Log('hash/size search result: ')
        #Log(subtitleResponse)
      except:
        subtitleResponse = False

    if not subtitleResponse and imdbID:  # Let's try the imdbID, if we have one

      Log('Found nothing via hash, trying search with imdbID: %s' % (imdbID))
      try:
        subtitleResponse = proxy.SearchSubtitles(token,[{'sublanguageid':l, 'imdbid':imdbID}])['data']
        #Log(subtitleResponse)
      except:
        subtitleResponse = False

    if not subtitleResponse and filename and season and episode: # TV

      Log('Found nothing via hash, trying search with filename/season/episode: %s, %s, %s' % (filename, season, episode))
      try:
        subtitleResponse = proxy.SearchSubtitles(token,[
          {'sublanguageid':l, 'season':season, 'episode':episode, 'tag':filename},
          {'sublanguageid':l, 'season':season, 'episode':episode, 'query':filename}
        ])['data']
        #Log(subtitleResponse)
      except:
        subtitleResponse = False

    if subtitleResponse:

      for st in subtitleResponse:  # Remove any subtitle formats we don't recognize

        if st['SubFormat'] not in SUBTITLE_EXT:
          Log('Removing a subtitle of type: %s' % (st['SubFormat']))
          subtitleResponse.remove(st)

      if len(subtitleResponse) == 0:
        Log('No valid subtitles. Skipping.')
        continue

      st = sorted(subtitleResponse, key=lambda k: int(k['SubDownloadsCnt']), reverse=True)  # Sort by 'most downloaded' subtitle file for current language

      filename = os.path.split(part.file)[1]
      lastScore = float(0.0)

      for sub in st:

        score = difflib.SequenceMatcher(None, sub['SubFileName'], filename).ratio()
        Log('Comparing "%s" vs. "%s" and it had the ratio: %f' % (sub['SubFileName'], filename, score))

        if score >= 0.6:

          if lastScore < score:
            Log('Choosing sub "%s" that scored %f' % (sub['SubFileName'], score))
            st = sub
            lastScore = score

        else:
          st = sorted(subtitleResponse, key=lambda k: int(k['SubDownloadsCnt']), reverse=True)[0]

      subUrl = st['SubDownloadLink'].rsplit('/sid-',1)[0]

      # Download subtitle only if it's not already present
      if subUrl not in part.subtitles[Locale.Language.Match(st['SubLanguageID'])]:

        try:
          subGz = HTTP.Request(st['SubDownloadLink'], headers={'Accept-Encoding':'gzip'})
          downloadQuota = int(subGz.headers['Download-Quota'])
        except Ex.HTTPError, e:
          if e.code == 407:
            Log('24 hour download quota has been reached')
            Dict['quotaReached'] = int(Datetime.TimestampFromDatetime(Datetime.Now()))
            return None
          else:
            Log('HTTP error: %d' % (e.code))
            continue
        except:
          try:
            errorMsg = "Sorry, maximum download count for IP"
            errorLocation = subGz.content.find(errorMsg)

            if errorLocation != -1:
              Log('Found \'%s\' in HTTP response. 24 hour download quota has been reached.' % (errorMsg))
              Dict['quotaReached'] = int(Datetime.TimestampFromDatetime(Datetime.Now()))
            else:
              Log('Error when retrieving subtitle. Skipping')

            return None

          except:
            Log('Error when retrieving subtitle. Skipping')
            continue

        if downloadQuota > 0:

          subData = Archive.GzipDecompress(subGz.content)
          part.subtitles[Locale.Language.Match(st['SubLanguageID'])][subUrl] = Proxy.Media(subData, ext=st['SubFormat'])
          Log('Download quota: %d' % (downloadQuota))

        else:
          Dict['quotaReached'] = int(Datetime.TimestampFromDatetime(Datetime.Now()))

      else:
        Log('Skipping, subtitle already downloaded (%s)' % (subUrl))

    else:
      Log('No subtitles available for language %s' % (l))

####################################################################################################
class OpenSubtitlesAgentMovies(Agent.Movies):

  name = 'OpenSubtitles.org'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  contributes_to = ['com.plexapp.agents.imdb']

  def search(self, results, media, lang):

    if quotaReached():
      return None

    results.Append(MetadataSearchResult(
      id    = media.primary_metadata.id.strip('t'),
      score = 100
    ))

  def update(self, metadata, media, lang):

    if quotaReached():
      return None

    (proxy, token) = opensubtitlesProxy()

    if token != '':
      for i in media.items:
        for part in i.parts:
          fetchSubtitles(proxy, token, part, imdbID=metadata.id)
    else: 
      Log('Unable to retrieve valid token. Skipping')

####################################################################################################
class OpenSubtitlesAgentTV(Agent.TV_Shows):

  name = 'OpenSubtitles.org'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  contributes_to = ['com.plexapp.agents.thetvdb']

  def search(self, results, media, lang):

    if quotaReached():
      return None

    results.Append(MetadataSearchResult(
      id    = 'null',
      score = 100
    ))

  def update(self, metadata, media, lang):

    if quotaReached():
      return None

    (proxy, token) = opensubtitlesProxy()

    if token != '':
      for s in media.seasons:
        # just like in the Local Media Agent, if we have a date-based season skip for now.
        if int(s) < 1900:
          for e in media.seasons[s].episodes:
            for i in media.seasons[s].episodes[e].items:
              for part in i.parts:
                filename = String.Unquote(part.file).split('/')[-1].split('\\')[-1]
                fetchSubtitles(proxy, token, part, filename=filename, season=str(s), episode=str(e))
    else:
      Log('Unable to retrieve valid token. Skipping')
