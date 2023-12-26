# Rewrite (use JSON API, other matching tweaks) by ToMM

import time

# Last.fm API
API_KEY = 'd5310352469c2631e5976d0f4a599773'
BASE_URL = 'https://lastfm-z.plexapp.com/2.0/'

ARTIST_SEARCH_URL = BASE_URL + '?method=artist.search&artist=%s&page=%d&limit=%d&format=json&api_key=' + API_KEY
ARTIST_ALBUM_SEARCH_URL = BASE_URL + '?method=artist.gettopalbums&artist=%s&page=%s&limit=%s&format=json&api_key=' + API_KEY
ARTIST_INFO_URL = BASE_URL + '?method=artist.getInfo&artist=%s&autocorrect=1&lang=%s&format=json&api_key=' + API_KEY
ARTIST_TOP_TRACKS_URL = BASE_URL + "?method=artist.getTopTracks&artist=%s&lang=%s&page=%d&limit=100&format=json&api_key=" + API_KEY
ARTIST_SIMILAR_ARTISTS_URL = BASE_URL + "?method=artist.getSimilar&artist=%s&lang=%s&format=json&limit=20&api_key=" + API_KEY

ALBUM_SEARCH_URL = BASE_URL + '?method=album.search&album=%s&limit=%s&format=json&api_key=' + API_KEY
ALBUM_INFO_URL = BASE_URL + '?method=album.getInfo&artist=%s&album=%s&autocorrect=1&lang=%s&format=json&api_key=' + API_KEY

# Concert information from SongKick
SONGKICK_BASE_URL = "http://127.0.0.1:32400/services/songkick?uri=%s"
SONGKICK_ARTIST_EVENTS_URL_MBID = "/artists/mbid:%s/calendar.json"
SONGKICK_ARTIST_EVENTS_URL_SONGKICKID = "/artists/%s/calendar.json"
SONGKICK_ARTIST_SEARCH_URL = "/search/artists.json?query=%s"

# MusicBrainz mbid lookup
MB_ARTIST_URL = 'https://musicbrainz.plex.tv/ws/2/artist/%s'
MB_NS = {'a': 'http://musicbrainz.org/ns/mmd-2.0#'}
MB_HEADERS = {'User-Agent':'Plex Music Agent/1.0 (http://plex.tv)'}

ARTWORK_SIZE_RANKING = { 'mega':0 , 'extralarge':1 , 'large':2 } # Don't even try to add 'medium' or 'small' artwork.
VARIOUS_ARTISTS_POSTER = 'https://music.plex.tv/pixogs/various_artists_poster.jpg'

# Tunables.
ARTIST_MATCH_LIMIT = 9 # Max number of artists to fetch for matching purposes.
ARTIST_MATCH_MIN_SCORE = 75 # Minimum score required to add to custom search results.
ARTIST_MANUAL_MATCH_LIMIT = 120 # Number of artists to fetch when trying harder for manual searches.  Multiple API hits.
ARTIST_SEARCH_PAGE_SIZE = 30 # Number of artists in a search result page.  Asking for more has no effect.
ARTIST_ALBUMS_MATCH_LIMIT = 3 # Max number of artist matches to try for album bonus.  Each one incurs an additional API request.
ARTIST_ALBUMS_LIMIT = 50 # Number of albums by artist to grab for artist matching bonus and quick album match.
ARTIST_MIN_LISTENER_THRESHOLD = 250 # Minimum number of listeners for an artist to be considered credible.
ARTIST_MATCH_GOOD_SCORE = 90 # Include artists with this score or higher regardless of listener count.
ALBUM_MATCH_LIMIT = 8 # Max number of results returned from standalone album searches with no artist info (e.g. Various Artists).
ALBUM_MATCH_MIN_SCORE = 75 # Minimum score required to add to custom search results.
ALBUM_MATCH_GOOD_SCORE = 96 # Minimum score required to rely on only Albums by Artist and not search.
ALBUM_TRACK_BONUS_MATCH_LIMIT = 5 # Max number of albums to try for track bonus.  Each one incurs at most one API request per album.
QUERY_SLEEP_TIME = 0.1 # How long to sleep before firing off each API request.

