import re, time, unicodedata, hashlib, urllib, urlparse, types
from chapterdb import PlexChapterDBAgent
import countrycode

FREEBASE_URL = 'https://meta.plex.tv/m/%s?lang=%s&ratings=1&reviews=1&extras=1'
PLEXMOVIE_URL = 'https://meta.plex.tv'
PLEXMOVIE_BASE = 'movie'

# CineMaterial
CINE_ROOT = 'https://api.cinematerial.com'
CINE_JSON = '%s/1/request.json?imdb_id=%%s&key=plex&secret=%%s&width=720&thumb_width=100.' % CINE_ROOT
CINE_SECRET = '157de27cd9815301d29ab8dcb2791bdf'

# PlexMovie tunables.
INITIAL_SCORE = 100 # Starting value for score before deductions are taken.
PERCENTAGE_PENALTY_MAX = 20.0 # Maximum amount to penalize matches with low percentages.
COUNT_PENALTY_THRESHOLD = 500.0 # Items with less than this value are penalized on a scale of 0 to COUNT_PENALTY_MAX.
COUNT_PENALTY_MAX = 10.0 # Maximum amount to penalize matches with low counts.
FUTURE_RELEASE_DATE_PENALTY = 10.0 # How much to penalize movies whose release dates are in the future.
YEAR_PENALTY_MAX = 10.0 # Maximum amount to penalize for mismatched years.
GOOD_SCORE = 98 # Score required to short-circuit matching and stop searching.
SEARCH_RESULT_PERCENTAGE_THRESHOLD = 80 # Minimum 'percentage' value considered credible for PlexMovie results. 

# Extras.
IVA_ASSET_URL = 'iva://api.internetvideoarchive.com/2.0/DataService/VideoAssets(%s)?lang=%s&bitrates=%s&duration=%s&adaptive=%d&dts=%d'
TYPE_ORDER = ['primary_trailer', 'trailer', 'behind_the_scenes', 'interview', 'scene_or_sample']
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


# TMDb Cut & Pasted constants
TMDB_BASE_URL = 'http://127.0.0.1:32400/services/tmdb?uri=%s'
TMDB_CONFIG = '/configuration'

# Movies
TMDB_MOVIE_SEARCH = '/search/movie?query=%s&year=%s&language=%s&include_adult=%s'
TMDB_MOVIE = '/movie/%s?append_to_response=releases,credits&language=%s'
TMDB_MOVIE_RECOMMENDATIONS = '/movie/%s/recommendations'
TMDB_MOVIE_IMAGES = '/movie/%s/images'

ARTWORK_ITEM_LIMIT = 15
POSTER_SCORE_RATIO = .3 # How much weight to give ratings vs. vote counts when picking best posters. 0 means use only ratings.
BACKDROP_SCORE_RATIO = .3
RE_IMDB_ID = Regex('^tt\d{7,10}$')

def Start():
  HTTP.CacheTime = CACHE_1WEEK
  
