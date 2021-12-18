import re, time, unicodedata, hashlib, types
from collections import defaultdict

TVDB_API_KEY = 'D4DDDAEFAD083E6F'

META_HOST = 'https://meta.plex.tv'

# Plex Metadata endpoints
META_TVDB_GUID_SEARCH = '%s/tv/guid/' % META_HOST
META_TVDB_QUICK_SEARCH = '%s/tv/names/' % META_HOST
META_TVDB_TITLE_SEARCH = '%s/tv/titles/' % META_HOST

# TVDB V2 API
TVDB_BASE_URL = 'https://thetvdb.com'
TVDB_V2_PROXY_SITE = 'https://tvdb2.plex.tv'
TVDB_LOGIN_URL = '%s/login' % TVDB_V2_PROXY_SITE
TVDB_SEARCH_URL = '%s/search/series?name=%%s' % TVDB_V2_PROXY_SITE
TVDB_SERIES_URL = '%s/series/%%s?lang=%%s' % TVDB_V2_PROXY_SITE
TVDB_ACTORS_URL = '%s/series/%%s/actors' % TVDB_V2_PROXY_SITE
TVDB_SERIES_IMG_INFO_URL = '%s/series/%%s/images?lang=%%s' % TVDB_V2_PROXY_SITE
TVDB_SERIES_IMG_QUERY_URL = '%s/series/%%s/images/query?keyType=%%s&lang=%%s' % TVDB_V2_PROXY_SITE
TVDB_EPISODES_URL = '%s/series/%%s/episodes?page=%%s' % TVDB_V2_PROXY_SITE
TVDB_EPISODE_DETAILS_URL = '%s/episodes/%%s?lang=%%s' % TVDB_V2_PROXY_SITE
TVDB_IMG_ROOT = 'https://artworks.thetvdb.com/banners/%s'

GOOGLE_JSON_TVDB = 'https://ajax.googleapis.com/ajax/services/search/web?v=1.0&rsz=large&q=%s+"thetvdb.com"+series+%s'
GOOGLE_JSON_TVDB_TITLE = 'https://ajax.googleapis.com/ajax/services/search/web?v=1.0&rsz=large&q=%s+"thetvdb.com"+series+info+%s'
GOOGLE_JSON_BROAD = 'https://ajax.googleapis.com/ajax/services/search/web?v=1.0&rsz=large&q=%s+site:thetvdb.com+%s'
GOOGLE_JSON_IMDB = 'https://ajax.googleapis.com/ajax/services/search/web?v=1.0&rsz=large&q=%s+site:imdb.com+tv+%s'

SCRUB_FROM_TITLE_SEARCH_KEYWORDS = ['uk','us']
NETWORK_IN_TITLE = ['bbc']
EXTRACT_AS_KEYWORDS = ['uk','us','bbc']

# Extras
THETVDB_EXTRAS_URL = '%s/tv_e/%%s/%%s/%%s' % META_HOST
IVA_ASSET_URL = 'iva://api.internetvideoarchive.com/2.0/DataService/VideoAssets(%s)?lang=%s&bitrates=%s&duration=%s'
TYPE_ORDER = ['primary_trailer', 'trailer', 'behind_the_scenes', 'interview', 'scene_or_sample']
EXTRA_TYPE_MAP = {'primary_trailer' : TrailerObject,
                  'trailer' : TrailerObject,
                  'interview' : InterviewObject,
                  'behind_the_scenes' : BehindTheScenesObject,
                  'scene_or_sample' : SceneOrSampleObject}
IVA_LANGUAGES = {-1   : Locale.Language.Unknown,
                  0   : Locale.Language.English,
                  12  : Locale.Language.Swedish,
                  3   : Locale.Language.French,
                  2   : Locale.Language.Spanish,
                  32  : Locale.Language.Dutch,
                  10  : Locale.Language.German,
                  11  : Locale.Language.Italian,
                  9   : Locale.Language.Danish,
                  26  : Locale.Language.Arabic,
                  44  : Locale.Language.Catalan,
                  8   : Locale.Language.Chinese,
                  18  : Locale.Language.Czech,
                  80  : Locale.Language.Estonian,
                  33  : Locale.Language.Finnish,
                  5   : Locale.Language.Greek,
                  15  : Locale.Language.Hebrew,
                  36  : Locale.Language.Hindi,
                  29  : Locale.Language.Hungarian,
                  276 : Locale.Language.Indonesian,
                  7   : Locale.Language.Japanese,
                  13  : Locale.Language.Korean,
                  324 : Locale.Language.Latvian,
                  21  : Locale.Language.Norwegian,
                  24  : Locale.Language.Persian,
                  40  : Locale.Language.Polish,
                  17  : Locale.Language.Portuguese,
                  28  : Locale.Language.Romanian,
                  4   : Locale.Language.Russian,
                  105 : Locale.Language.Slovak,
                  25  : Locale.Language.Thai,
                  64  : Locale.Language.Turkish,
                  493 : Locale.Language.Ukrainian,
                  50  : Locale.Language.Vietnamese}

# Language table
# NOTE: if you add something here, make sure
# to add the language to the appropriate
# tvdb cache download script on the data
# processing servers
THETVDB_LANGUAGES_CODE = {
  'cs': '28',
  'da': '10',
  'de': '14',
  'el': '20',
  'en': '7',
  'es': '16',
  'fi': '11',
  'fr': '17',
  'he': '24',
  'hr': '31',
  'hu': '19',
  'it': '15',
  'ja': '25',
  'ko': '32',
  'nl': '13',
  'no': '9',
  'pl': '18',
  'pt': '26',
  'ru': '22',
  'sv': '8',
  'tr': '21',
  'zh': '27',
  'sl': '30'
}

ROMAN_NUMERAL_MAP = {
    ' i:': ' 1:',
    ' ii:': ' 2:',
    ' iii:': ' 3:',
    ' iv:': ' 4:',
    ' v:': ' 5:',
    ' vi:': ' 6:',
    ' vii:': ' 7:',
    ' viii:': ' 8:',
    ' ix:': ' 9:',
    ' x:': ' 10:',
    ' xi:': ' 11:',
    ' xii:': ' 12:',
}

GOOD_MATCH_THRESHOLD = 98 # Short circuit once we find a match better than this.
ACCEPTABLE_MATCH_THRESHOLD = 80

# UMP
UMP_BASE_URL = 'http://127.0.0.1:32400/services/ump/matches?%s'
UMP_MATCH_URL = 'type=2&title=%s&year=%s&lang=%s&manual=%s'

HEADERS = {'User-agent': 'Plex/Nine'}