# Advanced tunables.
NAME_DISTANCE_THRESHOLD = 2 # How close do album/track names need to be to match for bonuses?
ARTIST_INITIAL_SCORE = 90 # Starting point for artists before bonus/deductions.
ARTIST_ALBUM_BONUS_INCREMENT = 1 # How much to boost the bonus for a each good artist/album match.
ARTIST_ALBUM_MAX_BONUS = 15 # Maximum number of bonus points to give artists with good album matches.
ARTIST_MAX_DIST_PENALTY = 40 # Maxiumum amount to penalize for Lev ratio difference in artist names.
ALBUM_INITIAL_SCORE = 92 # Starting point for albums before bonus/deductions.
ALBUM_NAME_DIST_COEFFICIENT = 3 # Multiply album Lev. distance to give it a bit more weight.
ALBUM_TRACK_BONUS_INCREMENT = 1 # How much to boost the bonus for a each good album/track match.
ALBUM_TRACK_MAX_BONUS = 20 # Maximum number of bonus points to give to albums with good track name matches.
ALBUM_TRACK_BONUS_MAX_ARTIST_DSIT = 2 # How similar do the parent artist and album search result artist need to be to ask for info?
ALBUM_NUM_TRACKS_BONUS = 5 # How much to boost the bonus if the total number of tracks match.

RE_STRIP_PARENS = Regex('\([^)]*\)')


def Start():
  HTTP.CacheTime = CACHE_1WEEK

@expose
def ArtistMbidLookup(lastfm_artist):
  artist_mbid = None
  if lastfm_artist and 'mbid' in lastfm_artist and len(lastfm_artist['mbid']) == 36:  # Sanity check.
    artist_mbid = lastfm_artist['mbid']

    # See if there's an updated MBID.
    try: artist_mbid = XML.ElementFromURL(MB_ARTIST_URL % artist_mbid, headers=MB_HEADERS).xpath('//a:artist/@id', namespaces=MB_NS)[0]
    except: pass

  if artist_mbid is not None:
    Log('Found MBID: %s' % artist_mbid)
  else:
    Log('Couldn\'t find MBID.')

  return artist_mbid

@expose
def GetMusicBrainzId(artist, album=None):
  if album:
    dict = GetAlbum(String.Quote(artist), String.Quote(album))
  else:
    dict = GetArtist(String.Quote(artist))

  if 'mbid' in dict:
    return dict['mbid']
  return None


@expose
def ArtistSearch(artist, albums=[], lang='en'):
  if artist == '[Unknown Artist]' or artist == 'Various Artists' or artist == 'OST':
    return
  artist_results = []
  artists = SearchArtists(artist, ARTIST_MATCH_LIMIT)

  # Extra shot if there's an & in there.
  if '&' in artist:
    artists += SearchArtists(artist.replace('&', 'and'), ARTIST_MATCH_LIMIT)

  score_artists(artists, artist, albums, lang, artist_results)
  if len(artist_results) > 0 and artist_results[0].score >= 85:
    return GetArtist(artist_results[0].id)

@expose
def AlbumSearch(artist, album, year, lang):
  id = String.Quote(artist.decode('utf-8').encode('utf-8')).replace(' ','+')

  # Try by top albums.
  albums = GetAlbumsByArtist(id, albums=[])
  for a in albums:
    if LevenshteinRatio(a['name'], album) > 0.95:
      album_id = String.Quote(a['name'].decode('utf-8').encode('utf-8')).replace(' ','+')
      return GetAlbum(id, album_id, lang)

  # Try looking up album directly.
  albums = SearchAlbums(album, limit=50, legacy=False)
  for a in albums:
    artist_id = String.Quote(a['artist'].decode('utf-8').encode('utf-8')).replace(' ','+')
    if LevenshteinRatio(a['name'], album) > 0.95 and artist_id == id:
      album_id = String.Quote(a['name'].decode('utf-8').encode('utf-8')).replace(' ','+')
      return GetAlbum(artist_id, album_id, lang)

  return None

@expose
def ArtistTopTracks(artist, lang='en'):
  id = String.Quote(artist.decode('utf-8').encode('utf-8')).replace(' ','+')
  return GetArtistTopTracks(id, lang)


@expose
def ArtistGetSimilar(artist, lang='en'):
  id = String.Quote(artist.decode('utf-8').encode('utf-8')).replace(' ','+')
  return GetArtistSimilar(id, lang)


@expose
def ArtistGetEvents(lastfm_artist):
  artist_mbid = ArtistMbidLookup(lastfm_artist)
  artist_songkickid = None

  if artist_mbid is None:
    artist_songkickid = GetArtistSongkickId(lastfm_artist['name'])

  return GetArtistEventsFromSongkickById(artist_mbid=artist_mbid, artist_songkickid=artist_songkickid)