class PlexMovieAgent(Agent.Movies):
  name = 'Plex Movie (Legacy)'
  primary_provider = True
  accepts_from = ['com.plexapp.agents.localmedia']
  contributes_to = ['com.plexapp.agents.themoviedb']

  languages = [Locale.Language.English, Locale.Language.Swedish, Locale.Language.French, 
               Locale.Language.Spanish, Locale.Language.Dutch, Locale.Language.German, 
               Locale.Language.Italian, Locale.Language.Danish,Locale.Language.Arabic, 
               Locale.Language.Catalan, Locale.Language.Chinese, Locale.Language.Czech,
               Locale.Language.Estonian, Locale.Language.Finnish, Locale.Language.Greek,
               Locale.Language.Hebrew, Locale.Language.Hindi, Locale.Language.Hungarian,
               Locale.Language.Indonesian, Locale.Language.Japanese, Locale.Language.Korean,
               Locale.Language.Latvian, Locale.Language.Norwegian, Locale.Language.Persian,
               Locale.Language.Polish, Locale.Language.Portuguese, Locale.Language.Romanian,
               Locale.Language.Russian, Locale.Language.Slovak, Locale.Language.Thai,
               Locale.Language.Turkish, Locale.Language.Ukrainian, Locale.Language.Vietnamese]

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

  def titleyear_guid(self, title, year):
    if title is None:
      title = ''

    if year == '' or year is None or not year:
      string = "%s" % self.identifierize(title)
    else:
      string = "%s_%s" % (self.identifierize(title).lower(), year)
    return self.guidize("%s" % string)
  
  def getPublicIP(self):
    ip = HTTP.Request('https://v4.plex.tv/pms/:/ip', cacheTime=CACHE_1DAY).content.strip()
    return ip

  def getPlexMovieResults(self, media, matches, search_type='hash', plex_hash='', force=False):
    if search_type is 'hash' and plex_hash is not None:
      url = '%s/%s/hash/%s/%s.xml' % (PLEXMOVIE_URL, PLEXMOVIE_BASE, plex_hash[0:2], plex_hash)
    else:
      titleyear_guid = self.titleyear_guid(media.name,media.year)
      url = '%s/%s/guid/%s/%s.xml' % (PLEXMOVIE_URL, PLEXMOVIE_BASE, titleyear_guid[0:2], titleyear_guid)

    try:
      Log("checking %s search vector: %s" % (search_type, url))
      res = XML.ElementFromURL(url, cacheTime=0 if force else CACHE_1WEEK, headers={'Accept-Encoding':'gzip'})
      
      for match in res.xpath('//match'):
        id    = "tt%s" % match.get('guid')
        name  = safe_unicode(match.get('title'))
        year  = safe_unicode(match.get('year'))
        count = int(match.get('count', 0))
        pct   = int(match.get('percentage', 0))
        dist  = Util.LevenshteinDistance(media.name, name.encode('utf-8'))

        # Intialize.
        if not matches.has_key(id):
          matches[id] = [1000, '', None, 0, 0, 0]
          
        # Tally.
        vector = matches[id]
        vector[3] = vector[3] + pct
        vector[4] = vector[4] + count
          
        # See if a better name.
        if dist < vector[0]:
          vector[0] = dist
          vector[1] = name
          vector[2] = year

    except Exception, e:
      Log("freebase/proxy %s lookup failed: %s" % (search_type, str(e)))

  def scoreResults(self, media, matches):

    Log('Scoring ' + str(matches))

    for key in matches.keys():
      match = matches[key]

      dist = match[0]
      name = match[1]
      year = match[2]
      total_pct = match[3]
      total_cnt = match[4]
      
      # Compute score penalty for percentage/count.
      score_penalty = (100 - total_pct) * (PERCENTAGE_PENALTY_MAX / 100)
      if total_cnt < COUNT_PENALTY_THRESHOLD:
        score_penalty += (COUNT_PENALTY_THRESHOLD - total_cnt) / COUNT_PENALTY_THRESHOLD * COUNT_PENALTY_MAX

      # Year penalty/bonus.
      try:
        if int(year) > Datetime.Now().year:
          score_penalty += FUTURE_RELEASE_DATE_PENALTY

        if media.year and year:
          per_year_penalty = int(YEAR_PENALTY_MAX / 3)
          year_delta = abs(int(media.year) - (int(year)))
          if year_delta > 3:
            score_penalty += YEAR_PENALTY_MAX
          else:
            score_penalty += year_delta * per_year_penalty
      except:
        Log('Exception applying year penalty/bonus')

      # Store the final score in the result vector.
      matches[key][5] = int(INITIAL_SCORE - dist - score_penalty)

  def perform_tmdb_movie_search(self, results, media, lang, manual, get_imdb_id=False):

    # If this a manual search (Fix Incorrect Match) and we get an IMDb id as input.
    if manual and RE_IMDB_ID.search(media.name):
      tmdb_dict = GetTMDBJSON(url=TMDB_MOVIE % (media.name, lang))

      if isinstance(tmdb_dict, dict) and 'id' in tmdb_dict:

        if get_imdb_id:
          id = media.name
        else:
          id = tmdb_dict['id']

        result = MetadataSearchResult(id=id,
                                      name=tmdb_dict['title'],
                                      year=int(tmdb_dict['release_date'].split('-')[0]) if tmdb_dict['release_date'] else None,
                                      score=100,
                                      lang=lang)
        Log(result)
        results.Append(result)
        return False

    # If this is an automatic search
    else:
      if media.year and int(media.year) > 1900:
        year = media.year
      else:
        year = ''

      include_adult = 'false'
      if Prefs['adult']:
        include_adult = 'true'

      # Historically we've StrippedDiacritics() here, but this is a pretty aggressive function that won't pass
      # anything that can't be encoded to ASCII, and as such has a tendency to nuke whole titles in, e.g., Asian
      # languages (See GHI #26).  If we have a string that was modified by StripDiacritics() and we get no results,
      # try the search again with the original.
      #
      stripped_name = String.StripDiacritics(media.name)
      tmdb_dict = GetTMDBJSON(url=TMDB_MOVIE_SEARCH % (String.Quote(stripped_name, True), year, lang, include_adult))
      if media.name != stripped_name and (tmdb_dict == None or len(tmdb_dict['results']) == 0):
        Log('No results for title modified by strip diacritics, searching again with the original: ' + media.name)
        tmdb_dict = GetTMDBJSON(url=TMDB_MOVIE_SEARCH % (String.Quote(media.name, True), year, lang, include_adult))

      if isinstance(tmdb_dict, dict) and 'results' in tmdb_dict:

        for i, movie in enumerate(sorted(tmdb_dict['results'], key=lambda k: k['popularity'], reverse=True)):
          score = 90
          score = score - abs(String.LevenshteinDistance(movie['title'].lower(), media.name.lower()))

          # Adjust score slightly for 'popularity' (helpful for similar or identical titles when no media.year is present)
          score = score - (5 * i)

          if 'release_date' in movie and movie['release_date']:
            release_year = int(movie['release_date'].split('-')[0])
          else:
            release_year = -1

          if media.year and int(media.year) > 1900 and release_year:
            per_year_penalty = int(YEAR_PENALTY_MAX / 3)
            year_delta = abs(int(media.year) - (int(release_year)))
            if year_delta > 3:
              score = score - YEAR_PENALTY_MAX
            else:
              score = score - year_delta * per_year_penalty

          if score <= 0:
            continue
          else:

            if get_imdb_id and 'imdb_id' in movie and RE_IMDB_ID.search(movie['imdb_id']):
              id = str(movie['imdb_id'])
            elif get_imdb_id:
              id = imdb_id_from_tmdb(str(movie['id']))
            else:
              id = movie['id']

            if get_imdb_id and not id.startswith('tt'):
              id = 'tt%s' % id

            # Make sure ID looks like an IMDb ID
            if not re.match('t*[0-9]{7,10}', id):
              continue

            result = MetadataSearchResult(id=id,
                                          name=movie['title'],
                                          year=release_year,
                                          score=score,
                                          lang=lang)
            Log(result)
            results.Append(result)

            if score >= GOOD_SCORE:
              Log('Found perfect match with TMDb query.')

              if not manual:
                return False

    return True

  def get_originally_available_at(self, movie, country_code='', metadata=None):

    year = metadata.year if metadata else None
    originally_available_at = metadata.originally_available_at if metadata else None
    us_year = ''
    us_oaa = ''

    # Defer to TMDb release information, because it's better than IMDb
    if year and originally_available_at:
      return originally_available_at, year

    for release_date in movie.xpath('originally_available_at'):

      curr_country = release_date.get('country')
      curr_oaa = release_date.get('originally_available_at')

      if curr_country == country_code:
        elements = curr_oaa.split('-')
        try:
          if len(elements) >= 1 and len(elements[0]) == 4 and int(elements[0]):
            year = int(elements[0])

          if len(elements) == 3:
            originally_available_at = Datetime.ParseDate(curr_oaa).date()
        except Exception:
          pass

      elif curr_country == 'US':
        elements = curr_oaa.split('-')
        if len(elements) >= 1 and len(elements[0]) == 4:
          us_year = int(elements[0])

        if len(elements) == 3:
          us_oaa = Datetime.ParseDate(curr_oaa).date()

    invalid_metadata_year = not year or year < 1900

    if not originally_available_at and us_oaa and invalid_metadata_year:
      originally_available_at = us_oaa

    if invalid_metadata_year and us_year  and us_year > 1900:
      year = us_year

    return originally_available_at, year

  def search(self, results, media, lang, manual=False):

    # Keep track of best name.
    lockedNameMap = {}
    idMap = {}
    bestNameMap = {}
    bestNameDist = 1000
    bestHitScore = 0
    continueSearch = True

    # Map GUID to [distance, best name, year, percentage, count, score].
    hash_matches = {}
    title_year_matches = {}    
   
    # TODO: create a plex controlled cache for lookup
    # TODO: by imdbid  -> (title,year)

    # See if we're being passed a raw ID.
    findByIdCalled = False
    if media.guid or re.match('t*[0-9]{7,10}', media.name):
      
      theGuid = media.guid or media.name
      
      # If this looks like a TMDB GUID, get the IMDB ID.
      tmdb_search = re.search(r'^(com.plexapp.agents.themoviedb://)([\d]+)\?.+', theGuid)
      if tmdb_search and tmdb_search.group(1) and tmdb_search.group(2):
        theGuid = imdb_id_from_tmdb(tmdb_search.group(2))

      if not theGuid.startswith('tt'):
        theGuid = 'tt' + theGuid
      Log('Found an ID, attempting quick match based on: ' + theGuid)
      
      # Add a result for the id found in the passed in guid hint.
      findByIdCalled = True
      (title, year) = self.findById(theGuid, lang, False)
      if title is not None:
        bestHitScore = 100 # Treat a guid-match as a perfect score
        results.Append(MetadataSearchResult(id=theGuid, name=title, year=year, lang=lang, score=bestHitScore))
        bestNameMap[theGuid] = title
        bestNameDist = Util.LevenshteinDistance(media.name, title)
        continueSearch = False
          
    # Clean up year.
    if media.year:
      searchYear = u' (' + safe_unicode(media.year) + u')'
    else:
      searchYear = u''

    # Grab hash matches first, since a perfect score based on hash is almost certainly correct.
    # Build plex hash list and search each one.
    plexHashes = []
    if manual or continueSearch:
      try:
        for item in media.items:
          for part in item.parts:
            if part.hash: plexHashes.append(part.hash)
      except:
        try: plexHashes.append(media.hash)
        except: pass

      for plex_hash in plexHashes:
        self.getPlexMovieResults(media, hash_matches, search_type='hash', plex_hash=plex_hash)

      self.scoreResults(media, hash_matches)
      
      Log('---- HASH RESULTS MAP ----')
      Log(str(hash_matches))

      # Add scored hash results to search results.
      for key in hash_matches.keys():
        match = hash_matches[key]
        if int(match[3]) >= SEARCH_RESULT_PERCENTAGE_THRESHOLD or manual:
          best_name, year = get_best_name_and_year(key[2:], lang, match[1], match[2], lockedNameMap)
          if best_name is not None and year is not None:
            Log("Adding hash match: %s (%s) score=%d, key=%s" % (best_name, year, match[5], key))
            results.Append(MetadataSearchResult(id = key, name  = best_name, year = year, lang  = lang, score = match[5]))
            if bestHitScore < match[5]:
              bestHitScore = match[5]
        else:
          Log("Skipping hash match (doesn\'t meet percentage threshold): %s (%s) percentage=%d" % (match[1], match[2], match[3]))

      if bestHitScore >= GOOD_SCORE:
        Log('Found perfect match with plex hash query.')
        continueSearch = False

    # Grab title/year matches.
    if manual or continueSearch:
      bestHitScore = 0
      self.getPlexMovieResults(media, title_year_matches, search_type='title/year')
      self.scoreResults(media, title_year_matches)

      Log('---- TITLE_YEAR RESULTS MAP ----')
      Log(str(title_year_matches))

      # Add scored title year results to search results.
      for key in title_year_matches.keys():
        match = title_year_matches[key]
        if int(match[3]) >= SEARCH_RESULT_PERCENTAGE_THRESHOLD or manual:
          best_name, year = get_best_name_and_year(key[2:], lang, match[1], match[2], lockedNameMap)
          if best_name is not None and year is not None:
            Log("Adding title_year match: %s (%s) score=%d, key=%s" % (best_name, year, match[5], key))
            results.Append(MetadataSearchResult(id = key, name  = best_name, year = year, lang  = lang, score = match[5]))
            if bestHitScore < match[5]:
              bestHitScore = match[5]
        else:
          Log("Skipping title/year match (doesn\'t meet percentage threshold): %s (%s) percentage=%d" % (match[1], match[2], match[3]))

      if bestHitScore >= GOOD_SCORE:
        Log('Found perfect match with title/year query.')
        continueSearch = False

    # Search TMDb
    if manual or continueSearch:

      Log('---- TMDb RESULTS MAP ----')
      continueSearch = self.perform_tmdb_movie_search(results, media, lang, manual, True)

    results.Sort('score', descending=True)
    
    # Finally, de-dupe the results.
    toWhack = []
    resultMap = {}
    for result in results:
      if not resultMap.has_key(result.id):
        resultMap[result.id] = True
      else:
        toWhack.append(result)
        
    for dupe in toWhack:
      results.Remove(dupe)

    # Make sure we're using the closest names.
    for result in results:
      if not lockedNameMap.has_key(result.id) and bestNameMap.has_key(result.id):
        Log("id=%s score=%s -> Best name being changed from %s to %s" % (result.id, result.score, result.name, bestNameMap[result.id]))
        result.name = bestNameMap[result.id]
        
    # Augment with art.
    if manual == True:
      for result in results[0:3]:
        try: 
          id = re.findall('(tt[0-9]+)', result.id)[0]
          secret = Cipher.Crypt('%s%s' % (id, CINE_SECRET), 'rand0mn3ss123')
          queryJSON = JSON.ObjectFromURL(CINE_JSON % (id, secret), sleep=1.0)
          if not queryJSON.has_key('errors') and queryJSON.has_key('posters'):
            thumb_url = queryJSON['posters'][0]['thumbnail_location']
            result.thumb = thumb_url
        except:
          pass

  def update(self, metadata, media, lang, force=False):

    # If this looks like a TMDB GUID, get the IMDB ID.
    tmdb_search = re.search(r'^(com.plexapp.agents.themoviedb://)([\d]+)\?.+', metadata.guid)
    if tmdb_search and tmdb_search.group(1) and tmdb_search.group(2):
      guid = imdb_id_from_tmdb(tmdb_search.group(2))
    else:
      guid = re.findall('tt([0-9]+)', metadata.guid)[0]

    # Get all of TMDb's metadata first!
    try:
      get_tmdb_metadata(guid, lang, metadata, force)
    except Exception, e:
      Log('Unable to get TMDb movie data (in Plex Movie) for %s: %s' % (guid, str(e)))

    # Set the title. Only do this once, otherwise we'll pull new names
    # that get edited out of the database.
    #
    setTitle = False
    if media and metadata.title is None:
      setTitle = True
      metadata.title = media.title

    url = FREEBASE_URL % (guid, lang)
    movie = None

    try:
      movie = XML.ElementFromURL(url, cacheTime=0)

      if len(movie.xpath('//title')) == 0:
        Log('No IMDb details found for %s, aborting.' % guid)
        raise RuntimeWarning('No details found.')

      if Prefs['summary'] != 'The Movie Database' or metadata.summary in [None, '']:
        for summary in movie.xpath('summary'):
          if lang.strip() == summary.get('lang').strip():
            metadata.summary = summary.get('summary')

      # Title.
      if not setTitle:
        d = {}
        name,year = get_best_name_and_year(guid, lang, None, None, d, force=force)
        if name is not None:
          metadata.title = name

      # Genres.
      if len(movie.xpath('genre')) > 0:
        metadata.genres.clear()
        for genre in [g.get('genre') for g in movie.xpath('genre')]:
          metadata.genres.add(genre)

      # Directors.
      director_xml = movie.xpath('director')
      if (Prefs['cast_list'] != 'The Movie Database' or len(metadata.directors) < 1) and len(director_xml) > 0:
        metadata.directors.clear()
        for movie_director in director_xml:
          director = metadata.directors.new()
          director.name = movie_director.get('name')
        
      # Writers.
      writer_xml = movie.xpath('writer')
      if (Prefs['cast_list'] != 'The Movie Database' or len(metadata.writers) < 1) and len(writer_xml) > 0:
        metadata.writers.clear()
        for movie_writer in writer_xml:
          writer = metadata.writers.new()
          writer.name = movie_writer.get('name')
        
      # Studio
      if movie.get('company'):
        metadata.studio = movie.get('company')
        
      # Tagline.
      if len(movie.get('tagline')) > 0:
        metadata.tagline = movie.get('tagline')

      # Actors.
      actors_xml = movie.xpath('actor')
      if (Prefs['cast_list'] != 'The Movie Database' or len(metadata.roles) < 1) and len(actors_xml) > 0:
        metadata.roles.clear()
        for movie_role in actors_xml:
          role = metadata.roles.new()
          if movie_role.get('role'):
            role.role = movie_role.get('role')
          role.name = movie_role.get('name')

      # IMDb Poster - this is fairly low-res so only append it to TMDb's artwork, no prefs
      for poster in movie.xpath('poster'):
        poster_url = poster.get('url')
        try: metadata.posters[poster_url] = Proxy.Preview(HTTP.Request(poster_url).content)
        except: pass

      # Get all country based metadata (fallback to US)
      if Prefs['country'] != '' and Prefs['country'] in countrycode.COUNTRY_TO_CODE:
        country_code = countrycode.COUNTRY_TO_CODE[Prefs['country']]

        us_code = 'US'
        us_content_rating = ''

        for content_rating in movie.xpath('content_rating'):

          curr_country = content_rating.get('country')
          curr_rating = content_rating.get('content_rating')

          if curr_country == country_code:
            metadata.content_rating = curr_rating
            break
          elif curr_country == us_code:
            us_content_rating = curr_rating

        if metadata.content_rating in [None, False, ''] and us_content_rating not in [None, False, '']:
          metadata.content_rating = us_content_rating

        (metadata.originally_available_at, metadata.year) = self.get_originally_available_at(movie, country_code, metadata)

        for run_time in movie.xpath('runtime'):

          curr_country = run_time.get('country')
          curr_duration = run_time.get('runtime')

          if curr_country == country_code:
            try:
              if int(curr_duration or 0) > 0:
                metadata.duration = int(curr_duration) * 60 * 1000
                break
            except Exception:
              pass

      # Country.
      try:
        metadata.countries.clear()
        if movie.get('country'):
          country = movie.get('country')
          country = country.replace('United States of America', 'USA')
          metadata.countries.add(country)
      except:
        pass

      if Prefs['ratings'].strip() != 'The Movie Database':
        for imdb_rating in movie.xpath('imdb_ratings'):
          try:
            metadata.rating = (int(imdb_rating.get('audience_score')) or 0) / 10.0
          except TypeError:
            metadata.rating = 0.0

          metadata.audience_rating = 0.0
          metadata.rating_image = 'imdb://image.rating'
          metadata.audience_rating_image = None

    except Exception, e:
      Log('Plex_Movie_FAILED - Error obtaining Plex movie data for %s: %s' % (guid, str(e)))

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
    except NameError, e:
      Log('Not adding extras: Framework v2.5.0+ required')
      find_extras = False

    if find_extras and Prefs['extras']:

      TYPE_MAP = {'primary_trailer' : TrailerObject,
                  'trailer' : TrailerObject,
                  'interview' : InterviewObject,
                  'behind_the_scenes' : BehindTheScenesObject,
                  'scene_or_sample' : SceneOrSampleObject}

      try:
        xml = movie

        extras = []
        media_title = None
        for extra in xml.xpath('//extra'):
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
          if extra_type == 'trailer' or extra_type == 'scene_or_sample':
            include = False
          
          # Don't include anything besides trailers if pref is set.
          if extra_type != 'primary_trailer' and Prefs['only_trailers']:
            include = False

          if include:

            bitrates = extra.get('bitrates') or ''
            duration = int(extra.get('duration') or 0)
            adaptive = 1 if extra.get('adaptive') == 'true' else 0
            dts = 1 if extra.get('dts') == 'true' else 0

            # Remember the title if this is the primary trailer.
            if extra_type == 'primary_trailer':
              media_title = extra.get('title')

            # Add the extra.
            if extra_type in TYPE_MAP:
              extras.append({ 'type' : extra_type,
                              'lang' : spoken_lang,
                              'extra' : TYPE_MAP[extra_type](url=IVA_ASSET_URL % (extra.get('iva_id'), spoken_lang, bitrates, duration, adaptive, dts),
                                                             title=extra.get('title'),
                                                             year=avail.year,
                                                             originally_available_at=avail,
                                                             thumb=extra.get('thumb') or '')})
            else:
                Log('Skipping extra %s because type %s was not recognized.' % (extra.get('iva_id'), extra_type))

        # Sort the extras, making sure the primary trailer is first.
        extras.sort(key=lambda e: TYPE_ORDER.index(e['type']))

        # If red band trailers were requested in prefs, see if we have one and swap it in.
        if Prefs['redband']:
          redbands = [t for t in xml.xpath('//extra') if t.get('type') == 'trailer' and re.match(r'.+red.?band.+', t.get('title'), re.IGNORECASE) and IVA_LANGUAGES.get(int(t.get('lang_code') or -1)) == lang]
          if len(redbands) > 0:
            extra = redbands[0]
            adaptive = 1 if extra.get('adaptive') == 'true' else 0
            dts = 1 if extra.get('dts') == 'true' else 0
            extras[0]['extra'].url = IVA_ASSET_URL % (extra.get('iva_id'), lang, extra.get('bitrates') or '', int(extra.get('duration') or 0), adaptive, dts)
            extras[0]['extra'].thumb = extra.get('thumb') or ''
            Log('Adding red band trailer: ' + extra.get('iva_id'))

        # If our primary trailer is in English but the library language is something else, see if we can do better.
        if lang != Locale.Language.English and extras and extras[0]['lang'] == Locale.Language.English:
          lang_matches = [t for t in xml.xpath('//extra') if t.get('type') == 'trailer' and IVA_LANGUAGES.get(int(t.get('subtitle_lang_code') or -1)) == lang]
          lang_matches += [t for t in xml.xpath('//extra') if t.get('type') == 'trailer' and IVA_LANGUAGES.get(int(t.get('lang_code') or -1)) == lang]
          if len(lang_matches) > 0:
            extra = lang_matches[0]
            spoken_lang = IVA_LANGUAGES.get(int(extra.get('lang_code') or -1)) or Locale.Language.Unknown
            adaptive = 1 if extra.get('adaptive') == 'true' else 0
            dts = 1 if extra.get('dts') == 'true' else 0
            extras[0]['lang'] = spoken_lang
            extras[0]['extra'].url = IVA_ASSET_URL % (extra.get('iva_id'), spoken_lang, extra.get('bitrates') or '', int(extra.get('duration') or 0), adaptive, dts)
            extras[0]['extra'].thumb = extra.get('thumb') or ''
            Log('Adding trailer with spoken language %s and subtitled langauge %s to match library language.' % (spoken_lang, IVA_LANGUAGES.get(int(extra.get('subtitle_lang_code') or -1)) or Locale.Language.Unknown))

        # Clean up the found extras.
        extras = [scrub_extra(extra, media_title) for extra in extras]

        # Add them in the right order to the metadata.extras list.
        for extra in extras:
          metadata.extras.add(extra['extra'])

        Log('Added %d of %d extras.' % (len(metadata.extras), len(xml.xpath('//extra'))))

      except Ex.HTTPError, e:
        if e.code == 403:
          Log('Skipping online extra lookup (an active Plex Pass is required).')

      except Exception, e:
        Log('Error obtaining extras for %s: %s' % (guid, str(e)))

    # Rotten Tomatoes Ratings and Reviews.

    # Do a quick check to make sure we've got the attributes available in this
    # framework version, and that the server is new enough to read them.
    #
    try:
      find_ratings = True

      if not hasattr(metadata, 'audience_rating'):
        find_ratings = False
        Log('Not adding Rotten Tomatoes ratings: Framework v2.5.1+ required.')

      if not Util.VersionAtLeast(Platform.ServerVersion, 0,9,9,16):
        find_ratings = False
        Log('Not adding Rotten Tomateos ratings: Server v0.9.9.16+ required.')

      # Ratings.
      if (Prefs['ratings'].strip() == 'Rotten Tomatoes' or metadata.rating is None) and find_ratings and movie.xpath('rating') is not None:

        rating_image_identifiers = {'Certified Fresh' : 'rottentomatoes://image.rating.certified', 'Fresh' : 'rottentomatoes://image.rating.ripe', 'Ripe' : 'rottentomatoes://image.rating.ripe', 'Rotten' : 'rottentomatoes://image.rating.rotten', None : ''}
        audience_rating_image_identifiers = {'Upright' : 'rottentomatoes://image.rating.upright', 'Spilled' : 'rottentomatoes://image.rating.spilled', None : ''}

        ratings = movie.xpath('//ratings')
        if ratings:

          ratings = ratings[0]

          try:
            metadata.rating = float(ratings.get('critics_score', 0.0)) / 10
          except TypeError:
            metadata.rating = 0.0

          try:
            metadata.audience_rating = float(ratings.get('audience_score', 0.0)) / 10
          except:
            metadata.audience_rating = 0.0

          try:
            metadata.rating_image = rating_image_identifiers[ratings.get('critics_rating', None)]
          except KeyError:
            metadata.rating_image = ''

          try:
            metadata.audience_rating_image = audience_rating_image_identifiers[ratings.get('audience_rating', None)]
          except KeyError:
            metadata.audience_rating_image = ''

      # Reviews.
      metadata.reviews.clear()
      if find_ratings:
        for review in movie.xpath('//review'):
          if review.text not in [None, False, '']:
            r = metadata.reviews.new()
            r.author = review.get('critic')
            r.source = review.get('publication')
            r.image = 'rottentomatoes://image.review.fresh' if review.get('freshness') == 'fresh' else 'rottentomatoes://image.review.rotten'
            r.link = review.get('link')
            r.text = review.text
    except Exception, e:
      Log('Error obtaining Rotten tomato data for %s: %s' % (guid, str(e)))

    try:
      # chapters
      chapterAgent = PlexChapterDBAgent()
      chapterAgent.update(metadata, media, lang)
    except Exception, e:
      Log('Error obtaining Plex movie Chapter data for %s: %s' % (guid, str(e)))

    m = re.search('(tt[0-9]+)', metadata.guid)
    if m and not metadata.year:
      id = m.groups(1)[0]
      (title, year) = self.findById(id, lang, force)
      if year:
        metadata.year = int(year)

  def findById(self, id, lang, force):
    title = None
    year = None

    # Try Freebase first, since spamming Google will easily get us blocked
    url = FREEBASE_URL % (id[2:], lang)

    try:
      movie = XML.ElementFromURL(url, cacheTime=0 if force else CACHE_1WEEK, headers={'Accept-Encoding':'gzip'})

      # Title
      if len(movie.get('title')) > 0:
        title = movie.get('title')

      # Year
      (originally_available_at, year) = self.get_originally_available_at(movie)
    except:
      pass

    if title and year:
      return safe_unicode(title), safe_unicode(year)
    elif title:
      return safe_unicode(title), None
    else:
      return None, None


