#
# Copyright (c) 2019 Plex Development Team. All rights reserved.
#

from collections import defaultdict

Languages = [Locale.Language.English, Locale.Language.Arabic, Locale.Language.Bulgarian, Locale.Language.Chinese, Locale.Language.Croatian,
             Locale.Language.Czech, Locale.Language.Danish, Locale.Language.Dutch, Locale.Language.Finnish, Locale.Language.French,
             Locale.Language.German, Locale.Language.Greek, Locale.Language.Hungarian, Locale.Language.Indonesian, Locale.Language.Italian,
             Locale.Language.Japanese, Locale.Language.Korean, Locale.Language.NorwegianNynorsk, Locale.Language.Polish,
             Locale.Language.Portuguese, Locale.Language.Romanian, Locale.Language.Russian, Locale.Language.Serbian, Locale.Language.Slovak,
             Locale.Language.Spanish, Locale.Language.Swedish, Locale.Language.Thai, Locale.Language.Turkish, Locale.Language.Vietnamese,
             Locale.Language.Unknown]

BLANK_FIELD = '\x7f'

def StringOrBlank(s):
  if s is not None:
    s = str(s).strip('\0')
    if len(s) == 0:
      s = BLANK_FIELD
  else:
    s = BLANK_FIELD
  return s

def Start():
  HTTP.CacheTime = 0

def find_songkick_events(artist_mbid):
  try: return Core.messaging.call_external_function('com.plexapp.agents.lastfm', 'MessageKit:GetArtistEventsFromSongkickById', kwargs = dict(artist_mbid=artist_mbid))
  except: return None

def add_graphics(object, graphics):
  valid_keys = []
  for i, graphic in enumerate(graphics):
    try:
      key = graphic.get('key')
      preview = graphic.get('previewKey')
      order = '%02d' % (i + 1)
      if key not in object:
        if preview:
          object[key] = Proxy.Preview(HTTP.Request(preview).content, sort_order = order)
        else:
          object[key] = Proxy.Media(HTTP.Request(key), sort_order = order)
      valid_keys.append(key)
    except Exception, e:
      Log('Couldn\'t add poster (%s %s): %s' % (graphic.get('key'), graphic.get('previewKey'), str(e)))

  object.validate_keys(valid_keys)

def add_tags(res, metadata_tags, name):
  metadata_tags.clear()
  for tag in res.xpath('//Directory/' + name):
    metadata_tags.add(tag.get('tag'))

class PlexMusicArtistAgent(Agent.Artist):
  name = 'Plex Music'
  languages = Languages
  contributes_to = ['com.plexapp.agents.localmedia']

  def search(self, results, media, lang='en', manual=False, tree=None, primary=True):
    pass

  def update(self, metadata, media, lang, prefs):
    Log('Updating : %s (GUID: %s) : %s' % (media.title, media.guid, prefs))

    # Fetch the artist.
    rating_key = media.guid.split('://')[1].split('/')[1]
    url = 'http://127.0.0.1:32400/metadata/agents/music/library/metadata/' + rating_key + '?includeAlternates=1'
    res = XML.ElementFromURL(url)
    artists = res.xpath('//Directory[@type="artist"]')
    if len(artists) == 0:
      return

    # The basics.
    artist = artists[0]
    metadata.title = artist.get('title')
    metadata.title_sort = ''  # artist.get('titleSort')

    summary = artist.get('summary')
    metadata.summary = summary if (prefs['artistBios'] == 1 and summary) else BLANK_FIELD

    # Add posters and artwork.
    add_graphics(metadata.posters, res.xpath('//Directory[@type="artist"]/Thumb'))
    add_graphics(metadata.art, res.xpath('//Directory[@type="artist"]/Art'))

    # Tags.
    metadata.genres.clear()
    if prefs['genres'] == 1:
      add_tags(res, metadata.genres, 'Genre')

    add_tags(res, metadata.styles, 'Style')
    add_tags(res, metadata.moods, 'Mood')
    add_tags(res, metadata.countries, 'Country')

    # Similar.
    metadata.similar.clear()
    for similar in res.xpath('//Directory[@type="artist"]/Similar'):
      metadata.similar.add(similar.get('tag'))

    # Concerts.
    metadata.concerts.clear()
    if prefs['concerts'] == 0:
      return

    guids = res.xpath('//Directory[@type="artist"]/Guid')
    mbid_guids = [guid.get('id') for guid in guids if guid.get('id').startswith('mbid')]
    if len(mbid_guids) > 0:
      events = find_songkick_events(mbid_guids[0].split('://')[1])
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