# Score lists of artist results.  Permutes artist_results list.
def score_artists(artists, media_artist, media_albums, lang, artist_results):

  for i, artist in enumerate(artists):

    # Need to coerce this into a utf-8 string so String.Quote() escapes the right characters.
    id = String.Quote(artist['name'].decode('utf-8').encode('utf-8')).replace(' ','+')

    # Search returns ordered results, but no numeric score, so we approximate one with Levenshtein ratio.
    dist = int(ARTIST_MAX_DIST_PENALTY - ARTIST_MAX_DIST_PENALTY * LevenshteinRatio(artist['name'].lower(), media_artist.lower()))

    # If the match is exact, bonus.
    if artist['name'].lower() == media_artist.lower():
      dist = dist - 1

    # Fetching albums in order to apply bonus is expensive, so only do it for the top N artist matches.
    if i < ARTIST_ALBUMS_MATCH_LIMIT:
      bonus = get_album_bonus(media_albums, artist_id=id)
    else:
      bonus = 0

    # Adjust the score.
    score = ARTIST_INITIAL_SCORE + bonus - dist

    # Finally, apply some heuristics based on listener count. If there's only a single result, it will not include the 'listeners' key.
    # Single results tend to be a good matches. Distrust artists with fewer than N listeners if it was not a really good match.
    #
    if len(artists) > 1 and artist.has_key('listeners') and int(artist['listeners']) < ARTIST_MIN_LISTENER_THRESHOLD and score < ARTIST_MATCH_GOOD_SCORE:
      Log('Skipping %s with only %s listeners and score of %s.' % (artist['name'], artist['listeners'], score))
      continue

    name = artist['name']
    listeners = artist['listeners'] if artist.has_key('listeners') else '(no listeners data)'
    Log('Artist result: ' + name + ' dist: ' + str(dist) + ' album bonus: ' + str(bonus) + ' listeners: ' + str(listeners) + ' score: ' + str(score))

    # Skip matches that don't meet the minimum score.  There many be many, especially if this was a manual search.
    if score >= ARTIST_MATCH_MIN_SCORE:
      artist_results.append(MetadataSearchResult(id=id, name=name, lang=lang, score=score))
    else:
      Log('Skipping artist, didn\'t meet minimum score of ' + str(ARTIST_MATCH_MIN_SCORE))

    # Sort the resulting artists.
    artist_results.sort(key=lambda r: r.score, reverse=True)

# Get albums by artist and boost artist match score accordingly.  Returns bonus (int) of 0 - ARTIST_ALBUM_MAX_BONUS.
def get_album_bonus(media_albums, artist_id):

  Log('Fetching artist\'s albums and applying album bonus.')
  bonus = 0
  albums = GetAlbumsByArtist(artist_id, albums=[], limit=ARTIST_ALBUMS_LIMIT)

  try:
    for a in media_albums:
      media_album = a.lower()
      for album in albums:

        # If the album title is close enough to the media title, boost the score.
        if Util.LevenshteinDistance(media_album,album['name'].lower()) <= NAME_DISTANCE_THRESHOLD:
          bonus += ARTIST_ALBUM_BONUS_INCREMENT

        # This is a cheap comparison, so let's try again with the contents of parentheses removed, e.g. "(limited edition)"
        elif Util.LevenshteinDistance(media_album,RE_STRIP_PARENS.sub('',album['name'].lower())) <= NAME_DISTANCE_THRESHOLD:
          bonus += ARTIST_ALBUM_BONUS_INCREMENT

        # Stop trying once we hit the max bonus.
        if bonus >= ARTIST_ALBUM_MAX_BONUS:
          break

  except Exception, e:
    Log('Error applying album bonus: ' + str(e))
  if bonus > 0:
    Log('Applying album bonus of: ' + str(bonus))
  return bonus