def parseIMDBTitle(title, url):

  titleLc = title.lower()

  result = {
    'title':  None,
    'year':   None,
    'type':   'movie',
    'imdbId': None,
  }

  try:
    (scheme, host, path, params, query, fragment) = urlparse.urlparse(url)
    path      = re.sub(r"/+$","",path)
    pathParts = path.split("/")
    lastPathPart = pathParts[-1]

    if host.count('imdb.') == 0:
      ## imdb is not in the server.. bail
      return None

    if lastPathPart == 'quotes':
      ## titles on these parse fine but are almost
      ## always wrong
      return None

    if lastPathPart == 'videogallery':
      ## titles on these parse fine but are almost
      ## always wrong
      return None

    # parse the imdbId
    m = re.search('/(tt[0-9]+)/?', path)
    imdbId = m.groups(1)[0]
    result['imdbId'] = imdbId

    ## hints in the title
    if titleLc.count("(tv series") > 0:
      result['type'] = 'tvseries'
    elif titleLc.endswith("episode list"):
      result['type'] = 'tvseries'
    elif titleLc.count("(tv episode") > 0:
      result['type'] = 'tvepisode'
    elif titleLc.count("(vg)") > 0:
      result['type'] = 'videogame'
    elif titleLc.count("(video game") > 0:
      result['type'] = 'videogame'

    # NOTE: it seems that titles of the form
    # (TV 2008) are made for TV movies and not
    # regular TV series... I think we should
    # let these through as "movies" as it includes
    # stand up commedians, concerts, etc

    # NOTE: titles of the form (Video 2009) seem
    # to be straight to video/dvd releases
    # these should also be kept intact
  
    # hints in the url
    if lastPathPart == 'episodes':
      result['type'] = 'tvseries'

    # Parse out title, year, and extra.
    titleRx = '(.*) \(([^0-9]+ )?([0-9]+)(/.*)?.*?\).*'
    m = re.match(titleRx, title)
    if m:
      # A bit more processing for the name.
      result['title'] = cleanupIMDBName(m.groups()[0])
      result['year'] = int(m.groups()[2])
      
    else:
      longTitleRx = '(.*\.\.\.)'
      m = re.match(longTitleRx, title)
      if m:
        result['title'] = cleanupIMDBName(m.groups(1)[0])
        result['year']  = None

    if result['title'] is None:
      return None

    return result
  except:
    return None
 
