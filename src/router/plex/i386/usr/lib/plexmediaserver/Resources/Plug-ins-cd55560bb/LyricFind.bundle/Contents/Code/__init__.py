from collections import defaultdict

SEARCH_URL = 'http://127.0.0.1:32400/services/lyricfind/search'
SEARCH_PARAMS = 'reqtype=default&searchtype=track&limit=100&output=json'

def Start():
  HTTP.CacheTime = 0

def Search(artist, album=None, track=None):
  url = SEARCH_URL + '?' + SEARCH_PARAMS + '&artist=' + String.Quote(artist)
  if album:
    url += '&album=' + String.Quote(album)
  if track:
    url += '&track=' + String.Quote(track)
    
  try: 
    json = JSON.ObjectFromURL(url)
    
    # Some errors come back as 200 OK with info in the JSON.
    if 'tracks' not in json:
      json['tracks'] = []
  except: 
    json = {'tracks':[]}
    
  return json['tracks']
  
def AddLyric(metadata, album, track, lyric, valid_keys, track_key):
  score = 100 - (10 * abs(String.LevenshteinDistance(track.title.lower(), lyric['title'].lower())))
  
  if score > 85 and lyric['viewable']:
    # Add the lyric
    Log('Adding Lyric %s to track %s' % (lyric['title'], track.title))
    args = '&artist=%s&album=%s&track=%s' % (String.Quote(lyric['artist']['name']), String.Quote(album), String.Quote(lyric['title']))
    
    # Add the LRC first.
    sort_order = 1
    if lyric['has_lrc'] == True:
      url = 'http://127.0.0.1:32400/services/lyricfind/lyrics?id=' + lyric['lfid'] + '&lrc=1' + args
      metadata.tracks[track_key].lyrics[url] = Proxy.Remote(url, format = 'lrc', sort_order=sort_order)
      valid_keys[track_key].append(url)
      sort_order += 1
      
    # Add text lyrics.
    url = 'http://127.0.0.1:32400/services/lyricfind/lyrics?id=' + lyric['lfid'] + args
    metadata.tracks[track_key].lyrics[url] = Proxy.Remote(url, format = 'txt', sort_order=sort_order)
    valid_keys[track_key].append(url)
    sort_order += 1
    
    return True
    
  return False


def CopyLyrics(metadata, media, track, valid_keys):

  # We need to recreate the Proxy objects to effectively pass them through since the Framework doesn't
  # deserialize them and they're missing from the metadata object in here (we only have the keys).
  #
  sort_order = 1

  track_key = track.id

  # Old track could be old (guid) or new (id).
  old_track = None
  if track.guid in metadata.tracks:
    old_track = metadata.tracks[track.guid]
  elif track.id in metadata.tracks:
    old_track = metadata.tracks[track.id]

  if old_track is not None:
    for key in old_track.lyrics:
      metadata.tracks[track_key].lyrics[key] = Proxy.Remote(key, format='lrc' if '&lrc=1' in key else 'txt', sort_order=sort_order)
      valid_keys[track_key].append(key)
      sort_order += 1


####################################################################################################
class LyricFindAlbumAgent(Agent.Album):

  name = 'LyricFind'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  contributes_to = ['com.plexapp.agents.lastfm', 'tv.plex.agents.music', 'org.musicbrainz.agents.music']

  def search(self, results, media, lang, manual=False, tree=None):
    results.add(SearchResult(id = 'null', score = 100))
    
  def update(self, metadata, media, lang, force=False):

    valid_keys = defaultdict(list)
    valid_track_keys = []

    # Only search if this is a new addition, the album was released within the past six months, or this is a manual refresh (LyricFind DDoS mitigation).
    if media.refreshed_at == None or Datetime.Now() - Datetime.ParseDate(media.originally_available_at or '1900') < Datetime.Delta(days=180) or force:

      # First look for artist + album so we can grab a bunch of lyrics.
      lyrics = Search(artist=media.parentTitle, album=media.title)

      @parallelize
      def MatchLyrics():
        for index, track in enumerate(media.children):

            @task
            def MatchLyric(index=index, track=track):
              track_key = track.id or index
              valid_keys[track_key] = []
              valid_track_keys.append(track_key)
              added = False

              # Only search if this track is missing lyrics (LyricFind DDoS mitigation).
              if len(metadata.tracks[track_key].lyrics) > 0 and not force:
                Log('%s already has lyrics, we won\'t search again' % track.title)
                CopyLyrics(metadata, media, track_key, valid_keys)
              else:
                # See if any of the lyrics match.
                for lyric in lyrics:
                  added = AddLyric(metadata, media.title, track, lyric, valid_keys, track_key)
                  if added:
                    break

                # If we didn't find a lyric for the track, try artist/track search.
                if not added:
                  for lyric in Search(artist=media.parentTitle, track=track.title):
                    added = AddLyric(metadata, media.title, track, lyric, valid_keys, track_key)
                    if added:
                      break

    else:
      Log('Not searching for new lyrics for a periodic refresh of a preexisting album more than six months old.')
      for index, track in enumerate(media.children):
        track_key = track.id or index
        valid_track_keys.append(track_key)
        CopyLyrics(metadata, media, track, valid_keys)

    # Make sure we validate lyric keys.
    for key in valid_keys.keys():
      metadata.tracks[key].lyrics.validate_keys(valid_keys[key])

    metadata.tracks.validate_keys(valid_track_keys)