class LastFmAgent(Agent.Artist):
  name = 'Last.fm'
  languages = [Locale.Language.English, Locale.Language.Swedish, Locale.Language.French,
               Locale.Language.Spanish, Locale.Language.German, Locale.Language.Polish,
               Locale.Language.Italian, Locale.Language.Portuguese, Locale.Language.Japanese,
               Locale.Language.Turkish, Locale.Language.Russian, Locale.Language.Chinese]

  def search(self, results, media, lang, manual):

    # Handle a couple of edge cases where artist search will give bad results.
    if media.artist == '[Unknown Artist]':
      return
    if media.artist == 'Various Artists':
      results.Append(MetadataSearchResult(id = 'Various%20Artists', name= 'Various Artists', thumb = VARIOUS_ARTISTS_POSTER, lang  = lang, score = 100))
      return

    # Search for artist.
    Log('Artist search: ' + media.artist)
    if manual:
      Log('Running custom search...')
    artist_results = []

    artists = SearchArtists(media.artist, ARTIST_MATCH_LIMIT)
    media_albums = [a.title for a in media.children]

    # Score the first N results.
    score_artists(artists, media.artist, media_albums, lang, artist_results)

    # Last.fm search results are heavily influenced by popularity.  As a result, many less popular artist
    # results are buried far down the list, and may not appear on the first page.  In order to minimize API
    # requests during automated matching, we only grab the first page of results.  If we're running a manual
    # or custom match, we can afford to make a few more requests.
    #
    if manual and not artist_results:
      Log('Fetching additional artists for custom search...')
      artists = SearchArtists(media.artist, ARTIST_MANUAL_MATCH_LIMIT)
      score_artists(artists, media.artist, media_albums, lang, artist_results)

    for artist in artist_results:
      results.Append(artist)

  def update(self, metadata, media, lang):
    artist = GetArtist(metadata.id, lang)
    if not artist:
      return

    # Name.
    metadata.title = artist['name']

    # Bio.
    try:
      metadata.summary = String.DecodeHTMLEntities(String.StripTags(artist['bio']['content'][:artist['bio']['content'].find('\n\n')]).strip())
    except:
      pass

    # Artwork.
    if artist['name'] == 'Various Artists':
      metadata.posters[VARIOUS_ARTISTS_POSTER] = Proxy.Media(HTTP.Request(VARIOUS_ARTISTS_POSTER))
    else:
      valid_keys = []
      try:
        for image in artist['image']:
          try:
            if image['size'] in ARTWORK_SIZE_RANKING:
              valid_keys.insert(ARTWORK_SIZE_RANKING[image['size']],image['#text'])
          except:
            pass
        if valid_keys:
          metadata.posters[valid_keys[0]] = Proxy.Media(HTTP.Request(valid_keys[0]))
          metadata.posters.validate_keys(valid_keys[0])
      except:
        Log('Couldn\'t add artwork for artist.')

    # Find similar artists.
    metadata.similar.clear()
    similar_artists = ArtistGetSimilar(artist['name'], lang)
    if similar_artists is not None:
      for similar in similar_artists:
        metadata.similar.add(similar['name'])

    # Events.
    metadata.concerts.clear()
    if Prefs['concerts']:
      events = ArtistGetEvents(artist)
      if events:
        for event in events:
          try:
            concert = metadata.concerts.new()
            concert.title = event['displayName']
            concert.venue = event['venue']['displayName']
            concert.city = event['venue']['metroArea']['displayName']
            concert.country = event['venue']['metroArea']['country']['displayName']
            concert.date = Datetime.ParseDate(event['start']['date'], '%Y-%m-%d')
            concert.url = event['uri']
          except:
            pass

    # Genres.
    metadata.genres.clear()
    if Prefs['genres']:
      try:
        for genre in Listify(artist['tags']['tag']):
          metadata.genres.add(genre['name'].capitalize())
      except:
        Log('Couldn\'t add genre tags for artist.')