def cleanupIMDBName(s):
  imdbName = re.sub('^[iI][mM][dD][bB][ ]*:[ ]*', '', s)
  imdbName = re.sub('^details - ', '', imdbName)
  imdbName = re.sub('(.*:: )+', '', imdbName)
  imdbName = HTML.ElementFromString(imdbName).text

  if imdbName:
    if imdbName[0] == '"' and imdbName[-1] == '"':
      imdbName = imdbName[1:-1]
    return imdbName

  return None

def safe_unicode(s,encoding='utf-8'):
  if s is None:
    return None
  if isinstance(s, basestring):
    if isinstance(s, types.UnicodeType):
      return s
    else:
      return s.decode(encoding)
  else:
    return str(s).decode(encoding)

def get_best_name_and_year(guid, lang, fallback, fallback_year, best_name_map, force=False):
  ret = (fallback, fallback_year)

  try:
    tmdb_data = get_base_tmdb_data('tt%s' % guid, lang if Prefs['title'] else '', force)

    if tmdb_data and 'release_date' in tmdb_data:
      try: fallback_year = tmdb_data['release_date'][:4]
      except: pass

    if tmdb_data and 'title' in tmdb_data:
      ret = (tmdb_data['title'], fallback_year)
    else:
      return None, None

    # Note that we returned a pristine name.
    best_name_map['tt' + guid] = True
    return ret

  except:
    Log("Error getting best name.")

  return ret
  
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