def setJWT():

  try:
    jwtResp = JSON.ObjectFromString(HTTP.Request(TVDB_LOGIN_URL, data=JSON.StringFromObject(dict(apikey=TVDB_API_KEY)), headers={'Content-type': 'application/json'}, cacheTime=0).content)
  except Exception, e:
    Log("JWT Error: (%s) - %s" % (e, e.message))
    return

  if 'token' in jwtResp:
    HEADERS['Authorization'] = 'Bearer %s' % jwtResp['token']


def GetResultFromNetwork(url, fetchContent=True, additionalHeaders=None, data=None, cacheTime=CACHE_1WEEK):

    if additionalHeaders is None:
      additionalHeaders = dict()

    # Grab New Auth token
    if 'Authorization' not in HEADERS:
      setJWT()

    local_headers = HEADERS.copy()
    local_headers.update(additionalHeaders)

    try:
      result = HTTP.Request(url, headers=local_headers, timeout=60, data=data, cacheTime=cacheTime, immediate=fetchContent)
    except Ex.HTTPError, e:
      Log('HTTPError %s: %s' % (e.code, e.message))
      if (e.code == 401):
        Log('Problem with authentication, trying again...')
        try:
          setJWT()
          local_headers = HEADERS.copy()
          local_headers.update(additionalHeaders)
          result = HTTP.Request(url, headers=local_headers, timeout=60, data=data, cacheTime=cacheTime, immediate=fetchContent)
        except:
          return None
      else:
        return None
    except Exception, e:
      Log('Problem with the request: %s' % e.message)
      return None
    if fetchContent:
      try:
        result = result.content
      except Exception, e:
        Log('Content Error (%s) - %s' % (e, e.message))

    return result


def Start():
  HTTP.CacheTime = CACHE_1WEEK


def metadata_people(people_list, meta_people_obj):
  try:
    have_cleared = False

    if len(people_list) and 'sortOrder' in people_list[0]:
      people_list.sort(key=lambda x: x['sortOrder'])

    for person in people_list:

      if not have_cleared:
        meta_people_obj.clear()
        have_cleared = True

      if isinstance(person, basestring):
        for another_person in person.split('|'):
          if not len(another_person):
            continue
          else:
            new_person_obj = meta_people_obj.new()
            new_person_obj.name = another_person
      else:
        new_person_obj = meta_people_obj.new()
        new_person_obj.name = person.get('name', '')
        new_person_obj.role = person.get('role', '')
        new_person_obj.photo = TVDB_IMG_ROOT % person.get('image', '')

  except Exception, e:
    pass