class LastFmAlbumAgent(Agent.Album):
  name = 'Last.fm'
  languages = [Locale.Language.English, Locale.Language.Swedish, Locale.Language.French,
               Locale.Language.Spanish, Locale.Language.German, Locale.Language.Polish,
               Locale.Language.Italian, Locale.Language.Portuguese, Locale.Language.Japanese,
               Locale.Language.Turkish, Locale.Language.Russian, Locale.Language.Chinese]

  def search(self, results, media, lang, manual):

    # Handle a couple of edge cases where album search will give bad results.
    if media.parent_metadata.id is None:
      return
    if media.parent_metadata.id == '[Unknown Album]':
      return #eventually, we might be able to look at tracks to match the album

    # Search for album.
    if manual:
      # If this is a custom search, use the user-entered name instead of the scanner hint.
      try:
        Log('Custom album search for: ' + media.name)
        media.title = media.name
      except:
        pass
    else:
      Log('Album search: ' + media.title)
    albums = []
    found_good_match = False

    # First try matching in the list of albums by artist for single-artist albums.
    if media.parent_metadata.id != 'Various%20Artists':

      # Start with the first N albums (ideally a single API request).
      if not manual:
        albums = self.score_albums(media, lang, GetAlbumsByArtist(media.parent_metadata.id, albums=[], limit=ARTIST_ALBUMS_LIMIT))

        # Check for a good match within these reults.  If we find one, set the flag to stop looking.
        if albums and albums[0]['score'] >= ALBUM_MATCH_GOOD_SCORE:
          found_good_match = True
          Log('Good album match found (quick search) with score: ' + str(albums[0]['score']))

      # If we haven't found a good match yet, or we're running a custom search, get all albums by artist.  May be thousands
      # of albums and several API requests to complete this list, so we use it sparingly.
      if not found_good_match or manual:
        if manual:
          Log('Custom search terms specified, fetching all albums by artist.')
        else:
          Log('No good matches found in first ' + str(len(albums)) + ' albums, fetching all albums by artist.')
        albums = self.score_albums(media, lang, GetAlbumsByArtist(media.parent_metadata.id, albums=[]), manual=manual)

        # If we find a good match this way, set the flag to stop looking.
        if albums and albums[0]['score'] >= ALBUM_MATCH_GOOD_SCORE:
          Log('Good album match found with score: ' + str(albums[0]['score']))
          found_good_match = True
        else:
          Log('No good matches found in ' + str(len(albums)) + ' albums by artist.')

    # Either we're looking at Various Artists, or albums by artist search did not contain a good match.
    # Last.fm mysteriously omits certain (often popular) albums from albums-by-artist results, so it's
    # important to fall back even in the case of single-artist albums.
    if not found_good_match or albums:
      albums = self.score_albums(media, lang, SearchAlbums(media.title.lower(), ALBUM_MATCH_LIMIT), manual=manual) + albums

      # If we find a good match for the exact search, stop looking.
      if albums and albums[0]['score'] >= ALBUM_MATCH_GOOD_SCORE:
        found_good_match = True
        Log('Found a good match for album search.')

      # If we still haven't found anything, try another match with parenthetical phrases stripped from
      # album title.  This helps where things like '(Limited Edition)' and '(disc 1)' may confuse search.
      if not albums or not found_good_match:
        stripped_title = RE_STRIP_PARENS.sub('',media.title).lower()
        if stripped_title != media.title.lower():
          Log('No good matches found in album search for %s, searching for %s.' % (media.title.lower(), stripped_title))
          # This time we extend the results  and re-sort so we consider the best-scoring matches from both searches.
          albums  = self.score_albums(media, lang, SearchAlbums(stripped_title), manual=manual) + albums
        if albums:
          albums = sorted(albums, key=lambda k: k['score'], reverse=True)

    # Dedupe albums.
    seen = {}
    deduped = []
    for album in albums:
      if album['id'] in seen:
        continue
      seen[album['id']] = True
      deduped.append(album)
    albums = deduped

    Log('Found ' + str(len(albums)) + ' albums...')

    # Limit to 10 albums.
    albums = albums[:10]
    for album in albums:
      if album['score'] > 0:
        results.Append(MetadataSearchResult(id = album['id'], name = album['name'], lang = album['lang'], score = album['score']))

  # Score a list of albums, return a fresh list of scored matches above the ALBUM_MATCH_MIN_SCORE threshold.
  def score_albums(self, media, lang, albums, manual=False):
    res = []
    matches = []
    for album in albums:
      try:
        name = album['name']

        # Sanitize artist.  Last.fm sometimes returns a string, sometimes a list.
        if album.has_key('artist'):
          if not isinstance(album['artist'], basestring):
            artist = album['artist']['name']
          else:
            artist = album['artist']
        else:
          artist = ''

        id = media.parent_metadata.id + '/' + String.Quote(album['name'].decode('utf-8').encode('utf-8')).replace(' ','+')
        dist = Util.LevenshteinDistance(name.lower(),media.title.lower()) * ALBUM_NAME_DIST_COEFFICIENT

        # Freeform album searches will come back with wacky artists.  If they're not close, penalize heavily, skipping them.
        artist_dist = Util.LevenshteinDistance(artist.lower(),String.Unquote(media.parent_metadata.id).lower())
        if artist_dist > ALBUM_TRACK_BONUS_MAX_ARTIST_DSIT:
          artist_dist = 1000
          Log('Suppressing album result because artist looks wrong: ' + artist)

        # Apply album and artist penalties and append to temp results list.
        score = ALBUM_INITIAL_SCORE - dist - artist_dist
        res.append({'id':id, 'name':name, 'lang':lang, 'score':score})

      except:
        Log('Error scoring album.')

    if res:
      res = sorted(res, key=lambda k: k['score'], reverse=True)
      for i, result in enumerate(res):

        # Fetching albums to apply track bonus is expensive, so only do it for the top N results.
        if i < ALBUM_TRACK_BONUS_MATCH_LIMIT:
          bonus = self.get_track_bonus(media, result['name'], lang)
          res[i]['score'] = res[i]['score'] + bonus

        # Append albums that meet the minimum score, skip the rest.
        if res[i]['score'] >= ALBUM_MATCH_MIN_SCORE or manual:
          Log('Album result: ' + result['name'] + ' album bonus: ' + str(bonus) + ' score: ' + str(result['score']))
          matches.append(res[i])
        else:
          Log('Skipping %d album results that don\'t meet the minimum score of %d.' % (len(res) - i, ALBUM_MATCH_MIN_SCORE))
          break

    # Sort once more to account for track bonus and return.
    if matches:
      return sorted(matches, key=lambda k: k['score'], reverse=True)
    else:
      return matches

  # Get album info in order to compare track listings and apply bonus accordingly.  Return a bonus (int) of 0 - ALBUM_TRACK_MAX_BONUS.
  def get_track_bonus(self, media, name, lang):
    tracks = GetTracks(media.parent_metadata.id, String.Quote(name.decode('utf-8').encode('utf-8')).replace(' ','+'), lang)
    bonus = 0
    try:
      for i, t in enumerate(media.children):
        media_track = t.title.lower()
        for j, track in enumerate(tracks):

          # If the names are close enough, boost the score.
          if Util.LevenshteinDistance(track['name'].lower(), media_track) <= NAME_DISTANCE_THRESHOLD:
            bonus += ALBUM_TRACK_BONUS_INCREMENT

      # If the albums have the same number of tracks, boost more.
      if len(media.children) == len(tracks):
        bonus += ALBUM_NUM_TRACKS_BONUS

      # Cap the bonus.
      if bonus >= ALBUM_TRACK_MAX_BONUS:
        bonus = ALBUM_TRACK_MAX_BONUS

    except:
      Log('Didn\'t find any usable tracks in search results, not applying track bonus.')

    if bonus > 0:
      Log('Applying track bonus of: ' + str(bonus))
    return bonus

  def update(self, metadata, media, lang):
    album = GetAlbum(metadata.id.split('/')[0], metadata.id.split('/')[1], lang)
    if not album:
      return

    # Title.
    metadata.title = album['name']

    # Artwork.
    valid_keys = []
    try:
      for image in album['image']:
        try:
          if image['size'] in ARTWORK_SIZE_RANKING:
            valid_keys.insert(ARTWORK_SIZE_RANKING[image['size']],image['#text'])
        except:
          pass
      if valid_keys:
        metadata.posters[valid_keys[0]] = Proxy.Media(HTTP.Request(valid_keys[0]))
        metadata.posters.validate_keys(valid_keys[0])
    except:
      Log('Couldn\'t add artwork for album.')

    # Release Date.
    try:
      if album['releasedate']:
        metadata.originally_available_at = Datetime.ParseDate(album['releasedate'].split(',')[0].strip())
    except:
      Log('Couldn\'t add release date to album.')

    # Genres.
    metadata.genres.clear()
    if Prefs['genres']:
      try:
        for genre in Listify(album['toptags']['tag']):
          metadata.genres.add(genre['name'].capitalize())
      except:
        Log('Couldn\'t add genre tags to album.')

    # Top tracks.
    most_popular_tracks = {}
    try:
      top_tracks = GetArtistTopTracks(metadata.id.split('/')[0], lang)
      for track in top_tracks:
        most_popular_tracks[track['name']] = int(track['playcount'])
    except:
      pass

    valid_track_keys = []
    for index in media.tracks:
      track_key = media.tracks[index].id or int(index)
      valid_track_keys.append(track_key)
      for popular_track in most_popular_tracks.keys():
        if popular_track and LevenshteinRatio(popular_track, media.tracks[index].title) > 0.95:
          t = metadata.tracks[track_key]
          if Prefs['popular']:
            t.rating_count = most_popular_tracks[popular_track]
          else:
            t.rating_count = 0
    metadata.tracks.validate_keys(valid_track_keys)