def imdb_id_from_tmdb(tmdb_id):

  imdb_id = None

  try:
    imdb_id = Core.messaging.call_external_function('com.plexapp.agents.themoviedb', 'MessageKit:GetImdbId', kwargs=dict(tmdb_id=tmdb_id))
  except Ex.HTTPError, e:
    Log('Error: Cannot get imdb id from tmdb id (%s) - %s' % (tmdb_id, str(e)))

  if imdb_id is not None:
    imdb_id = imdb_id.replace('tt','')
  else:
    imdb_id = ''

  return imdb_id


def get_tmdb_metadata(id, lang, metadata, force):
  metadata_dict = PerformTMDbMovieUpdate('tt%s' % id, lang, metadata, force=force)
  return tmdb_dict_to_movie_metadata_obj(metadata_dict, metadata)


def tmdb_add_person(metadata_person_list, person):
  meta_person = metadata_person_list.new()
  if 'role' in person:
    meta_person.role = person['role']

  if 'name' in person:
    meta_person.name = person['name']

  if 'photo' in person:
    meta_person.photo = person['photo']


def tmdb_dict_to_movie_metadata_obj(metadata_dict, metadata):

  try:
    if not metadata or not metadata.attrs:
      return
  except AttributeError:
    Log('WARNING: Framework not new enough to use One True Agent')  # TODO: add a more official log message about version number when available
    return

  for attr_name, attr_obj in metadata.attrs.iteritems():

    if type(attr_obj) == Framework.modelling.attributes.SetObject:
      attr_obj.clear()

    if attr_name not in metadata_dict:
      continue

    dict_value = metadata_dict[attr_name]

    if isinstance(dict_value, list):

      attr_obj.clear()
      for val in dict_value:
        attr_obj.add(val)

    elif isinstance(dict_value, dict):

      if attr_name in ['posters', 'art', 'themes']:  # Can't access MapObject, so have to write these out

        for k, v in dict_value.iteritems():
          if isinstance(v, tuple):
            try: attr_obj[k] = Proxy.Preview(HTTP.Request(v[0]).content, sort_order=v[1])
            except: pass
          else:
            try: attr_obj[k] = Proxy.Preview(HTTP.Request(v[0]).content)
            except: pass            

        attr_obj.validate_keys(dict_value.keys())

      else:
        for k, v in dict_value.iteritems():
          attr_obj[k] = v

    elif attr_name is 'originally_available_at':

        try:
          attr_obj.setcontent(Datetime.ParseDate(dict_value).date())
        except:
          pass

    else:
      attr_obj.setcontent(dict_value)

  # Roles is a special kind of object
  if 'roles' in metadata_dict:
    metadata.roles.clear()

    for role in metadata_dict['roles']:
      tmdb_add_person(metadata.roles, role)

  if 'directors' in metadata_dict:
    metadata.directors.clear()

    for director in metadata_dict['directors']:
      tmdb_add_person(metadata.directors, director)

  if 'writers' in metadata_dict:
    metadata.writers.clear()

    for writer in metadata_dict['writers']:
      tmdb_add_person(metadata.writers, writer)

  if 'producers' in metadata_dict:
    metadata.producers.clear()

    for producer in metadata_dict['producers']:
      tmdb_add_person(metadata.producers, producer)
      
  if 'similar' in metadata_dict:
    metadata.similar.clear()
    for similar in metadata_dict['similar']:
      metadata.similar.add(similar)