##################################################################################
class PlexMusicAlbumAgent(Agent.Album):
  name = 'Plex Music'
  languages = Languages
  contributes_to = ['com.plexapp.agents.localmedia']

  def search(self, results, media, lang, manual=False, tree=None, primary=False):
    pass

  def update(self, metadata, media, lang, prefs):

    # Fetch the album, preferring the instance rating key.
    Log('Updating : %s (GUID: %s) : %s' % (media.title, media.guid, prefs))
    rating_key = media.instanceRatingKey or media.guid.split('://')[1].split('/')[1]
    url = 'http://127.0.0.1:32400/metadata/agents/music/library/metadata/' + rating_key + '?includeAlternates=1&includeChildren=1'
    res = XML.ElementFromURL(url)
    albums = res.xpath('//Directory[@type="album"]')
    if len(albums) == 0:
      return

    # The basics.
    album = albums[0]
    metadata.title = album.get('title')

    summary = album.get('summary')
    metadata.summary = summary if (prefs['albumReviews'] == 1 and summary) else BLANK_FIELD
    metadata.studio = album.get('studio') or BLANK_FIELD
    metadata.rating = float(album.get('rating') or -1.0) if (prefs['albumReviews'] == 1) else -1.0

    # Release date.
    if album.get('originallyAvailableAt'):
      metadata.originally_available_at = Datetime.ParseDate(album.get('originallyAvailableAt').split('T')[0])

    # Posters, if we want them.
    if prefs['albumPosters'] != 3:
      add_graphics(metadata.posters, res.xpath('//Directory[@type="album"]/Thumb'))
    else:
      metadata.posters.validate_keys([])

    # Genres.
    metadata.genres.clear()
    if prefs['genres'] == 1:
      add_tags(res, metadata.genres, 'Genre')

    add_tags(res, metadata.styles, 'Style')
    add_tags(res, metadata.moods, 'Mood')

    # Build a map of tracks, keeping in mind there could be multiple tracks with the same GUID.
    cloud_tracks = defaultdict(list)
    for track in res.xpath('//Track'):
      cloud_tracks[track.get('guid')].append(track)

    # Get track data.
    use_rating_count = (prefs['popularTracks'] == 1)
    valid_keys = []
    for track in media.children:
      track_key = track.id
      valid_keys.append(track_key)

      cloud_track = self.find_matching_track(cloud_tracks, track)
      metadata_track = metadata.tracks[track_key]
      if cloud_track and metadata_track:
        metadata_track.title = cloud_track.get('title')
        metadata_track.track_index = int(cloud_track.get('index'))
        metadata_track.disc_index = int(cloud_track.get('parentIndex') or '1')
        metadata_track.original_title = cloud_track.get('originalTitle') or BLANK_FIELD
        metadata_track.rating_count = int(cloud_track.get('ratingCount') or '0') if use_rating_count else 0
        metadata_track.moods.clear()
        for tag in cloud_track.xpath('Mood'):
          metadata_track.moods.add(tag.get('tag'))

    metadata.tracks.validate_keys(valid_keys)

  def find_matching_track(self, cloud_tracks, track):
    if track.guid in cloud_tracks:
      matches = cloud_tracks[track.guid]
      for match in matches:
        if track.index == match.get('index') and track.absoluteIndex == (match.get('parentIndex') or '1'):
          return match
    return None