def SearchArtists(artist, limit=10, legacy=False):
  artists = []

  if not artist:
    Log('Missing artist. Skipping match')
    return artists

  lim = min(limit,ARTIST_SEARCH_PAGE_SIZE)
  for i in range((limit-1)/ARTIST_SEARCH_PAGE_SIZE+1):
    try:
      a = artist.lower().encode('utf-8')
    except:
      a = artist.lower()
    url = ARTIST_SEARCH_URL % (String.Quote(a), i+1, lim)
    try:
      response = GetJSON(url)
      if response.has_key('error'):
        Log('Error retrieving artist search results: ' + response['message'])
      else:
        artist_results = response['results']
        artists = artists + Listify(artist_results['artistmatches']['artist'])
    except:
      Log('Error retrieving artist search results.')

  # Since LFM has lots of garbage artists that match garbage inputs, we'll only consider ones that have
  # either a MusicBrainz ID or artwork.
  #
  valid_artists = [a for a in artists if a['mbid'] or (len(a.get('image', [])) > 0 and a['image'][0].get('#text', None))]
  if len(artists) != len(valid_artists):
    Log('Skipping artist results because they lacked artwork or MBID: %s' % ', '.join({a['name'] for a in artists}.difference({a['name'] for a in valid_artists})))

  return valid_artists