####################################################################################################
def get_base_tmdb_data(metadata_id, lang, force=False):
  args = {}
  if force:
    args['cache_time'] = 0

  tmdb_dict = GetTMDBJSON(url=TMDB_MOVIE % (metadata_id, lang), **args)

  if not isinstance(tmdb_dict, dict) or 'overview' not in tmdb_dict or tmdb_dict['overview'] is None or tmdb_dict['overview'] == "":
    # Retry the query with no language specified if we didn't get anything from the initial request.
    tmdb_dict = GetTMDBJSON(url=TMDB_MOVIE % (metadata_id, ''), **args)

  return tmdb_dict


####################################################################################################
def PerformTMDbMovieUpdate(metadata_id, lang, existing_metadata, force=False):  # Shared with TheMovieDB.bundle

  metadata = dict(id=metadata_id)

  config_dict = GetTMDBJSON(url=TMDB_CONFIG, cache_time=0 if force else CACHE_1WEEK * 2)
  tmdb_dict = get_base_tmdb_data(metadata_id, lang, force)

  jsonArgs = {}
  if force:
    jsonArgs['cache_time'] = 0

  # This additional request is necessary since full art/poster lists are not returned if they don't exactly match the language
  tmdb_images_dict = GetTMDBJSON(url=TMDB_MOVIE_IMAGES % metadata_id, **jsonArgs)

  # Recommendations.
  try:
    metadata['similar'] = []
    tmdb_rec_dict = GetTMDBJSON(url=TMDB_MOVIE_RECOMMENDATIONS % (tmdb_dict['id']), **jsonArgs)
    for rec in tmdb_rec_dict['results']:
      metadata['similar'].append(rec['title'])
  except:
    pass

  if not isinstance(tmdb_dict, dict) or not isinstance(tmdb_images_dict, dict):
    return None

  # Rating.
  votes = tmdb_dict['vote_count']
  rating = tmdb_dict['vote_average']
  if votes > 3:
    metadata['rating'] = rating
    metadata['audience_ratinge'] = 0.0
    metadata['rating_image'] = None
    metadata['audience_rating_image'] = None

  # Title of the film.
  metadata['title'] = tmdb_dict['title'].strip()

  if 'original_title' in tmdb_dict and tmdb_dict['original_title'] != tmdb_dict['title']:
    metadata['original_title'] = tmdb_dict['original_title'].strip()

  # Tagline.
  metadata['tagline'] = tmdb_dict['tagline'].strip()

  # Release date.
  primary_date_set = False
  try:
    metadata['originally_available_at'] = tmdb_dict['release_date']
    metadata['year'] = Datetime.ParseDate(tmdb_dict['release_date']).date().year
    primary_date_set = True if metadata['originally_available_at'] else False
  except:
    pass

  if Prefs['country'] != '':
    c = Prefs['country']
    localized_date_set = False

    for country in tmdb_dict['releases']['countries']:
      if country['iso_3166_1'] == countrycode.COUNTRY_TO_CODE[c]:

        # Content rating.
        if 'certification' in country and country['certification'] != '':
          if countrycode.COUNTRY_TO_CODE[c] == 'US':
            metadata['content_rating'] = country['certification']
          else:
            metadata['content_rating'] = '%s/%s' % (countrycode.COUNTRY_TO_CODE[c].lower(), country['certification'])

        # Release date (country specific) only if primary
        if not metadata['year'] and 'release_date' in country and country['release_date'] != '':
          try:
            metadata['originally_available_at'] = country['release_date']
            metadata['year'] = Datetime.ParseDate(country['release_date']).date().year
          except:
            pass

        break

      if not primary_date_set and 'release_date' in country and country['release_date']:
        try:
          release_date = Datetime.ParseDate(country['release_date']).date()
          if not localized_date_set or (release_date and release_date < Datetime.ParseDate(metadata['originally_available_at']).date()):
            metadata['originally_available_at'] = country['release_date']
            metadata['year'] = release_date.year
            localized_date_set = True if metadata['originally_available_at'] else False
        except:
          pass

  # Summary.
  metadata['summary'] = String.DecodeHTMLEntities(tmdb_dict['overview'])
  if metadata['summary'] == 'No overview found.':
    metadata['summary'] = ""

  # Runtime.
  try: metadata['duration'] = int(tmdb_dict['runtime']) * 60 * 1000
  except: pass

  # Genres.
  metadata['genres'] = []
  for genre in tmdb_dict['genres']:
    metadata['genres'].append(genre['name'].strip())

  # Collections.
  metadata['collections'] = []
  if Prefs['collections'] and tmdb_dict['belongs_to_collection'] is not None:
    metadata['collections'].append(tmdb_dict['belongs_to_collection']['name'].replace(' Collection',''))

  # Studio.
  if 'production_companies' in tmdb_dict and len(tmdb_dict['production_companies']) > 0:
    index = tmdb_dict['production_companies'][0]['id']
    company = None

    for studio in tmdb_dict['production_companies']:
      if studio['id'] <= index:
        index = studio['id']
        company = studio['name'].strip()

    metadata['studio'] = company

  else:
    metadata['studio'] = None

  # Country.
  metadata['countries'] = []
  if 'production_countries' in tmdb_dict:
    for country in tmdb_dict['production_countries']:
      country = country['name'].replace('United States of America', 'USA')
      metadata['countries'].append(country)

  # Crew.
  metadata['directors'] = []
  metadata['writers'] = []
  metadata['producers'] = []

  for member in tmdb_dict['credits']['crew']:
    if member['job'] == 'Director':
      director = dict(name=member['name'].strip())

      if member.get('profile_path', None) is not None:
        director['photo'] = config_dict['images']['base_url'] + 'original' + member['profile_path']

      metadata['directors'].append(director)

    elif member['job'] in ('Writer', 'Screenplay', 'Author'):
      writer = dict(name=member['name'].strip())

      if member.get('profile_path', None) is not None:
        writer['photo'] = config_dict['images']['base_url'] + 'original' + member['profile_path']

      metadata['writers'].append(writer)

    elif member['job'] == 'Producer':
      producer = dict(name=member['name'].strip())

      if member.get('profile_path', None) is not None:
        producer['photo'] = config_dict['images']['base_url'] + 'original' + member['profile_path']

      metadata['producers'].append(producer)

  # Cast.
  metadata['roles'] = []

  for member in sorted(tmdb_dict['credits']['cast'], key=lambda k: k['order']):
    role = dict()
    role['role'] = member['character'].strip()
    role['name'] = member['name'].strip()
    if member['profile_path'] is not None:
      role['photo'] = config_dict['images']['base_url'] + 'original' + member['profile_path']
    metadata['roles'].append(role)

  # Note: for TMDB artwork, number of votes is a good predictor of poster quality. Ratings are assigned
  # using a Baysean average that appears to be poorly calibrated, so ratings are almost always between
  # 5 and 6 or zero.  Consider both of these, weighting them according to the POSTER_SCORE_RATIO.

  # No votes get zero, use TMDB's apparent initial Baysean prior mean of 5 instead.
  valid_names = list()

  metadata['posters'] = {}

  if tmdb_images_dict['posters']:
    max_average = max([(lambda p: p['vote_average'] or 5)(p) for p in tmdb_images_dict['posters']])
    max_count = max([(lambda p: p['vote_count'])(p) for p in tmdb_images_dict['posters']]) or 1

    for i, poster in enumerate(tmdb_images_dict['posters']):

      score = (poster['vote_average'] / max_average) * POSTER_SCORE_RATIO
      score += (poster['vote_count'] / max_count) * (1 - POSTER_SCORE_RATIO)
      tmdb_images_dict['posters'][i]['score'] = score

      # Boost the score for localized posters (according to the preference).
      if Prefs['localart']:
        if poster['iso_639_1'] == lang:
          tmdb_images_dict['posters'][i]['score'] = poster['score'] + 1

      # Discount score for foreign posters.
      if poster['iso_639_1'] != lang and poster['iso_639_1'] is not None and poster['iso_639_1'] != 'en':
        tmdb_images_dict['posters'][i]['score'] = poster['score'] - 1

    for i, poster in enumerate(sorted(tmdb_images_dict['posters'], key=lambda k: k['score'], reverse=True)):
      if i > ARTWORK_ITEM_LIMIT:
        break
      else:
        poster_url = config_dict['images']['base_url'] + 'original' + poster['file_path']
        thumb_url = config_dict['images']['base_url'] + 'w154' + poster['file_path']
        valid_names.append(poster_url)

        if poster_url not in metadata['posters']:
          try: metadata['posters'][poster_url] = (thumb_url, i+1)
          except: pass

  # Backdrops.
  valid_names = list()
  metadata['art'] = {}
  if tmdb_images_dict['backdrops']:
    max_average = max([(lambda p: p['vote_average'] or 5)(p) for p in tmdb_images_dict['backdrops']])
    max_count = max([(lambda p: p['vote_count'])(p) for p in tmdb_images_dict['backdrops']]) or 1

    for i, backdrop in enumerate(tmdb_images_dict['backdrops']):

      score = (backdrop['vote_average'] / max_average) * BACKDROP_SCORE_RATIO
      score += (backdrop['vote_count'] / max_count) * (1 - BACKDROP_SCORE_RATIO)
      tmdb_images_dict['backdrops'][i]['score'] = score

      # For backdrops, we prefer "No Language" since they're intended to sit behind text.
      if backdrop['iso_639_1'] == 'xx' or backdrop['iso_639_1'] == 'none' or backdrop['iso_639_1'] == None:
        tmdb_images_dict['backdrops'][i]['score'] = float(backdrop['score']) + 2

      # Boost the score for localized art (according to the preference).
      if Prefs['localart']:
        if backdrop['iso_639_1'] == lang:
          tmdb_images_dict['backdrops'][i]['score'] = float(backdrop['score']) + 1

      # Discount score for foreign art.
      if backdrop['iso_639_1'] != lang and backdrop['iso_639_1'] is not None and backdrop['iso_639_1'] != 'en':
        tmdb_images_dict['backdrops'][i]['score'] = float(backdrop['score']) - 1

    for i, backdrop in enumerate(sorted(tmdb_images_dict['backdrops'], key=lambda k: k['score'], reverse=True)):
      if i > ARTWORK_ITEM_LIMIT:
        break
      else:
        backdrop_url = config_dict['images']['base_url'] + 'original' + backdrop['file_path']
        thumb_url = config_dict['images']['base_url'] + 'w300' + backdrop['file_path']
        valid_names.append(backdrop_url)

        if backdrop_url not in metadata['art']:
          try: metadata['art'][backdrop_url] = (thumb_url, i+1)
          except: pass

  return metadata


####################################################################################################
def GetTMDBJSON(url, cache_time=CACHE_1MONTH):

  tmdb_dict = None

  try:
    tmdb_dict = JSON.ObjectFromURL(TMDB_BASE_URL % String.Quote(url, True), sleep=2.0, headers={'Accept': 'application/json'}, cacheTime=cache_time)
  except:
    Log('Error fetching JSON from The Movie Database: %s' % (TMDB_BASE_URL % String.Quote(url, True)))

  return tmdb_dict