class TVDBAgent(Agent.TV_Shows):

  name = 'TheTVDB'
  languages = [Locale.Language.English, 'fr', 'zh', 'sv', 'no', 'da', 'fi', 'nl', 'de', 'it', 'es', 'pl', 'hu', 'el', 'tr', 'ru', 'he', 'ja', 'pt', 'cs', 'ko', 'sl', 'hr']

  def dedupe(self, results):

    # make sure to keep the highest score for the id
    results.Sort('score', descending=True)

    toWhack = []
    resultMap = {}
    for result in results:
      if not resultMap.has_key(result.id):
        resultMap[result.id] = True
      else:
        toWhack.append(result)
    for dupe in toWhack:
      results.Remove(dupe)

  def searchByGuid(self, results, lang, title, year):

    # Compute the GUID
    guid = self.titleyear_guid(title,year)

    penalty = 0
    maxPercentPenalty = 30
    maxLevPenalty = 10
    minPercentThreshold = 25

    try:
      res = XML.ElementFromURL(META_TVDB_GUID_SEARCH + guid[0:2] + '/' + guid + '.xml')
      for match in res.xpath('//match'):
        guid = match.get('guid')
        count = int(match.get('count'))
        pct = int(match.get('percentage'))
        penalty += int(maxPercentPenalty * ((100-pct)/100.0))

        Log('Inspecting: guid = %s, count = %s, pct = %s' % (guid, count, pct))

        if pct > minPercentThreshold:
          try:
            series_data = JSON.ObjectFromString(GetResultFromNetwork(TVDB_SERIES_URL % (guid, lang), additionalHeaders={'Accept-Language': lang}))['data']
            name = series_data['seriesName']

            if '403: series not permitted' in name.lower():
              continue

            penalty += int(maxLevPenalty * (1 - lev_ratio(name, title)))
            try: year = series_data['firstAired'].split('-')[0]
            except: year = None
            Log('Adding (based on guid lookup) id: %s, name: %s, year: %s, lang: %s, score: %s' % (match.get('guid'), name, year, lang, 100 - penalty))
            results.Append(MetadataSearchResult(id=str(match.get('guid')), name=name, year=year, lang=lang, score=100 - penalty))
          except:
            continue

    except Exception, e:
      Log(repr(e))
      pass

  def searchByWords(self, results, lang, origTitle, year):
    # Process the text.
    title = origTitle.lower()
    title = re.sub(r'[\'":\-&,.!~()]', ' ', title)
    title = re.sub(r'[ ]+', ' ', title)

    # Search for words.
    show_map = {}
    total_words = 0

    for word in title.split():
      if word not in ['a', 'the', 'of', 'and']:
        total_words += 1
        wordHash = hashlib.sha1()
        wordHash.update(word.encode('utf-8'))
        wordHash = wordHash.hexdigest()
        try:
          matches = XML.ElementFromURL(META_TVDB_QUICK_SEARCH + lang + '/' + wordHash[0:2] + '/' + wordHash + '.xml', cacheTime=60)
          for match in matches.xpath('//match'):
            tvdb_id = match.get('id')
            title = match.get('title')
            titleYear = match.get('year')
            # Make sure we use the None type (not the string 'None' which evaluates to true and sorts differently).
            if titleYear == 'None':
              titleYear = None

            if not show_map.has_key(tvdb_id):
              show_map[tvdb_id] = [tvdb_id, title, titleYear, 1]
            else:
              show_map[tvdb_id] = [tvdb_id, title, titleYear, show_map[tvdb_id][3] + 1]
        except:
          pass

    resultList = show_map.values()
    resultList.sort(lambda x, y: cmp(y[3],x[3]))

    for i, result in enumerate(resultList):

      if i > 10:
        break

      score = 90 # Start word matches off at a slight defecit compared to guid matches.
      theYear = result[2]

      # Remove year suffixes that can mess things up.
      searchTitle = origTitle
      if len(origTitle) > 8:
        searchTitle = re.sub(r'([ ]+\(?[0-9]{4}\)?)', '', searchTitle)

      foundTitle = result[1]
      if len(foundTitle) > 8:
        foundTitle = re.sub(r'([ ]+\(?[0-9]{4}\)?)', '', foundTitle)

      # Remove prefixes that can screw things up.
      searchTitle = re.sub('^[Bb][Bb][Cc] ', '', searchTitle)
      foundTitle = re.sub('^[Bb][Bb][Cc] ', '', foundTitle)

      # Adjust if both have 'the' prefix by adding a prefix that won't be stripped.
      distTitle = searchTitle
      distFoundTitle = foundTitle
      if searchTitle.lower()[0:4] == 'the ' and foundTitle.lower()[0:4] == 'the ':
        distTitle = 'xxx' + searchTitle
        distFoundTitle = 'xxx' + foundTitle

      # Score adjustment for title distance.
      score = score - int(30 * (1 - lev_ratio(searchTitle, foundTitle)))

      # Discount for mismatched years.
      if theYear is not None and year is not None and theYear != year:
        score = score - 5

      # Discout for later results.
      score = score - i * 5

      # Use a relatively high threshold here to avoid pounding TheTVDB with a bunch of bogus stuff that 404's on our proxies.
      if score >= ACCEPTABLE_MATCH_THRESHOLD:

        # Make sure TheTVDB has heard of this show and we'll be able to parse the results.
        try:
          series_data = JSON.ObjectFromString(GetResultFromNetwork(TVDB_SERIES_URL % (result[0], lang), additionalHeaders={'Accept-Language': lang}))['data']
          Log('Adding (based on word matches) id: %s, name: %s, year: %s, lang: %s, score: %s' % (result[0],result[1],result[2],lang,score))
          results.Append(MetadataSearchResult(id=str(result[0]), name=result[1], year=result[2], lang=lang, score=score))
        except:
          Log('Skipping match with id %s: failed TVDB lookup.' % result[0])

    # Sort.
    results.Sort('score', descending=True)

  def exact_tvdb_match(self, mediaShowYear, media, results, lang='en'):
    Log('Searching for exact match with: %s (lang: %s)' % (mediaShowYear, lang))
    series_data = JSON.ObjectFromString(GetResultFromNetwork(TVDB_SEARCH_URL % mediaShowYear, additionalHeaders={'Accept-Language': lang}, cacheTime=0))['data']
    for i, series in enumerate(series_data):
      score = self.ParseSeries(media, series, lang, results, 80)
      if i > 4:
        break
    return max([r.score for r in results] or [0])

  def exact_tvdb_match_with_fallback(self, mediaShowYear, media, results, lang):
    score = 0
    try:
      score = self.exact_tvdb_match(mediaShowYear, media, results, lang)
    except Exception:
      Log('There was a problem attempting an exact TVDB match in %s (lang: %s)' % (mediaShowYear, lang))

    if lang != 'en' and score < ACCEPTABLE_MATCH_THRESHOLD:
      try:
        score = self.exact_tvdb_match(mediaShowYear, media, results)
      except Exception:
        Log('There was a problem attempting an exact TVDB match in %s (lang: en)' % mediaShowYear)

  def perform_ump_tv_search(self, results, media, lang, manual):
    ump_match_uri = UMP_MATCH_URL % (String.Quote(media.show), media.year if media.year else '', lang, 1 if manual else 0)
    ump_movie = XML.ElementFromURL(UMP_BASE_URL % ump_match_uri, cacheTime=CACHE_1DAY)

    for video in ump_movie.xpath('//Directory'):

      try:
        video_id = video.get('ratingKey')[video.get('ratingKey').rfind('/') + 1:]
        score = int(video.get('score'))
      except Exception, e:
        continue

      # Deal with year
      year = None
      try: year = int(video.get('year'))
      except: pass

      result = MetadataSearchResult(id=video_id, name=video.get('title'), year=year, lang=lang, thumb=video.get('thumb'), score=score)
      Log("UMP: %s" % repr(result))
      results.Append(result)

  def search(self, results, media, lang, manual=False):

    if media.primary_agent == 'com.plexapp.agents.themoviedb':

      # Get the TVDB id from the Movie Database Agent
      tvdb_id = Core.messaging.call_external_function(
        'com.plexapp.agents.themoviedb',
        'MessageKit:GetTvdbId',
        kwargs = dict(
          tmdb_id = media.primary_metadata.id
        )
      )

      if tvdb_id:
        results.Append(MetadataSearchResult(
          id = str(tvdb_id),
          score = 100
        ))

      return

    # MAKE SURE WE USE precomposed form, since that seems to be what TVDB prefers.
    media.show = unicodedata.normalize('NFC', unicode(media.show)).strip()

    # If we got passed in something that looks like an ID, use it.
    if len(media.show) > 3 and re.match('^[0-9]+$', media.show) is not None:
      url = TVDB_BASE_URL + '?tab=series&id=' + media.show
      self.TVDBurlParse(media, lang, results, 100, 0, url)

    # GUID-based matches.
    self.searchByGuid(results, lang, media.show, media.year)
    results.Sort('score', descending=True)

    for i,r in enumerate(results):
      if i > 2:
        break
      Log('Top GUID result: ' + str(results[i]))

    if not len(results) or results[0].score <= GOOD_MATCH_THRESHOLD or manual:
      # No good-enough matches in GUID search, try word matches.
      self.searchByWords(results, lang, media.show, media.year)
      self.dedupe(results)
      results.Sort('score', descending=True)

      for i,r in enumerate(results):
        if i > 2:
          break
        Log('Top GUID+name result: ' + str(results[i]))

    if not len(results) or results[0].score <= GOOD_MATCH_THRESHOLD or manual:
      mediaYear = ''
      if media.year is not None:
        mediaYear = ' (' + media.year + ')'
      w = media.show.lower().split(' ')
      keywords = ''
      for k in EXTRACT_AS_KEYWORDS:
        if k.lower() in w:
          keywords = keywords + k + '+'
      cleanShow = self.util_clean_show(media.show, SCRUB_FROM_TITLE_SEARCH_KEYWORDS)
      cs = cleanShow.split(' ')
      cleanShow = ''
      for x in cs:
        cleanShow = cleanShow + 'intitle:' + x + ' '

      cleanShow = cleanShow.strip()
      origShow = media.show
      SVmediaShowYear = {'normal': String.Quote((origShow + mediaYear).encode('utf-8'), usePlus=True).replace('intitle%3A', 'intitle:'),
                         'clean': String.Quote((cleanShow + mediaYear).encode('utf-8'), usePlus=True).replace('intitle%3A','intitle:'),
                         'normalNoYear': String.Quote(origShow.encode('utf-8'), usePlus=True).replace('intitle%3A', 'intitle:')}

      #try an exact tvdb match
      self.exact_tvdb_match_with_fallback(SVmediaShowYear['normal'], media, results, lang)

      if SVmediaShowYear['normal'] != SVmediaShowYear['normalNoYear']:
        self.exact_tvdb_match_with_fallback(SVmediaShowYear['normalNoYear'], media, results, lang)

    self.dedupe(results)

    #hunt for duplicate shows with different years
    resultMap = {}
    for result in results:
      for check in results:
        if result.name == check.name and result.id != check.id:
          resultMap[result.year] = result

    years = resultMap.keys()
    years.sort(reverse=True)

    # bump the score of newer dupes
    i=0
    for y in years[:-1]:
      if resultMap[y].score == resultMap[years[i]].score:
        resultMap[y].score = resultMap[y].score + 1

    for i,r in enumerate(results):
      if i > 10:
        break
      Log('Final result: ' + str(results[i]))

  def TVDBurlParse(self, media, lang, results, score, scorePenalty, url):
    if url.count('tab=series&id='):
      seriesRx = 'tab=series&id=([0-9]+)'
      m = re.search(seriesRx, url)
    elif url.count('id=') and url.count('tab=series'):
      seriesRx = 'id=([0-9]+)&tab=series'
      m = re.search(seriesRx, url)
    elif url.count('tab=seasonall&id='):
      seriesRx = 'tab=seasonall&id=([0-9]+)'
      m = re.search(seriesRx, url)
    elif url.count('tab=seasonall') and url.count('id='):
      seriesRx = 'id=([0-9]+)&tab=seasonall'
      m = re.search(seriesRx, url)
    else:
      seriesRx = 'seriesid=([0-9]+)'
      m = re.search(seriesRx, url)
    if m:
      try:
        series_data = JSON.ObjectFromString(GetResultFromNetwork(TVDB_SERIES_URL % (m.groups(1)[0], lang), additionalHeaders={'Accept-Language': lang}))['data']
        if len(series_data):
          self.ParseSeries(media, series_data, lang, results, score - scorePenalty)
      except Exception, e:
        Log('Couldn\'t find Series in TVDB XML: ' + str(e))

  def ParseSeries(self, media, series_data, lang, results, score):

    # Get attributes from the JSON
    series_id = series_data.get('id', '')
    series_name = series_data.get('seriesName', '')
    series_lang = lang
    Log('Scoring result: %s (%s)' % (series_name, series_id))
    
    if series_name.lower().strip() == media.show.lower().strip():
      score += 10
    elif series_name[:series_name.rfind('(')].lower().strip() == media.show.lower().strip():
      score += 6

    if series_name is '' or '403: series not permitted' in series_name.lower():
      return 0

    try:
      series_year = series_data['firstAired'][:4]
    except:
      series_year = None

    if not series_name:
      return 0

    if not media.year:
      clean_series_name = series_name.replace('(' + str(series_year) + ')','').strip().lower()
    else:
      clean_series_name = series_name.lower()

    cleanShow = self.util_clean_show(media.show, NETWORK_IN_TITLE)

    substringLen = len(Util.LongestCommonSubstring(cleanShow.lower(), clean_series_name))
    cleanShowLen = len(cleanShow)

    maxSubstringPoints = 5.0  # use a float
    score += int((maxSubstringPoints * substringLen)/cleanShowLen)  # max 15 for best substring match

    distanceFactor = .6
    score = score - int(distanceFactor * Util.LevenshteinDistance(cleanShow.lower(), clean_series_name))

    if series_year and media.year:
      if media.year == series_year:
        score += 10
      else:
        score = score - 10

    # sanity check to make sure we have SOME common substring
    if (float(substringLen) / cleanShowLen) < .15:  # if we don't have at least 15% in common, then penalize below the 80 point threshold
      score = score - 25

    Log("Score after heuristics: %s" % score)

    # Add a result for this show
    results.Append(
      MetadataSearchResult(
          id=str(series_id),
          name=series_name,
          year=series_year,
          lang=series_lang,
          score=score
      )
    )

    return score

  def eligibleForExtras(self):
    # Extras.
    try:
      # Do a quick check to make sure we've got the types available in this framework version, and that the server
      # is new enough to support the IVA endpoints.
      t = InterviewObject()
      if Util.VersionAtLeast(Platform.ServerVersion, 0,9,9,13):
        find_extras = True
      else:
        find_extras = False
        Log('Not adding extras: Server v0.9.9.13+ required')
    except Exception, e:
      Log('Not adding extras: Framework v2.5.0+ required')
      find_extras = False
    return find_extras

  """
  Lovingly borrowed from https://stackoverflow.com/questions/794663/net-convert-number-to-string-representation-1-to-one-2-to-two-etc
  As instructed by IVA's Normalization Rules, Step 17: http://www.internetvideoarchive.com/documentation/data-integration/iva-data-matching-guidelines/
  """
  def number_to_text(self, n):
    if n < 0:
      return "Minus " + self.number_to_text(-n)
    elif n == 0:
      return ""
    elif n <= 19:
      return ("One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen", "Seventeen", "Eighteen", "Nineteen")[n-1] + " "
    elif n <= 99:
      return ("Twenty", "Thirty", "Forty", "Fifty", "Sixty", "Seventy", "Eighty", "Ninety")[n / 10 - 2] + " " + self.number_to_text(n % 10)
    elif n <= 199:
      return "One Hundred " + self.number_to_text(n % 100)
    elif n <= 999:
      return self.number_to_text(n / 100) + "Hundred " + self.number_to_text(n % 100)
    elif n <= 1999:
      return "One Thousand " + self.number_to_text(n % 1000);
    elif n <= 999999:
      return self.number_to_text(n / 1000) + "Thousand " + self.number_to_text(n % 1000)
    elif n <= 1999999:
      return "One Million " + self.number_to_text(n % 1000000)
    elif n <= 999999999:
      return self.number_to_text(n / 1000000) + "Million " + self.number_to_text(n % 1000000)
    elif n <= 1999999999:
      return "One Billion " + self.number_to_text(n % 1000000000);
    else:
      return self.number_to_text(n / 1000000000) + "Billion " + self.number_to_text(n % 1000000000)

  # IVA Normalization rules found here: http://www.internetvideoarchive.com/documentation/data-integration/iva-data-matching-guidelines/
  def ivaNormalizeTitle(self, title):
    if not isinstance(title, basestring):
      return ""

    title = title.strip().upper()

    title = re.sub(r'^(AN |A |THE )|(, AN |, A |, THE)$|\([^\)]+\)$|\{[^\}]+\}$|\[[^\]]+\]$| AN IMAX 3D EXPERIENCE| AN IMAX EXPERIENCE| THE IMAX EXPERIENCE| IMAX 3D EXPERIENCE| IMAX 3D', "", title)

    title = title.lower().replace('&', 'and').strip().upper()

    title = re.sub(r'^(AN |A |THE )|(, AN |, A |, THE)$', "", title)

    title = title.lower()

    title = re.sub(r'( i:| ii:| iii:| iv:| v:| vi:| vii:| viii:| ix:| x:| xi:| xii:)', lambda m: ROMAN_NUMERAL_MAP[m.group(0)], title)

    title = re.sub(r'[!@#\$%\^\*\_\+=\{\}\[\]\|<>`\:\-\(\)\?/\\\&\~\.\,\'\"]', " ", title)

    title = re.sub(r'\b\d+\b', lambda m: self.number_to_text(int(m.group())).replace('-', ' '), title)

    title = title.lower().strip().replace(',', ' ')

    title = re.sub(r'( i$| ii$| iii$| iv$| v$| vi$| vii$| viii$| ix$| x$| xi$| xii$)', lambda m: ROMAN_NUMERAL_MAP[m.group(0)+":"][:-1], title)

    title = re.sub(r'\b\d+\b', lambda m: self.number_to_text(int(m.group())).replace('-', ' '), title)

    title = title.lower()

    normalized = unicodedata.normalize('NFKD', title)
    corrected = ''
    for i in range(len(normalized)):
      if not unicodedata.combining(normalized[i]):
        corrected += normalized[i]
    title = corrected

    return title.encode('utf-8').strip().replace("  ", " ")

  def getSeriesImages(self, metadata, lang='en'):
    # Get Image Counts
    image_types = {}
    try:
      image_types = JSON.ObjectFromString(GetResultFromNetwork(TVDB_SERIES_IMG_INFO_URL % (metadata.id, lang), additionalHeaders={'Accept-Language': lang}))['data']
    except:
      Log("Bad image type data for TVDB id: %s" % metadata.id)

    img_list = []
    for image_type, num_imgs in image_types.iteritems():
      try:
        img_list.extend(JSON.ObjectFromString(GetResultFromNetwork(TVDB_SERIES_IMG_QUERY_URL % (metadata.id, image_type, lang), additionalHeaders={'Accept-Language': lang}))['data'])
      except:
        Log("Bad image type query data for TVDB id: %s (image_type: %s)" % (metadata.id, image_type))

    return sorted(img_list, key=lambda img: img.get('ratingsInfo', dict()).get('average', -100), reverse=True)

  def processExtras(self, xml, metadata, lang, ivaNormTitle=""):

    # Bail if we don't have an XML.
    if not xml:
      return

    extras = []
    media_title = None
    for extra in xml.xpath('./extra'):
      avail = Datetime.ParseDate(extra.get('originally_available_at'))
      lang_code = int(extra.get('lang_code')) if extra.get('lang_code') else -1
      subtitle_lang_code = int(extra.get('subtitle_lang_code')) if extra.get('subtitle_lang_code') else -1

      spoken_lang = IVA_LANGUAGES.get(lang_code) or Locale.Language.Unknown
      subtitle_lang = IVA_LANGUAGES.get(subtitle_lang_code) or Locale.Language.Unknown
      include = False

      # Include extras in section language...
      if spoken_lang == lang:

        # ...if they have section language subs AND this was explicitly requested in prefs.
        if Prefs['native_subs'] and subtitle_lang == lang:
          include = True

        # ...if there are no subs.
        if subtitle_lang_code == -1:
          include = True

      # Include foreign language extras if they have subs in the section language.
      if spoken_lang != lang and subtitle_lang == lang:
        include = True

      # Always include English language extras anyway (often section lang options are not available), but only if they have no subs.
      if spoken_lang == Locale.Language.English and subtitle_lang_code == -1:
        include = True

      # Exclude non-primary trailers and scenes.
      extra_type = 'primary_trailer' if extra.get('primary') == 'true' else extra.get('type')

      if include:

        bitrates = extra.get('bitrates') or ''
        duration = int(extra.get('duration') or 0)

        # Remember the title if this is the primary trailer.
        if extra_type == 'primary_trailer':
          media_title = extra.get('title')

        # Add the extra.
        if extra_type in EXTRA_TYPE_MAP:
          extras.append({ 'type' : extra_type,
                          'lang' : spoken_lang,
                          'extra' : EXTRA_TYPE_MAP[extra_type](url=IVA_ASSET_URL % (extra.get('iva_id'), spoken_lang, bitrates, duration),
                                                               title=extra.get('title'),
                                                               year=avail.year,
                                                               originally_available_at=avail,
                                                               thumb=extra.get('thumb') or '')})
        else:
          Log('Skipping extra %s because type %s was not recognized.' % (extra.get('iva_id'), extra_type))

    # Sort the extras, making sure the primary trailer is first.
    extras.sort(key=lambda e: TYPE_ORDER.index(e['type']))

    # If our primary trailer is in English but the library language is something else, see if we can do better.
    if lang != Locale.Language.English and extras and extras[0]['lang'] == Locale.Language.English:
      lang_matches = [t for t in xml.xpath('//extra') if t.get('type') == 'trailer' and IVA_LANGUAGES.get(int(t.get('subtitle_lang_code') or -1)) == lang]
      lang_matches += [t for t in xml.xpath('//extra') if t.get('type') == 'trailer' and IVA_LANGUAGES.get(int(t.get('lang_code') or -1)) == lang]
      if len(lang_matches) > 0:
        extra = lang_matches[0]
        spoken_lang = IVA_LANGUAGES.get(int(extra.get('lang_code') or -1)) or Locale.Language.Unknown
        extras[0]['lang'] = spoken_lang
        extras[0]['extra'].url = IVA_ASSET_URL % (extra.get('iva_id'), spoken_lang, extra.get('bitrates') or '', int(extra.get('duration') or 0))
        extras[0]['extra'].thumb = extra.get('thumb') or ''
        Log('Adding trailer with spoken language %s and subtitled langauge %s to match library language.' % (spoken_lang, IVA_LANGUAGES.get(int(extra.get('subtitle_lang_code') or -1)) or Locale.Language.Unknown))

    # Clean up the found extras.
    extras = [scrub_extra(extra, media_title) for extra in extras]

    # Add them in the right order to the metadata.extras list.
    for extra in extras:
      metadata.extras.add(extra['extra'])

    Log('%s - Added %d of %d extras.' % (ivaNormTitle, len(extras), len(xml.xpath('./extra'))))

  def update(self, metadata, media, lang, force=False):
    Log("def update()")

    tvdb_series_data = defaultdict(lambda: '')
    tvdb_series_orig_data = dict()
    try:
      tvdb_series_orig_data = JSON.ObjectFromString(GetResultFromNetwork(TVDB_SERIES_URL % (metadata.id, lang), additionalHeaders={'Accept-Language': lang}, cacheTime=0 if force else CACHE_1WEEK))
      tvdb_series_data.update(tvdb_series_orig_data['data'])
    except:
      Log("Bad series data, no update for TVDB id: %s (lang: %s)" % (metadata.id, lang))
      # TODO: Add function to search TMDb by name
      pass
      
    # Find TheMovieDB match.
    try:
      TMDB_BASE_URL = 'http://127.0.0.1:32400/services/tmdb?uri=%s'
      url = '/find/' + metadata.id + '?external_source=tvdb_id'
      tmdb_dict = JSON.ObjectFromURL(TMDB_BASE_URL % String.Quote(url, True), sleep=2.0, headers={'Accept': 'application/json'}, cacheTime=0 if force else CACHE_1MONTH)
      tmdb_id = tmdb_dict['tv_results'][0]['id']
      
      url = '/tv/%s/recommendations' % tmdb_id
      tmdb_dict = JSON.ObjectFromURL(TMDB_BASE_URL % String.Quote(url, True), sleep=2.0, headers={'Accept': 'application/json'}, cacheTime=0 if force else CACHE_1MONTH)

      metadata.similar.clear()
      for rec in tmdb_dict['results']:
        metadata.similar.add(rec['name'])
    except:
      pass

    # English Fallback
    tvdb_english_series_data = defaultdict(lambda: '')
    try:
      if tvdb_series_orig_data['errors']['invalidLanguage'] and lang != 'en':
        tvdb_english_series_data.update(JSON.ObjectFromString(
          GetResultFromNetwork(TVDB_SERIES_URL % (metadata.id, 'en'),
            additionalHeaders={'Accept-Language': 'en'},
            cacheTime=0 if force else CACHE_1WEEK)
          )['data']
        )
    except KeyError, e:
      # Means there were no 'errors' so just move along
      pass
    except Exception, e:
      Log("Bad English series data, no update for TVDB id: %s" % metadata.id)
      # TODO: Add function to search TMDb by name
      return

    actor_data = None
    try:
      actor_data = JSON.ObjectFromString(GetResultFromNetwork(TVDB_ACTORS_URL % metadata.id, cacheTime=0 if force else CACHE_1WEEK))['data']
    except Exception, e:
      Log("Bad actor data, no update for TVDB id: %s" % metadata.id)

    metadata.title = tvdb_series_data['seriesName'] or tvdb_english_series_data['seriesName']
    metadata.summary = tvdb_series_data['overview'] or tvdb_english_series_data['overview']
    metadata.content_rating = tvdb_series_data['rating']
    metadata.studio = tvdb_series_data['network']

    # Convenience Function
    parse_date = lambda s: Datetime.ParseDate(s).date()

    try:
      originally_available_at = tvdb_series_data['firstAired']
      if len(originally_available_at) > 0:
        metadata.originally_available_at = parse_date(originally_available_at)
      else:
        metadata.originally_available_at = None
    except: pass

    series_extra_xml = None
    ivaNormTitle = ''
    if metadata.title is not None and metadata.title is not '' and metadata.id is not None and metadata.id is not '' and self.eligibleForExtras() and Prefs['extras']:
      ivaNormTitle = self.ivaNormalizeTitle(metadata.title)
      if len(ivaNormTitle) > 0:
        try:
          req = THETVDB_EXTRAS_URL % (metadata.id, ivaNormTitle.replace(' ', '+'), -1 if metadata.originally_available_at is None else metadata.originally_available_at.year)
          series_extra_xml = XML.ElementFromURL(req, cacheTime=0 if force else CACHE_1WEEK)

          self.processExtras(series_extra_xml, metadata, lang, ivaNormTitle)

        except Ex.HTTPError, e:
          if e.code == 403:
            Log('Skipping online extra lookup (an active Plex Pass is required).')
        except:
          Log('Skipping online extra lookup.')

    try: tvdb_runtime = int(tvdb_series_data['runtime']) * 60 * 1000
    except (ValueError, KeyError): tvdb_runtime = None

    metadata.duration = tvdb_runtime

    try: tvdb_rating = float(tvdb_series_data['siteRating'])
    except: tvdb_rating = None

    metadata.rating = tvdb_rating
    metadata.genres = tvdb_series_data.get('genre', [])

    # Cast
    metadata_people(actor_data, metadata.roles)

    # Create List of episodes
    episode_data = []
    next_page = 1
    try:
      while isinstance(next_page, int) or (isinstance(next_page, basestring) and next_page.isdigit()):
        next_page = int(next_page)
        episode_data_page = JSON.ObjectFromString(GetResultFromNetwork(TVDB_EPISODES_URL % (metadata.id, next_page), cacheTime=0 if force else CACHE_1HOUR * 24))
        episode_data.extend(episode_data_page['data'])
        next_page = episode_data_page['links']['next']
    except:
      pass

    # Get episode data
    @parallelize
    def UpdateEpisodes():

      ordering = media.settings.get('showOrdering', 'aired') if media.settings else 'aired'

      # This can end up empty ('') in "downgraded" libraries.
      if not ordering:
        ordering = 'aired'

      use_dvd_order = ordering == 'dvd'
      use_absolute_order = ordering == 'absolute'
      Log('Show ordering is: %s', ordering)

      for episode_info in episode_data:

        # Get the season and episode numbers
        season_num = str(episode_info.get('dvdSeason' if use_dvd_order else 'airedSeason', ''))
        episode_num = str(episode_info.get('dvdEpisodeNumber' if use_dvd_order else 'airedEpisodeNumber', ''))
        if use_absolute_order:
          absolute_num = str(episode_info.get('absoluteNumber', ''))
          if absolute_num:
            if ('1' in media.seasons) and (absolute_num in media.seasons['1'].episodes):
              episode_num = absolute_num
              season_num = '1'
            elif (season_num in media.seasons) and (absolute_num in media.seasons[season_num].episodes):
              episode_num = absolute_num

        if media is not None:
          # Also get the air date for date-based episodes.
          try:
            originally_available_at = parse_date(episode_info['firstAired'])
            date_based_season = originally_available_at.year
          except:
            originally_available_at = date_based_season = None

          if not ((season_num in media.seasons and episode_num in media.seasons[season_num].episodes) or
                  (use_absolute_order and '1' in media.seasons and episode_num in media.seasons['1'].episodes) or
                  (originally_available_at is not None and date_based_season in media.seasons and originally_available_at in media.seasons[date_based_season].episodes) or
                  (originally_available_at is not None and season_num in media.seasons and originally_available_at in media.seasons[season_num].episodes)):
            Log("No media for season %s episode %s - skipping population of episode data", season_num, episode_num)
            continue

        # Get the episode object from the model
        episode = metadata.seasons[season_num].episodes[episode_num]

        # Create a task for updating this episode
        @task
        def UpdateEpisode(episode=episode, episode_info=episode_info, lang=lang, series_available=metadata.originally_available_at, series_id=metadata.id, ivaNormTitle=ivaNormTitle, series_extra_xml=series_extra_xml):

          episode_id = str(episode_info['id'])

          tvdb_episode_details = defaultdict(lambda: '')
          tvdb_episode_orig_details = dict()
          try:
            tvdb_episode_orig_details = JSON.ObjectFromString(GetResultFromNetwork(TVDB_EPISODE_DETAILS_URL % (episode_id, lang), additionalHeaders={'Accept-Language': lang}, cacheTime=0 if force else CACHE_1WEEK))
            tvdb_episode_details.update(tvdb_episode_orig_details['data'])
          except:
            Log("Bad episode data, no update for TVDB id: %s (lang: %s)" % (episode_id, lang))
            # TODO: Add function to search TMDb by name
            pass

          # English Fallback
          tvdb_english_episode_details = defaultdict(lambda: '')
          try:
            if tvdb_episode_orig_details['errors']['invalidLanguage'] and lang != 'en':
              tvdb_english_episode_details.update(JSON.ObjectFromString(
                GetResultFromNetwork(TVDB_EPISODE_DETAILS_URL % (episode_id, 'en'),
                  additionalHeaders={'Accept-Language': 'en'},
                  cacheTime=0 if force else CACHE_1WEEK)
                )['data']
              )
          except KeyError, e:
            # Means there were no 'errors' so just move along
            pass
          except:
            Log("Bad English episode data, no update for TVDB id: %s" % episode_id)
            # TODO: Add function to search TMDb by name
            return

          # Need to reassign these because of parallel tasks
          season_num = tvdb_episode_details['airedSeason'] or tvdb_english_episode_details['airedSeason']
          episode_num = tvdb_episode_details['airedEpisodeNumber'] or tvdb_english_episode_details['airedEpisodeNumber']

          # Get episode information
          episode.title = tvdb_episode_details['episodeName'] or tvdb_english_episode_details['episodeName']
          episode.summary = tvdb_episode_details['overview'] or tvdb_english_episode_details['overview']

          try: episode.absolute_number = int(tvdb_episode_details['absoluteNumber'])
          except: pass

          try: tvdb_rating = float(tvdb_episode_details['siteRating'])
          except: tvdb_rating = None
          episode.rating = tvdb_rating

          try:
            originally_available_at = tvdb_episode_details['firstAired']
            if originally_available_at is not None and len(originally_available_at) > 0:
              episode.originally_available_at = parse_date(originally_available_at)
          except:
            pass

          metadata_people([tvdb_episode_details.get('director', '')], episode.directors)
          metadata_people(tvdb_episode_details.get('writers', []), episode.writers)

          # Download the episode thumbnail
          valid_names = list()

          if not len(valid_names) and tvdb_episode_details.get('filename'):
            thumb_file = tvdb_episode_details.get('filename')
            if thumb_file is not None and len(thumb_file) > 0:
              thumb_url = TVDB_IMG_ROOT % thumb_file
              thumb_data = GetResultFromNetwork(thumb_url, False, cacheTime=0 if force else CACHE_1WEEK)

              # Check that the thumb doesn't already exist before downloading it
              valid_names.append(thumb_url)
              if thumb_url not in episode.thumbs:
                try:
                  episode.thumbs[thumb_url] = Proxy.Preview(thumb_data)
                except:
                  # tvdb doesn't have a thumb for this show
                  pass

          episode.thumbs.validate_keys(valid_names)

          if Prefs['extras']:
            try:
              episode_extra_xml = series_extra_xml.xpath('./related_extras/season_%s/related_extras/episode_%s' % (season_num, episode_num))
              if len(episode_extra_xml):
                self.processExtras(episode_extra_xml[0], episode, lang, ivaNormTitle)

            except Ex.HTTPError, e:
              if e.code == 403:
                Log('Skipping online extra lookup (an active Plex Pass is required).')

            except AttributeError:
              Log("Season Extra XML is empty - therefore, no episode XML")

            except Exception, e:
              Log('An error occurred while grabbing individual episode TV extras (%s) - %s' % (e, e.message))

    # Maintain a list of valid image names
    valid_names = list()

    img_list = self.getSeriesImages(metadata, lang)

    if lang != 'en':
      en_img_list = self.getSeriesImages(metadata, 'en')
      img_list.extend(en_img_list)

    @parallelize
    def DownloadImages():

      # Add a download task for each image
      i = 0
      for img_info in img_list:
        i += 1
        @task
        def DownloadImage(metadata=metadata, img_info=img_info, i=i, valid_names=valid_names):

          # Parse the banner.
          banner_type, banner_path, banner_thumb, proxy = self.parse_banner(img_info)

          # Compute the banner name and prepare the data
          banner_name = TVDB_IMG_ROOT % banner_path
          banner_url = TVDB_IMG_ROOT % banner_thumb

          valid_names.append(banner_name)

          # Find the attribute to add to based on the image type, checking that data doesn't
          # already exist before downloading
          if banner_type == 'fanart' and banner_name not in metadata.art:
            try: metadata.art[banner_name] = proxy(self.banner_data(banner_url), sort_order=i)
            except Exception, e: Log(str(e))

          elif banner_type == 'poster' and banner_name not in metadata.posters:
            try: metadata.posters[banner_name] = proxy(self.banner_data(banner_url), sort_order=i)
            except Exception, e: Log(str(e))

          elif banner_type == 'series' and banner_name not in metadata.banners:
            try: metadata.banners[banner_name] = proxy(self.banner_data(banner_url), sort_order=i)
            except Exception, e: Log(str(e))

          elif banner_type == 'season':
            season_num = str(img_info.get('subKey', ''))

            # Need to check for date-based season (year) as well.
            try: date_based_season = str(int(season_num) + metadata.originally_available_at.year - 1)
            except: date_based_season = None

            if media is None or season_num in media.seasons or date_based_season in media.seasons:
              if banner_name not in metadata.seasons[season_num].posters:
                try: metadata.seasons[season_num].posters[banner_name] = proxy(self.banner_data(banner_url), sort_order=i)
                except Exception, e: Log(str(e))

    # Fallback to foreign art if localized art doesn't exist.
    if len(metadata.art) == 0 and lang == 'en':
      i = 0
      for img_info in img_list:
        banner_type, banner_path, banner_thumb, proxy = self.parse_banner(img_info)
        banner_name = TVDB_IMG_ROOT % banner_path
        if banner_type == 'fanart' and banner_name not in metadata.art:
          try: metadata.art[banner_name] = proxy(self.banner_data(TVDB_IMG_ROOT % banner_thumb), sort_order=i)
          except: pass
        i += 1

    # Check each poster, background & banner image we currently have saved. If any of the names are no longer valid, remove the image
    metadata.posters.validate_keys(valid_names)
    metadata.art.validate_keys(valid_names)
    metadata.banners.validate_keys(valid_names)

    # Grab season level extras
    if Prefs['extras']:
      for season_num in metadata.seasons:
        try:
          season_extra_xml = series_extra_xml.xpath('./related_extras/season_%s' % season_num)
          if len(season_extra_xml):
                self.processExtras(season_extra_xml[0], metadata.seasons[season_num], lang, ivaNormTitle)
          elif len(ivaNormTitle) > 0 and metadata.id is not None and metadata.id is not "":
            req = THETVDB_EXTRAS_URL % (metadata.id, ivaNormTitle.replace(' ', '+'), -1 if metadata.originally_available_at is None else metadata.originally_available_at.year)
            req = req + '/' + str(season_num)
            xml = XML.ElementFromURL(req, cacheTime=0 if force else CACHE_1WEEK)
            self.processExtras(xml, metadata.seasons[season_num], lang, ivaNormTitle)

        except Ex.HTTPError, e:
          if e.code == 403:
            Log('Skipping online extra lookup (an active Plex Pass is required).')

        except AttributeError:
          Log("Series Extra XML is empty - therefore, no season XML")

        except Exception, e:
          Log('An error occurred while grabbing TV season extras (%s) - %s' % (e, e.message))

  def parse_banner(self, img_info):
    # Get the image attributes from the XML
    banner_type = img_info.get('keyType', '')
    banner_path = img_info.get('fileName', '')
    # If we are missing a value for the thumbnail attribute, fallback to the '_cache' thumbnail
    banner_thumb = img_info.get('thumbnail') or '_cache/%s' % banner_path
    proxy = Proxy.Preview

    return banner_type, banner_path, banner_thumb, proxy

  def banner_data(self, path):
    return GetResultFromNetwork(path, False)

  def util_clean_show(self, clean_show, scrub_list):
    for word in scrub_list:
      c = word.lower()
      l = clean_show.lower().find('(' + c + ')')
      if l >= 0:
        clean_show = clean_show[:l] + clean_show[l+len(c)+2:]
      l = clean_show.lower().find(' ' + c)
      if l >= 0:
        clean_show = clean_show[:l] + clean_show[l+len(c)+1:]
      l = clean_show.lower().find(c + ' ')
      if l >= 0:
        clean_show = clean_show[:l] + clean_show[l+len(c)+1:]
    return clean_show

  def identifierize(self, string):
    string = re.sub( r"\s+", " ", string.strip())
    string = unicodedata.normalize('NFKD', safe_unicode(string))
    string = re.sub(r"['\"!?@#$&%^*\(\)_+\.,;:/]","", string)
    string = re.sub(r"[_ ]+","_", string)
    string = string.strip('_')
    return string.strip().lower()

  def guidize(self, string):
    hash = hashlib.sha1()
    hash.update(string.encode('utf-8'))
    return hash.hexdigest()

  def titleyear_guid(self, title, year=None):
    if title is None:
      title = ''

    if year == '' or year is None or not year:
      string = u"%s" % self.identifierize(title)
    else:
      string = u"%s_%s" % (self.identifierize(title), year)
    return self.guidize(string)

  def best_title_by_language(self, lang, localTitle, tvdbID ):

    # this returns not only the best title, but the best
    # levenshtien ratio found amongst all of the titles
    # in the title list... the lev ratio is to give an overall
    # confidence that the local title corresponds to the
    # tvdb id.. even if the picked title is in a language
    # other than the locally named title

    titles = {'best_lev_ratio': {'title': None, 'lev_ratio': -1.0}}   # -1 to force > check later
    try:
      res = XML.ElementFromURL(META_TVDB_TITLE_SEARCH + tvdbID[0:2] + '/' + tvdbID + '.xml')
      for row in res.xpath("/records/record"):
        t = row['title']
        l = row['lang']
        lev = lev_ratio(localTitle,t)
        titles[lang] = {'title': t, 'lev_ratio': lev, 'lang': l}
        if lev > titles.get('best_lev_ratio').get('lev_ratio'):
          titles['best_lev_ratio'] = {'title': t, 'lev_ratio': lev, 'lang': l}
    except Exception, e:
      Log(e)
      return localTitle, lang, 0.0

    bestLevRatio = titles.get('best_lev_ratio').get('lev_ratio')
    if bestLevRatio < 0:
      return localTitle, lang, 0.0

    if titles.has_key(lang):
      useTitle = titles.get(lang)
    elif titles.has_key('en'):
      useTitle = titles.get('en')
    else:
      useTitle = titles.get('best_lev_ratio')

    return (useTitle.get('title'), useTitle.get('lang'), useTitle.get('lev_ratio'))


def scrub_extra(extra, media_title):

  e = extra['extra']

  # Remove the "Movie Title: " from non-trailer extra titles.
  if media_title is not None:
    r = re.compile(media_title + ': ', re.IGNORECASE)
    e.title = r.sub('', e.title)

  # Remove the "Movie Title Scene: " from SceneOrSample extra titles.
  if media_title is not None:
    r = re.compile(media_title + ' Scene: ', re.IGNORECASE)
    e.title = r.sub('', e.title)

  # Capitalise UK correctly.
  e.title = e.title.replace('Uk', 'UK')

  return extra


def lev_ratio(s1, s2):
  distance = Util.LevenshteinDistance(safe_unicode(s1), safe_unicode(s2))
  max_len = float(max([ len(s1), len(s2) ]))

  ratio = 0.0
  try:
    ratio = float(1 - (distance/max_len))
  except:
    pass

  return ratio


def safe_unicode(s, encoding='utf-8'):
  if s is None:
    return None
  if isinstance(s, basestring):
    if isinstance(s, types.UnicodeType):
      return s
    else:
      return s.decode(encoding)
  else:
    return str(s).decode(encoding)