def SearchAlbums(album, limit=10, legacy=False):
  albums = []

  if not album:
    Log('Missing album. Skipping match')
    return albums

  try:
    a = album.lower().encode('utf-8')
  except:
    a = album.lower()

  url = ALBUM_SEARCH_URL % (String.Quote(a), limit)
  try:
    response = GetJSON(url)
    if response.has_key('error'):
      Log('Error retrieving album search results: ' + response['message'])
      return albums
    else:
      album_results = response['results']
      albums = Listify(album_results['albummatches']['album'])
  except:
    Log('Error retrieving album search results.')

  return albums


def GetAlbumsByArtist(artist, page=1, limit=ARTIST_ALBUMS_LIMIT*4, pg_size=ARTIST_ALBUMS_LIMIT, albums=[], legacy=True):
  url = ARTIST_ALBUM_SEARCH_URL % (String.Quote(String.Unquote(artist.lower())), page, pg_size)
  total = 0
  try:

    # We use a larger page size when fetching all to limit the number of API requests. We can't use
    # a huge value, e.g. 10000 because not all results will be returned for some unknown reason.
    if not limit:
      pg_size = 200

    response = GetJSON(url)
    if response.has_key('error'):
      Log('Error retrieving artist album search results: ' + response['message'])
      return albums
    else:
      album_results = response['topalbums']

    # Handle two different formats that Last.fm may use to return total matches.
    if album_results.has_key('@attr'):
      total = int(album_results['@attr']['total'])
    elif album_results.has_key('total'):
      total = int(album_results['total'])
    if total == 0:
      Log('No results for album search.')

  except:
    Log('Error retrieving artist album search results.')

  try:
    albums.extend(Listify(album_results['album']))
  except:
    # Sometimes the API will lie and say there's an Nth page of results, but the last one will return garbage.
    pass

  if (total > page * pg_size and not limit) or (page * pg_size < limit):
    return GetAlbumsByArtist(artist, page=page+1, limit=limit, pg_size=pg_size, albums=albums, legacy=False)
  else:
    return albums


def GetArtist(id, lang='en'):
  url = ARTIST_INFO_URL % (id.lower(), lang)
  try:
    artist_results = GetJSON(url)
    if artist_results.has_key('error'):
      Log('Error retrieving artist metadata: ' + artist_results['message'])
      return {}
    return artist_results['artist']
  except:
    Log('Error retrieving artist metadata.')
    return {}


def GetAlbum(artist_id, album_id, lang='en'):
  url = ALBUM_INFO_URL % (artist_id.lower(), album_id.lower(), lang)
  try:
    album_results = GetJSON(url)
    if album_results.has_key('error'):
      Log('Error retrieving album metadata: ' + album_results['message'])
      return {}
    return album_results['album']
  except:
    Log('Error retrieving album metadata.')
    return {}


def GetTracks(artist_id, album_id, lang='en'):
  url = ALBUM_INFO_URL % (artist_id.lower(), album_id.lower(), lang)
  try:
    tracks_result = GetJSON(url)
    if tracks_result.has_key('error'):
      Log('Error retrieving tracks: ' + tracks_result['message'])
      return []
    return Listify(tracks_result['album']['tracks']['track'])
  except:
    Log('Error retrieving tracks.')
    return []


def GetArtistTopTracks(artist_id, lang='en'):
  result = []

  try:
    page = 1

    while True:
        url = ARTIST_TOP_TRACKS_URL % (artist_id.lower(), lang, page)
        top_tracks_result = GetJSON(url)

        # Get out if we have an error.
        if top_tracks_result.has_key('error'):
          Log('Error receiving top tracks: ' + top_tracks_result['message'])
          break

        total_pages = int(top_tracks_result['toptracks']['@attr']['totalPages'])
        new_results = Listify(top_tracks_result['toptracks']['track'])
        result.extend(new_results)
        Log('Last popular track in page %d (out of %d) had %d listeners', page, total_pages, int(result[-1]['listeners']))

        # Get out if we've reached the very unpopular.
        if int(result[-1]['listeners']) < 1000:
          break

        # Get out if we've exceeded the number of pages.
        page += 1
        if page > total_pages:
          break
  except:
    Log('Exception getting top tracks.')

  return result

def GetArtistSimilar(artist_id, lang='en'):
  url = ARTIST_SIMILAR_ARTISTS_URL % (artist_id.lower(), lang)
  try:
    similar_artists_result = GetJSON(url)
    if similar_artists_result.has_key('error'):
      Log('Error receiving similar artists: ' + similar_artists_result['message'])
      return []
    if isinstance(similar_artists_result['similarartists']['artist'], list) or isinstance(similar_artists_result['similarartists']['artist'], dict):
      return Listify(similar_artists_result['similarartists']['artist'])
  except:
    Log('Exception getting similar artists.')
    return []

@expose
def GetArtistEventsFromSongkickById(artist_mbid=None, artist_songkickid = None):
  url = None
  if artist_mbid:
    url = SONGKICK_ARTIST_EVENTS_URL_MBID % artist_mbid
  elif artist_songkickid:
    url = SONGKICK_ARTIST_EVENTS_URL_SONGKICKID % artist_songkickid
  else:
    return []

  try:
    events_result = GetSongkickJSON(url, cache_time=CACHE_1WEEK)
    status = events_result['resultsPage']['status']
    totalEntries = events_result['resultsPage']['totalEntries']
    if status == 'ok' and totalEntries > 0:
      return events_result['resultsPage']['results']['event']
  except:
    Log('Exception getting events.')

  return []


def GetArtistSongkickId(artist_name):
  artist_name_escaped = NormalizeArtist(artist_name)
  url = SONGKICK_ARTIST_SEARCH_URL % artist_name_escaped
  try:
    artist_search_result = GetSongkickJSON(url, cache_time=CACHE_1WEEK)
    status = artist_search_result['resultsPage']['status']
    totalEntries = artist_search_result['resultsPage']['totalEntries']
    if status == 'ok' and totalEntries > 0:
      first_artist_result = artist_search_result['resultsPage']['results']['artist'][0]
      artist_songkick_name = first_artist_result['displayName']
      if LevenshteinRatio(artist_name_escaped, NormalizeArtist(artist_songkick_name)) > 0.95:
        return first_artist_result['id']
  except:
    Log('Exception searching Songkick artist id.')

  return None


def GetJSON(url, sleep_time=QUERY_SLEEP_TIME, cache_time=CACHE_1MONTH):
  d = None
  try:
    d = JSON.ObjectFromURL(url, sleep=sleep_time, cacheTime=cache_time, headers={'Accept-Encoding':'gzip', 'X-PLEX-VERSION':'2.0'})
    if isinstance(d, dict):
      return d
  except:
    Log('Error fetching JSON.')
    return None


def GetSongkickJSON(url, sleep_time=QUERY_SLEEP_TIME, cache_time=CACHE_1MONTH):
  d = None
  try:
    d = JSON.ObjectFromURL(SONGKICK_BASE_URL % String.Quote(url, True), sleep=sleep_time, headers={'Accept': 'application/json'}, cacheTime=cache_time)
    if isinstance(d, dict):
      return d
  except:
    Log('Error fetching JSON.')
    return None


def LevenshteinRatio(first, second):
  return 1 - (Util.LevenshteinDistance(first, second) / float(max(len(first), len(second))))

def NormalizeArtist(name):
  return Core.messaging.call_external_function('com.plexapp.agents.plexmusic', 'MessageKit:NormalizeArtist', kwargs = dict(artist=name))

# Utility functions for sanitizing Last.fm API responses.
def Listify(obj):
  if isinstance(obj, list):
    return obj
  else:
    return [obj]

def Dictify(obj, key=''):
  if isinstance(obj, dict):
    return obj
  else:
    return {key:obj}
